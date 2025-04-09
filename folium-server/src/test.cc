// dispatcher.cpp
#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#include <functional>
#include "dispatcher.h"
#include "f_task.h"
#include "core.h"
#include "fifo_channel.h"

namespace Dispatcher {

// --- Internal extended task structure ---
// Extends the base F_Task (from f_task.h) to include parameters for note operations.
struct ExtendedTask : public F_Task {
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
int getStaticPriority(Task_Type type) {
    switch (type) {
        case PING:
            return 5;
        case CREATE_NOTE:
            return 1;
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
struct PrioritizedTask {
    ExtendedTask task;
    int priority;

    PrioritizedTask(const ExtendedTask &t, int p)
        : task(t), priority(p) {}
};

// Comparator: lower numerical priority values are processed first.
struct TaskComparator {
    bool operator()(const PrioritizedTask &a, const PrioritizedTask &b) {
        return a.priority > b.priority;
    }
};

// --- Dispatcher Implementation ---
class DispatcherImpl {
public:
    // Constructor takes FIFO paths.
    DispatcherImpl(const std::string &reqFifo, const std::string &resFifo)
      : requestFifo(reqFifo), responseFifo(resFifo),
        stopFlag(false), specialTaskActive(false)
    {}

    ~DispatcherImpl() {
        stop();
    }

    // Create the worker thread pool.
    bool createThreads(unsigned int numThreads) {
        if (numThreads == 0)
            throw std::runtime_error("Number of threads must be greater than 0");
        for (unsigned int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&DispatcherImpl::workerThread, this);
        }
        return true;
    }

    // Start the listener thread using the FIFO paths stored in the object.
    // The callback is called each time an IPC task is received.
    void startListener(const std::function<void(const ipc::Task&)>& callback) {
        listenerThread = std::thread(&DispatcherImpl::listen, this, callback);
    }

    // Add a task to the dispatcher.
    void addTask(const ExtendedTask &task) {
        int priority = getStaticPriority(task.type);
        if (task.type == CREATE_NOTE) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                specialTaskActive = true;  // Stall other tasks.
            }
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

    // Stop the dispatcher, joining all worker and listener threads.
    void stop() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        cv.notify_all();
        for (auto &t : threads) {
            if (t.joinable())
                t.join();
        }
        if (listenerThread.joinable())
            listenerThread.join();
    }

private:
    // The listener function that continuously reads IPC tasks.
    void listen(const std::function<void(const ipc::Task&)>& callback) {
        ipc::FifoChannel requestChannel(requestFifo, O_RDONLY, true);
        ipc::FifoChannel responseChannel(responseFifo, O_WRONLY, true);
        ipc::Task ipcTask;
        while (!stopFlag) {
            if (requestChannel.read(ipcTask)) {
                std::cout << "[Dispatcher] Received IPC Task: ";
                ipcTask.print();
                callback(ipcTask);

                Task_Type type = static_cast<Task_Type>(ipcTask.id);
                ExtendedTask extTask(type, ipcTask.id, 0, ipcTask.payload);
                addTask(extTask);

                ipcTask.completed = true;
                responseChannel.send(ipcTask);
            }
        }
    }

    // Worker thread: processes tasks from the priority queue.
    void workerThread() {
        while (true) {
            PrioritizedTask prioritizedTask(ExtendedTask(PING, 0, 0, ""), 0);
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this]{ return stopFlag || (!taskQueue.empty() && !specialTaskActive); });
                if (stopFlag)
                    break;
                if (specialTaskActive)
                    continue;
                prioritizedTask = taskQueue.top();
                taskQueue.pop();
            }
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

    // Process a CREATE_NOTE task exclusively.
    void processSpecialTask(const ExtendedTask &task) {
        std::cout << "[Dispatcher] Processing special CREATE_NOTE task exclusively.\n";
        Core::createNote(task.noteID, task.userID, task.noteData);
        std::cout << "[Dispatcher] Special CREATE_NOTE task completed.\n";
    }

    // FIFO paths stored in the object.
    std::string requestFifo;
    std::string responseFifo;

    std::priority_queue<PrioritizedTask, std::vector<PrioritizedTask>, TaskComparator> taskQueue;
    std::vector<std::thread> threads;
    std::thread listenerThread;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool stopFlag;
    bool specialTaskActive;
};

// --- Public Interface Functions ---
static std::unique_ptr<DispatcherImpl> dispatcherImpl;

bool create_threads(unsigned int num_threads) {
    // For simplicity, create the dispatcherImpl if it doesn't exist.
    // You may later want to initialize it with configurable FIFO paths.
    if (!dispatcherImpl) {
        // Here we hard-code the FIFO paths; in production, pass via configuration.
        dispatcherImpl = std::make_unique<DispatcherImpl>("/tmp/dispatcher_req.fifo", "/tmp/dispatcher_res.fifo");
    }
    return dispatcherImpl->createThreads(num_threads);
}

void start_listening(const std::string &reqFifo, const std::string &resFifo,
                     const std::function<void(const ipc::Task&)>& callback) {
    // If dispatcherImpl is not created yet, create it with given FIFO paths.
    if (!dispatcherImpl) {
        dispatcherImpl = std::make_unique<DispatcherImpl>(reqFifo, resFifo);
    }
    dispatcherImpl->startListener(callback);
}

#ifdef DISPATCHER_TEST
void addTaskForTesting(const ExtendedTask &task) {
    dispatcherImpl->addTask(task);
}

void stopDispatcher() {
    dispatcherImpl->stop();
}
#endif

} // namespace Dispatcher

// Example main() for testing (if DISPATCHER_TEST is defined):
/*
#ifdef DISPATCHER_TEST
#include <chrono>
int main() {
    using namespace Dispatcher;
    create_threads(4);
    start_listening("/tmp/dispatcher_req.fifo", "/tmp/dispatcher_res.fifo",
                    [](const ipc::Task &task) { std::cout << "[Test Callback] Received IPC Task\n"; });
    // Add some tasks...
    ExtendedTask pingTask(PING, 0, 0, "");
    addTaskForTesting(pingTask);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stopDispatcher();
    return 0;
}
#endif
*/
