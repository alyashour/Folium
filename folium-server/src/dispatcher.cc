#include "dispatcher.h"

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "util.h"
#include "logger.h"
#include "f_task.h"
#include "fifo_channel.h"

using namespace dispatcher;

constexpr double MIN_SLEEP = 2, MAX_SLEEP = 3; // seconds

Dispatcher::Dispatcher(ipc::FifoChannel &in, ipc::FifoChannel &out, const unsigned int numThreads)
    : in_(in), out_(out)
{
    // pong the gateway
    F_Task task;
    in_.read(task);

    Logger::log("Dispatch Pong!");
    out_.send(F_Task(F_TaskType::PING));

    createThreadPool(numThreads);
}

void Dispatcher::createThreadPool(const unsigned int numThreads) {
    // reserve space in the vector
    threadPool_.reserve(numThreads);

    // create the worker threads
    for (int i = 0; i < numThreads; i++) {
        threadPool_.emplace_back(&Dispatcher::processInboundTasks, this);
    }
}


// Start the listening process
void Dispatcher::start() {
    std::vector<bool> threadBusy(threadPool_.size(), false);
    std::vector<F_Task> currentTasks(threadPool_.size());
    std::mutex taskMutex;
    std::condition_variable taskCV;
    running_ = true;

    // main loop to read from input
    while (running_) {
        F_Task task;

        if (in_.read(task)) {
            // got a task!
            Logger::log("Task recieved at dispatch");

            // if its a syskill task
            if (task.type_ == F_TaskType::SYSKILL) {
                running_ = false;
                break;
            }

            // assign it to an idle thread
            std::unique_lock<std::mutex> lock(taskMutex);

            // check for idle thread
            int idleThreadId = -1;
            for (int i = 0; i < threadBusy.size(); i++) {
                if (!threadBusy[i]) {
                    idleThreadId = i;
                    break;
                }
            }

            if (idleThreadId != -1) {
                // assign task to idle thread
                threadBusy[idleThreadId] = true;
                currentTasks[idleThreadId] = task;

                // signal thread to process task
                taskCV.notify_one();
            } else {
                // no idle thread found
                // add to queue based on priority
                {
                    std::lock_guard<std::mutex> queueLock(queueMutex_);
                    taskQueue_.push(task);
                }

                // check for preemption
                int lowestPriorityThreadId = -1;
                int lowestPriority = -1;

                for (int i = 0; i < threadBusy.size(); i++) {
                    if (threadBusy[i]) {
                        int currentPriority = currentTasks[i].getPriority();
                        if (lowestPriorityThreadId == -1 || currentPriority > lowestPriority) {
                            lowestPriority = currentPriority;
                            lowestPriorityThreadId = i;
                        }
                    }
                }

                if (lowestPriorityThreadId != -1 &&
                    task.getPriority() < lowestPriority) {
                        // signal the threads
                }
            }
        }
    }
}

F_Task processTask(F_Task& task) {
    Logger::logS("Processing task: ", task.type_);
    Logger::logS("Done processing task: ", task.type_);

    return task;
}


void Dispatcher::processInboundTasks() {
    while (running_) {
        F_Task task;
        bool hasTask = false;

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!taskQueue_.empty()) {
                task = taskQueue_.top();
                taskQueue_.pop();
                hasTask = true;
            }
        }

        if (hasTask) {
            // process the task
            F_Task result = processTask(task);

            out_.send(result);
        } else {
            // no task is available, sleep to avoid busy waiting
            util::randomSleep(MIN_SLEEP, MAX_SLEEP);
        }
    }
}