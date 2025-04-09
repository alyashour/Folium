#pragma once

#include <string>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <csignal>
#include <vector>

namespace ipc
{

    static std::vector<std::string> registeredFifos;

    inline void create_fifo(const std::string &path)
    {
        if (!std::filesystem::exists(path))
        {
            if (mkfifo(path.c_str(), 0666) < 0)
            {
                perror(("mkfifo failed: " + path).c_str());
                throw std::runtime_error("Failed to create FIFO: " + path);
            }
        }
        registeredFifos.push_back(path);
    }

    inline void delete_fifo(const std::string &path)
    {
        if (std::filesystem::exists(path))
        {
            unlink(path.c_str());
        }
    }

    inline void cleanup_all_fifos()
    {
        for (const auto &fifo : registeredFifos)
        {
            delete_fifo(fifo);
        }
    }

    // 🛡️ Scoped RAII Fifo Guard
    class ScopedFifo
    {
    public:
        ScopedFifo(const std::string &path) : path_(path)
        {
            create_fifo(path_);
        }

        ~ScopedFifo()
        {
            delete_fifo(path_);
        }

    private:
        std::string path_;
    };

    // ☠️ Clean up on Ctrl+C
    inline void install_signal_handler()
    {
        std::signal(SIGINT, [](int)
                    {
        std::cout << "\n🧹 Caught SIGINT. Cleaning up FIFOs...\n";
        cleanup_all_fifos();
        std::_Exit(0); });
    }

} // namespace ipc
