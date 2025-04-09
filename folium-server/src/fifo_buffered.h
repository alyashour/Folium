#pragma once

#include "logger.h"
#include "fifo_channel.h"

namespace ipc {

class BufferedSender {
public:
    BufferedSender(FifoChannel& channel, size_t maxInFlight)
        : channel_(channel), maxTasks_(maxInFlight), inFlight_(0) {}

    bool trySend(const Task& task) {
        if (inFlight_ >= maxTasks_) {
            std::cerr << "⚠️ BufferedSender: Queue full. Dropping Task " << task.id << "\n";
            return false;
        }

        if (channel_.send(task)) {
            inFlight_++;
            return true;
        }

        std::cerr << "❌ BufferedSender: Failed to send Task " << task.id << "\n";
        return false;
    }

    void markCompleted() {
        if (inFlight_ > 0) --inFlight_;
    }

    size_t inFlight() const { return inFlight_; }

private:
    FifoChannel& channel_;
    size_t maxTasks_;
    size_t inFlight_;
};

class BufferedReceiver {
public:
    BufferedReceiver(FifoChannel& channel) : channel_(channel) {}

    bool tryReceive(Task& task) {
        return channel_.read(task);
    }

private:
    FifoChannel& channel_;
};

} // namespace ipc
