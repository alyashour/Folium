// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http_gateway.h"
#include "api_routes.h"

#include <atomic>
#include <thread>
#include <string>
#include <iostream>

#include "httplib.h"

#include "logger.h"

static std::atomic<bool> should_stop(false);
static httplib::Server svr;
static std::thread server_thread;
static std::thread input_thread;

void static joinThreads() {
    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (input_thread.joinable()) {
        input_thread.join();
    }
}

void Gateway::listen(std::string ip, int port)
{
    should_stop = false;
    
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(
            "Hello World!\n", 
            "text/plain"
        );
    });
    // setuping the API routes
    setup_routes(svr);
    
    // Start server in a separate thread
    server_thread = std::thread([ip, port]() {
        Logger::log("Server starting on " + ip + ":" + std::to_string(port));
        std::cout << "Type 'exit' to stop the server" << std::endl;
        svr.listen(ip, port);
        std::cout << "Server stopped" << std::endl;
    });
    
    // Start input thread to listen for exit command
    input_thread = std::thread([]() {
        std::string input;
        while (!should_stop) {
            std::getline(std::cin, input);
            if (input == "exit") {
                std::cout << "Exit command received, shutting down..." << std::endl;
                should_stop = true;
                break;
            }
        }
    });

    // Check periodically if we should stop
    while (!should_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Signal server to stop and wait for threads to finish
    svr.stop();
    joinThreads();
}