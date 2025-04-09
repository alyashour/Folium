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

        /**
         * Initializes the gateway's routes.
         */
        void initializeRoutes(httplib::Server &svr);

        /**
         * Processes a single task and returns a response
         */
        F_Task processTaskAndWaitForResponse(const F_Task &task, int timeoutMs = 5000);
    public:
        /**
         * @brief Creates an http gateway connected with dispatch through pipes.
         */
        Gateway(ipc::FifoChannel& in, ipc::FifoChannel& out);
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