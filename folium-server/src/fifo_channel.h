#pragma once

#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <poll.h>  // Add this for polling

#include "f_task.h"

namespace ipc
{

    class FifoChannel
    {
    public:
        FifoChannel(const std::string &path, int flags, bool create = true)
            : path_(path), fd_(-1)
        {
            if (create)
            {
                if (mkfifo(path.c_str(), 0666) == -1 && errno != EEXIST)
                {
                    throw std::runtime_error("mkfifo failed: " + path);
                }
            }

            fd_ = open(path.c_str(), flags);
            if (fd_ == -1)
            {
                throw std::runtime_error("Failed to open FIFO: " + path);
            }
        }

        ~FifoChannel()
        {
            if (fd_ != -1)
            {
                close(fd_);
            }
        }

        bool send(const F_Task&task)
        {
            ssize_t n = ::write(fd_, &task, sizeof(F_Task));
            return n == sizeof(F_Task);
        }

        bool read(F_Task &task)
        {
            ssize_t n = ::read(fd_, &task, sizeof(F_Task));
            return n == sizeof(F_Task);
        }
        
        // Add this new method to check if data is available
        bool hasData(int timeout_ms = 0) const
        {
            struct pollfd pfd;
            pfd.fd = fd_;
            pfd.events = POLLIN;
            
            int res = poll(&pfd, 1, timeout_ms);
            
            if (res < 0) {
                // Error occurred
                return false;
            }
            
            // Return true if POLLIN is set, indicating data available to read
            return (res > 0) && (pfd.revents & POLLIN);
        }

    private:
        std::string path_;
        int fd_;
    };

} // namespace ipc
