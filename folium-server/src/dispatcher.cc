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

namespace Dispatcher {

// --- Internal extended task structure ---
// Extends the base F_Task (from f_task.h) to include parameters for note operations.
struct ExtendedTask : public F_Task {
    int noteID;
    int userID;
    std::string noteData;
    
    ExtendedTask(Task_Type t, int nID, int uID, const std::string& data)
      : noteID(nID), userID(uID), noteData(data) {
          type = t;
          progress = 0;
          is_done = false;
          thread_id = 0;
      }
};

// --- Function to return a static priority for each task type ---
int getStaticPriority(Task_Type type) {
    // Arbitrary static priority values:
    // Lower numbers mean higher priority.
    switch (type) {
        case PING:       return 5;
        case CREATE_NOTE:return 1;  // Also special, so handled separately.
        case EDIT_NOTE:  return 2;
        case SING_IN:    return 3;
        case SING_UP:    return 3;
        case LOG_OUT:    return 4;
        default:         return 10;
    }
}

// --- Wrapper to attach a priority to each task ---
struct PrioritizedTask {
    ExtendedTask task;
    int priority; // Lower value indicates higher priority.
    
    PrioritizedTask(const ExtendedTask& t, int p)
      : task(t), priority(p) {}
};

// Comparator so that tasks with lower numerical priority come first.
struct TaskComparator {
    bool operator()(const PrioritizedTask& a, const PrioritizedTask& b) {
        return a.priority > b.priority;
    }
};

// --- Dispatcher Implementation ---
class DispatcherImpl {
public:
    // Constructor now takes FIFO paths for requests and responses.
    DispatcherImpl(const std::string &reqFifo, const std::string &resFifo)
      : requestFifo(reqFifo), responseFifo(resFifo),
        stopFlag(false), specialTaskActive(false)
    {}
        ~DispatcherImpl() { stop(); }

    // Create the thread pool. This function corresponds to the header declaration.
    bool createThreads(unsigned int numThreads) {
        if (numThreads == 0) {
            throw std::runtime_error("Number of threads must be greater than 0");
        }
        for (unsigned int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&DispatcherImpl::workerThread, this);
        }
        return true;
    }

        // Start the listener thread. The callback will be invoked for each IPC task received.
    // The FIFO paths are already stored in the object.
    void startListener(const std::function<void(const ipc::Task&)>& callback) {
        listenerThread = std::thread(&DispatcherImpl::listen, this, callback);
    }

    // Add a task to the dispatcher.
    void addTask(const ExtendedTask& task) {
        int priority = getStaticPriority(task.type);
        if (task.type == CREATE_NOTE) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                specialTaskActive = true;  // Stall other tasks.
            }
            // Process special task in its own temporary thread.
            std::thread specialThread(&DispatcherImpl::processSpecialTask, this, task);
            specialThread.join();
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                specialTaskActive = false;
            }
            cv.notify_all();
        } else {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskQueue.push(PrioritizedTask(task, priority));
            cv.notify_one();
        }
    }

private:
    // Listener function that continuously reads IPC tasks.
    void listen(const std::function<void(const ipc::Task&)>& callback) {
        // Open FIFO channels (blocking mode, auto-create if needed is assumed)
        ipc::FifoChannel requestChannel(requestFifo, O_RDONLY, true);
        ipc::FifoChannel responseChannel(responseFifo, O_WRONLY, true);
        ipc::Task ipcTask;
        while (!stopFlag) {
            if (requestChannel.read(ipcTask)) {
                std::cout << "[Dispatcher] Received IPC Task: ";
                ipcTask.print();
                
                // Invoke the callback to notify the application.
                callback(ipcTask);

                // Convert the ipc::Task to an ExtendedTask.
                Task_Type type = static_cast<Task_Type>(ipcTask.id);
                ExtendedTask extTask(type, ipcTask.id, 0, ipcTask.payload);
                addTask(extTask);

                ipcTask.completed = true;
                responseChannel.send(ipcTask);
            }
        }
    }

        // New function: Start the listener on a separate thread.
    void start(const std::string& reqFifoPath, const std::string& resFifoPath) {
        std::thread listenerThread(&DispatcherImpl::listen, this, reqFifoPath, resFifoPath);
        // Detach so that the listener runs in the background.
        listenerThread.detach();
    }

    // Stop the dispatcher: signal stop, join worker threads and listener thread.
    void stop() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        cv.notify_all();
        for (auto &t : threads) {
            if (t.joinable()) t.join();
        }
        if (listenerThread.joinable())
            listenerThread.join();
    }

private:
    // Worker thread function: processes tasks from the queue.
    void workerThread() {
        while (true) {
            PrioritizedTask prioritizedTask(ExtendedTask(PING, 0, 0, ""), 0); // Default initialization.
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this]{
                    return stopFlag || (!taskQueue.empty() && !specialTaskActive);
                });
                if (stopFlag) break;
                if (specialTaskActive) continue;
                prioritizedTask = taskQueue.top();
                taskQueue.pop();
            }
            
            // Process task based on type.
            switch (prioritizedTask.task.type) {
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
    void processSpecialTask(const ExtendedTask& task) {
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
bool create_threads(unsigned int num_threads) {
    return dispatcherImpl.createThreads(num_threads);
}

// New: Expose the start() function so that the dispatcher begins listening.
void start_listening(const std::string& reqFifoPath, const std::string& resFifoPath) {
    dispatcherImpl.start(reqFifoPath, resFifoPath);
}

// --- Additional internal functions for testing ---
// These functions are not part of the original header interface but are useful for adding tasks and stopping the dispatcher.
#ifdef DISPATCHER_TEST
// Expose addTask for testing purposes.
void addTaskForTesting(const ExtendedTask& task) {
    dispatcherImpl.addTask(task);
}

// Expose stop functionality.
void stopDispatcher() {
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
