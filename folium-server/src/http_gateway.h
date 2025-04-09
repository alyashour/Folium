/**
 * @file http_gateway.h
 * @brief HTTP Gateway for the Folium server.
 *
 * This file defines the interface for the HTTP Gateway, which is responsible
 * for handling incoming HTTP requests and routing them to the appropriate
 * server components. It listens on a specified IP and port for client connections.
 *
 * @section Responsibilities
 * - Listen for incoming HTTP requests.
 * - The entry point for client (Ledger)-server communication.
 */

#ifndef FOLSERV_HTTP_GATEWAY_H_
#define FOLSERV_HTTP_GATEWAY_H_

#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>

#include "httplib.h"

#include "logger.h"
#include "f_task.h"
#include "fifo_channel.h"

namespace gateway
{
    class Gateway
    {
    private:
        std::thread serverThread;
        httplib::Server svr;

        ipc::FifoChannel in_, out_;

        // Helper method to process tasks through FIFO channels
        F_Task processTaskAndWaitForResponse(const F_Task &task, int timeoutMs = 5000)
        {
            // Send task to out channel
            bool sent = out_.send(task);
            if (!sent)
            {
                Logger::log("Gateway: Failed to send task to processing service");
                F_Task errorTask;
                errorTask.type = F_TaskType::ERROR;
                errorTask.data = {{"status", "error"}, {"message", "IPC communication failure"}};
                return errorTask;
            }

            // Wait for response on in channel with timeout
            auto start = std::chrono::steady_clock::now();
            while (true)
            {
                // Check if we've exceeded timeout
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeoutMs)
                {
                    Logger::log("Gateway: Response timeout");
                    F_Task timeoutTask;
                    timeoutTask.type = F_TaskType::ERROR;
                    timeoutTask.data = {{"status", "error"}, {"message", "Response timeout"}};
                    return timeoutTask;
                }

                // Check if data is available to read
                if (in_.hasData(10))
                { // Poll with 10ms timeout
                    // Read response
                    F_Task response;
                    bool read = in_.read(response);
                    if (!read)
                    {
                        Logger::log("Gateway: Failed to read response");
                        F_Task readErrorTask;
                        readErrorTask.type = F_TaskType::ERROR;
                        readErrorTask.data = {{"status", "error"}, {"message", "Failed to read response"}};
                        return readErrorTask;
                    }
                    return response; // Return the complete F_Task
                }

                // Short sleep to prevent CPU hogging
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

    public:
        Gateway(ipc::FifoChannel in, ipc::FifoChannel out);
        ~Gateway();

        /**
         * Tells the coupled process to shut down (through the fifo pipe).
         */
        void signal_shutdown();

        /**
         * @brief Start listening for incoming HTTP requests in another thread.
         * @param ip The IP address to bind the server to.
         * @param port The port to listen on.
         */
        void listen(std::string ip, int port);

        /**
         * @brief Stops the running server.
         */
        void stop();
    };
}

#endif // FOLSERV_HTTP_GATEWAY_H_