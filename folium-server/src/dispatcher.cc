#include "dispatcher.h"

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>

#include "util.h"
#include "logger.h"
#include "f_task.h"
#include "fifo_channel.h"

using namespace dispatcher;

constexpr double MIN_SLEEP = 2, MAX_SLEEP = 3; // seconds

F_Task processTask(F_Task &task)
{
    logger::logS("Processing task: ", task.type_);
    
    // Ensure PING tasks get proper responses
    if (task.type_ == F_TaskType::PING) {
        task.data_ = {{"status", "success"}, {"message", "pong from dispatch"}};
        logger::log("Created PING response with data payload");
    }
    
    logger::logS("Done processing task: ", task.type_);
    return task;
}

Dispatcher::Dispatcher(ipc::FifoChannel &in, ipc::FifoChannel &out, const unsigned int numThreads)
    : in_(in), out_(out), running_(true)
{
    // pong the gateway
    F_Task task;
    in_.read(task);

    logger::log("Dispatch Pong!");
    out_.send(F_Task(F_TaskType::PING));

    createThreadPool(numThreads);
}

void Dispatcher::createThreadPool(const unsigned int numThreads)
{
    // reserve space in the vector
    threadPool_.reserve(numThreads);

    // create the worker threads
    for (int i = 0; i < numThreads; i++)
    {
        threadPool_.emplace_back(&Dispatcher::processInboundTasks, this, i);
    }
}

// Worker thread function to process tasks
void Dispatcher::processInboundTasks(int threadId)
{
    logger::logS("Worker thread ", threadId, " started");
    
    while (running_)
    {
        F_Task task;
        bool hasTask = false;
        
        // Get a task from the queue
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            
            // Wait until there's a task or we're shutting down
            taskCV_.wait(lock, [this]() { 
                return !taskQueue_.empty() || !running_; 
            });
            
            // If we're shutting down and there are no tasks, exit
            if (!running_ && taskQueue_.empty())
                break;
                
            if (!taskQueue_.empty()) {
                task = taskQueue_.top();
                taskQueue_.pop();
                hasTask = true;
                task.threadId_ = threadId;
                logger::logS("Thread ", threadId, " picked up task of type: ", task.type_);
            }
        }
        
        // Process the task outside the lock
        if (hasTask) {
            F_Task response = processTask(task);
            out_.send(response);
            logger::logS("Thread ", threadId, " completed task");
        }
    }
    
    logger::logS("Worker thread ", threadId, " shutting down");
}

// Start the listening process
void Dispatcher::start()
{
    // main loop to read from input
    while (running_)
    {
        F_Task task;
        in_.read(task);
        
        logger::log("Task received at dispatch!");

        // if it's a syskill task
        if (task.type_ == F_TaskType::SYSKILL)
        {
            logger::log("Dispatch recieved kill signal.");
            // Signal threads to shut down
            {
                std::unique_lock<std::mutex> lock(taskMutex_);
                running_ = false;
                taskCV_.notify_all();
            }
            break;
        }

        // Add the task to the queue for worker threads
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            
            // Check if we're at max capacity
            if (taskQueue_.size() >= threadPool_.size()) {
                logger::log("WARN: Server too busy, dropping request...");
                // Release lock before sending response
                lock.unlock();
                
                // Return a message that the server is busy and the task was dropped
                F_Task response(F_TaskType::ERROR);
                response.data_ = {
                    {"error", "Server busy! Request dropped, please try again later."}
                };
                out_.send(response);
            } else {
                // Add task to queue and notify a waiting thread
                taskQueue_.push(task);
                taskCV_.notify_one();
                logger::log("Task added to queue");
            }
        }
    }
    
    // Wait for all threads to finish
    for (auto& thread : threadPool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    logger::log("Dispatcher shut down");
}