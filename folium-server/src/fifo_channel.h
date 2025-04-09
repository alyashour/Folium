#pragma once

#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <filesystem>

namespace ipc
{

    constexpr int MAX_PAYLOAD = 256;

    inline void create_fifo(const std::string& path) {
        namespace fs = std::filesystem;
        if (!fs::exists(path)) {
            if (mkfifo(path.c_str(), 0666) < 0) {
                perror(("mkfifo failed for " + path).c_str());
                throw std::runtime_error("Failed to create FIFO: " + path);
            }
        }
    }
    
    inline void delete_fifo(const std::string& path) {
        namespace fs = std::filesystem;
        if (fs::exists(path)) {
            unlink(path.c_str());
        }
    }
    
    inline void setup_fifos(const std::string& path1, const std::string& path2) {
        create_fifo(path1);
        create_fifo(path2);
    }
    
    inline void cleanup_fifos(const std::string& path1, const std::string& path2) {
        delete_fifo(path1);
        delete_fifo(path2);
    }

    struct Task
    {
        int id;
        char payload[MAX_PAYLOAD];
        bool completed; // 0 = pending, 1 = done

        void print() const
        {
            std::cout << "Task[" << id << "] (" << (completed ? "✓" : "✗") << "): " << payload << "\n";
        }
    };

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

        bool send(const Task &task)
        {
            ssize_t n = ::write(fd_, &task, sizeof(Task));
            return n == sizeof(Task);
        }

        bool read(Task &task)
        {
            ssize_t n = ::read(fd_, &task, sizeof(Task));
            return n == sizeof(Task);
        }

    private:
        std::string path_;
        int fd_;
    };

} // namespace ipc
