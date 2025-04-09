// dispatcher.cpp
#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

#include "dispatcher.h"
#include "f_task.h"
#include "core.h"
#include "fifo_channel.h"

namespace Dispatcher
{

    // --- Internal extended task structure ---
    // Extends the base F_Task (from f_task.h) to include parameters for note operations.
    struct ExtendedTask : public F_Task
    {
        int noteID;
        int userID;
        std::string noteData;

        ExtendedTask(Task_Type t, int nID, int uID, const std::string &data)
            : noteID(nID), userID(uID), noteData(data)
        {
            type = t;
            progress = 0;
            is_done = false;
            thread_id = 0;
        }
    };

    // --- Function to return a static priority for each task type ---
    int getStaticPriority(Task_Type type)
    {
        // Arbitrary static priority values:
        // Lower numbers mean higher priority.
        switch (type)
        {
        case PING:
            return 5;
        case CREATE_NOTE:
            return 1; // Also special, so handled separately.
        case EDIT_NOTE:
            return 2;
        case SING_IN:
            return 3;
        case SING_UP:
            return 3;
        case LOG_OUT:
            return 4;
        default:
            return 10;
        }
    }

    // --- Wrapper to attach a priority to each task ---
    struct PrioritizedTask
    {
        ExtendedTask task;
        int priority; // Lower value indicates higher priority.

        PrioritizedTask(const ExtendedTask &t, int p)
            : task(t), priority(p) {}
    };

    // Comparator so that tasks with lower numerical priority come first.
    struct TaskComparator
    {
        bool operator()(const PrioritizedTask &a, const PrioritizedTask &b)
        {
            return a.priority > b.priority;
        }
    };

    // --- Dispatcher Implementation ---
    class DispatcherImpl
    {
    public:
        DispatcherImpl() : stopFlag(false), specialTaskActive(false) {}
        ~DispatcherImpl() { stop(); }

        // Create the thread pool. This function corresponds to the header declaration.
        bool createThreads(unsigned int numThreads)
        {
            if (numThreads == 0)
            {
                throw std::runtime_error("Number of threads must be greater than 0");
            }
            for (unsigned int i = 0; i < numThreads; ++i)
            {
                threads.emplace_back(&DispatcherImpl::workerThread, this);
            }
            return true;
        }

