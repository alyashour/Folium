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
    

    // Comparator so that tasks with lower numerical priority come first.
    struct TaskComparator
    {
        bool operator()(const PrioritizedTask &a, const PrioritizedTask &b)
        {
            return a.priority > b.priority;
        }
    };

    // --- Dispatcher Implementation ---

    // Constructor now takes FIFO paths for requests and responses.
    DispatcherImpl::DispatcherImpl(ipc::FifoChannel &reqFifo, ipc::FifoChannel &resFifo)
        : requestFifo(reqFifo), responseFifo(resFifo),
          stopFlag(false), specialTaskActive(false)
    {
    }
    DispatcherImpl::~DispatcherImpl() { stop(); }

    // Create the thread pool. This function corresponds to the header declaration.
    bool DispatcherImpl::createThreads(unsigned int numThreads)
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

    

    // Start the listener thread. The callback will be invoked for each IPC task received.
    // The FIFO paths are already stored in the object.
    void DispatcherImpl::startListener(const std::function<void(const ipc::Task &)> &callback)
    {
        listenerThread = std::thread(&DispatcherImpl::listen, this, callback);
    }

    // Add a task to the dispatcher.
    void DispatcherImpl::addTask(const ExtendedTask &task)
    {
        int priority = getStaticPriority(task.type);
        if (task.type == CREATE_NOTE)
        {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                specialTaskActive = true; // Stall other tasks.
            }
            // Process special task in its own temporary thread.
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
            std::unique_lock<std::mutex> lock(queueMutex);
            taskQueue.push(PrioritizedTask(task, priority));
            cv.notify_one();
        }
    }

    // Listener function that continuously reads IPC tasks.
    void DispatcherImpl::listen(const std::function<void(const ipc::Task &)> &callback)
    {
        // Open FIFO channels (blocking mode, auto-create if needed is assumed)
        ipc::Task ipcTask;
        while (!stopFlag)
        {
            if (requestFifo.read(ipcTask))
            {
                std::cout << "[Dispatcher] Received IPC Task: ";
                ipcTask.print();

                // Invoke the callback to notify the application.
                callback(ipcTask);

                // Convert the ipc::Task to an ExtendedTask.
                Task_Type type = static_cast<Task_Type>(ipcTask.id);
                ExtendedTask extTask(type, ipcTask.id, 0, ipcTask.payload);
                addTask(extTask);

                ipcTask.completed = true;
                responseFifo.send(ipcTask);
            }
        }
    }

    // New function: Start the listener on a separate thread.
    void DispatcherImpl::start(const std::string &reqFifoPath, const std::string &resFifoPath)
    {
        std::thread listenerThread(&DispatcherImpl::listen, this, reqFifoPath, resFifoPath);
        // Detach so that the listener runs in the background.
        listenerThread.detach();
    }

    // Stop the dispatcher: signal stop, join worker threads and listener thread.
    void DispatcherImpl::stop()
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
        if (listenerThread.joinable())
            listenerThread.join();
    }

    // Worker thread function: processes tasks from the queue.
    void DispatcherImpl::workerThread()
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
    void DispatcherImpl::processSpecialTask(const ExtendedTask &task)
    {
        std::cout << "[Dispatcher] Processing special CREATE_NOTE task exclusively.\n";
        Core::createBigNote(task.noteID, task.userID, task.noteData, "Default Title");
        std::cout << "[Dispatcher] Special CREATE_NOTE task completed.\n";
    }

    // --- Public interface as declared in the header file ---
    bool DispatcherImpl::create_threads(unsigned int num_threads)
    {
        return this->createThreads(num_threads);
    }

    // New: Expose the start() function so that the dispatcher begins listening.
    void DispatcherImpl::start_listening(const std::string &reqFifoPath, const std::string &resFifoPath)
    {
        this->start(reqFifoPath, resFifoPath);
    }

}
