/**
 * @file dispatcher.h
 * @brief Dispatcher module for managing thread pools in the Folium server.
 *
 * This file defines the interface for creating and managing thread pools, which
 * are used to handle concurrent tasks efficiently in the server. The dispatcher
 * ensures that tasks are distributed across threads for optimal performance.
 *
 * @section Responsibilities
 * - Create and manage thread pools.
 * - Handle incoming IPC tasks via FIFO channels.
 */

#ifndef FOLSERV_DISPATCHER_H_
#define FOLSERV_DISPATCHER_H_

#include <string>
#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "f_task.h"       // Assumes that Task_Type and F_Task are defined.
#include "fifo_channel.h" // Assumes ipc::FifoChannel and ipc::Task are defined.
namespace Dispatcher
{

    // Forward declaration of the internal ExtendedTask used in testing.
    // Its full definition is hidden in dispatcher.cpp.
    struct ExtendedTask;

    /**
     * @brief Creates a thread pool with the specified number of threads.
     * @param num_threads The number of threads to create in the thread pool.
     * @return True if the thread pool was successfully created, false otherwise.
     * @throws std::runtime_error if the thread pool cannot be created.
     */


struct PrioritizedTask
    {
        ExtendedTask* task;
        int priority; // Lower value indicates higher priority.

        PrioritizedTask(ExtendedTask* t, int p)
            : task(t), priority(p) {}
    };

        // Exposed comparator.
    struct TaskComparator {
        bool operator()(const PrioritizedTask &a, const PrioritizedTask &b) {
            return a.priority > b.priority; // Lower priority value comes first.
        }
    };

    

    class DispatcherImpl
    {
    public:
        // Constructor now takes FIFO paths for requests and responses.
        DispatcherImpl(ipc::FifoChannel &reqFifo, ipc::FifoChannel &resFifo);

        ~DispatcherImpl();

        // Create the thread pool. This function corresponds to the header declaration.
        bool createThreads(unsigned int numThreads);

        // Start the listener thread. The callback will be invoked for each IPC task received.
        // The FIFO paths are already stored in the object.
        void startListener(const std::function<void(const F_Task &)> &callback);

        // Add a task to the dispatcher.
        void addTask(const ExtendedTask &task);

    private:
        // Listener function that continuously reads IPC tasks.
        void listen(const std::function<void(const F_Task &)> &callback);

        // New function: Start the listener on a separate thread.
        void start(const std::string &reqFifoPath, const std::string &resFifoPath);

        // Stop the dispatcher: signal stop, join worker threads and listener thread.
        void stop();

    private:
        // Worker thread function: processes tasks from the queue.
        void workerThread();

        // Processes a special CREATE_NOTE task exclusively.
        void processSpecialTask(const ExtendedTask &task);

        bool create_threads(unsigned int num_threads);

        void start_listening(const std::string &reqFifoPath, const std::string &resFifoPath);

        // Internal members.
        std::priority_queue<PrioritizedTask, std::vector<PrioritizedTask>, TaskComparator> taskQueue;
        std::vector<std::thread> threads;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stopFlag;
        bool specialTaskActive;
        ipc::FifoChannel requestFifo, responseFifo;
        std::thread listenerThread;
    };

    /**
     * @brief Starts the dispatcher listener, which waits for incoming IPC tasks.
     *
     * The listener uses the provided FIFO paths for request and response and
     * invokes the callback for each task received.
     *
     * @param reqFifoPath The path to the FIFO used for incoming requests.
     * @param resFifoPath The path to the FIFO used for sending responses.
     * @param callback A callback function to be invoked for each incoming IPC task.
     */

#ifdef DISPATCHER_TEST
    // These additional functions are provided for testing purposes.
    void addTaskForTesting(const ExtendedTask &task);
    void stopDispatcher();
#endif

} // namespace Dispatcher

#endif // FOLSERV_DISPATCHER_H_