        // Modified addTask: no need to pass a priority; we compute it statically.
        void addTask(const ExtendedTask &task)
        {
            int priority = getStaticPriority(task.type);
            // If the task is a CREATE_NOTE, treat it as a special task.
            if (task.type == CREATE_NOTE)
            {
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    specialTaskActive = true; // Stall other tasks.
                }
                // Process the special task in its own thread (blocking until complete).
                std::thread specialThread(&DispatcherImpl::processSpecialTask, this, task);
                specialThread.join();
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    specialTaskActive = false;
                }
                cv.notify_all();
            }
            else
            {
                // For all other tasks, push them into the priority queue.
                std::unique_lock<std::mutex> lock(queueMutex);
                taskQueue.push(PrioritizedTask(task, priority));
                cv.notify_one();
            }
        }

        // New function: Listens for incoming tasks via a FIFO channel.
        // This function opens a FIFO for reading requests and another for sending responses.
        // It loops forever, reading a Task from the request FIFO, dispatching it, and then sending back a response.
        void listen(const std::string &reqFifoPath, const std::string &resFifoPath)
        {
            // Create FIFO channels for requests and responses.
            ipc::FifoChannel requestChannel(reqFifoPath, O_RDONLY, true);
            ipc::FifoChannel responseChannel(resFifoPath, O_WRONLY, true);

            ipc::Task ipcTask;
            while (true)
            {
                // Block until a task is received via the FIFO.
                if (requestChannel.read(ipcTask))
                {
                    std::cout << "[Dispatcher] Received IPC Task: ";
                    ipcTask.print();
                    // Convert ipc::Task to ExtendedTask.
                    // (In a real system, you’d parse payload to determine type and parameters.)
                    // Here we use task.id as an example to pick a type:
                    Task_Type type = static_cast<Task_Type>(ipcTask.id);
                    ExtendedTask extTask(type, ipcTask.id, 0, ipcTask.payload);
                    // Dispatch the task using our addTask function.
                    addTask(extTask);
                    // After processing, mark the IPC task as completed and send it back.
                    ipcTask.completed = true;
                    responseChannel.send(ipcTask);
                }
            }
        }

        // New function: Start the listener on a separate thread.
        void start(const std::string &reqFifoPath, const std::string &resFifoPath)
        {
            std::thread listenerThread(&DispatcherImpl::listen, this, reqFifoPath, resFifoPath);
            // Detach so that the listener runs in the background.
            listenerThread.detach();
        }

        // Stop the dispatcher and join all threads.
        void stop()
        {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                stopFlag = true;
            }
            cv.notify_all();
            for (auto &t : threads)
            {
                if (t.joinable())
                    t.join();
            }
        }

    private:
        // Worker thread function: processes tasks from the queue.
        void workerThread()
        {
            while (true)
            {
                PrioritizedTask prioritizedTask(ExtendedTask(PING, 0, 0, ""), 0); // Default initialization.
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [this]
                            { return stopFlag || (!taskQueue.empty() && !specialTaskActive); });
                    if (stopFlag)
                        break;
                    if (specialTaskActive)
                        continue;
                    prioritizedTask = taskQueue.top();
                    taskQueue.pop();
                }

                // Process task based on type.
                switch (prioritizedTask.task.type)
                {
                case PING:
                    std::cout << "[Worker] Processing PING task. Responding with PONG.\n";
                    break;
                case EDIT_NOTE:
                    std::cout << "[Worker] Processing EDIT_NOTE task.\n";
                    Core::editNote(prioritizedTask.task.noteID,
                                   prioritizedTask.task.userID,
                                   prioritizedTask.task.noteData);
                    break;
                case SING_IN:
                    // Process sign-in...
                    break;
                case SING_UP:
                    // Process sign-up...
                    break;
                case LOG_OUT:
                    // Process log-out...
                    break;
                default:
                    std::cout << "[Worker] Unknown task type received.\n";
                    break;
                }
            }
        }

        // Processes a special CREATE_NOTE task exclusively.
        void processSpecialTask(const ExtendedTask &task)
        {
            std::cout << "[Dispatcher] Processing special CREATE_NOTE task exclusively.\n";
            Core::createNote(task.noteID, task.userID, task.noteData);
            std::cout << "[Dispatcher] Special CREATE_NOTE task completed.\n";
        }

        // Internal members.
        std::priority_queue<PrioritizedTask, std::vector<PrioritizedTask>, TaskComparator> taskQueue;
        std::vector<std::thread> threads;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stopFlag;
        bool specialTaskActive;
    };

    // --- Singleton instance for internal use ---
    static DispatcherImpl dispatcherImpl;

    // --- Public interface as declared in the header file ---
    bool create_threads(unsigned int num_threads)
    {
        return dispatcherImpl.createThreads(num_threads);
    }

    // New: Expose the start() function so that the dispatcher begins listening.
    void start_listening(const std::string &reqFifoPath, const std::string &resFifoPath)
    {
        dispatcherImpl.start(reqFifoPath, resFifoPath);
    }

// --- Additional internal functions for testing ---
// These functions are not part of the original header interface but are useful for adding tasks and stopping the dispatcher.
#ifdef DISPATCHER_TEST
    // Expose addTask for testing purposes.
    void addTaskForTesting(const ExtendedTask &task)
    {
        dispatcherImpl.addTask(task);
    }

    // Expose stop functionality.
    void stopDispatcher()
    {
        dispatcherImpl.stop();
    }
#endif

} // namespace Dispatcher

// // --- Example main() for testing if DISPATCHER_TEST is defined ---
// #ifdef DISPATCHER_TEST
// #include <chrono>
// int main() {
//     using namespace Dispatcher;

//     // Create a thread pool of 4 worker threads.
//     create_threads(4);

//     // Create and submit a few tasks.
//     ExtendedTask pingTask(PING, 0, 0, "");
//     addTaskForTesting(pingTask);

//     ExtendedTask editTask(EDIT_NOTE, 101, 1, "Updated note content");
//     addTaskForTesting(editTask);

//     // Special task: CREATE_NOTE will stall other threads.
//     ExtendedTask createTask(CREATE_NOTE, 102, 2, "New note content");
//     addTaskForTesting(createTask);

//     ExtendedTask pingTask2(PING, 0, 0, "");
//     addTaskForTesting(pingTask2);

//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     stopDispatcher();
//     return 0;
// }
// #endif
