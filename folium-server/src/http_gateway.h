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

#include "httplib.h"

#include <string>
#include <thread>
#include <atomic>

namespace gateway
{
    class Gateway
    {
    private:
        std::thread serverThread;
        httplib::Server svr;
    public:
        Gateway();
        ~Gateway();

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