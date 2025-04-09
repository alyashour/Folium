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

#include "f_task.h"
#include "fifo_channel.h"
#include "core.h"

namespace dispatcher
{

    // --- Wrapper to attach a priority to each task ---
    // Comparator so that tasks with lower numerical priority come first.
    struct TaskComparator
    {
        bool operator()(const F_Task& a, const F_Task& b)
        {
            return a.getPriority() > b.getPriority();
        }
    };

    class Dispatcher
    {
    private:
        std::vector<std::thread> threadPool_;
        std::priority_queue<F_Task, std::vector<F_Task>, TaskComparator> taskQueue_;
        std::atomic<bool> running_ = false; // tells the threads to stop

        std::mutex queueMutex_;
        ipc::FifoChannel in_, out_;

        // initializes the thread pool
        void createThreadPool(const unsigned int numThreads);

        // the function that threads run
        void processInboundTasks();
    public:
        // Constructor now takes FIFO paths for requests and responses.
        Dispatcher(ipc::FifoChannel &in, ipc::FifoChannel &out, const unsigned int numThreads);

        // New function: Start the listener on a separate thread.
        void start();
    };
}

#endif // FOLSERV_DISPATCHER_H_
