#include "http_gateway.h"

#include <string>
#include <thread>
#include <iostream>

#include "httplib.h"
#include "logger.h"

static httplib::Server svr;

using namespace gateway;

Gateway::Gateway()
{
    this->initializeRoutes();
}

Gateway::~Gateway() {
    this->stop();
}

void Gateway::initializeRoutes()
{
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) { 
            res.set_content(
                "Hello World!\n",
                "text/plain"
            ); 
        }
    );
}

void Gateway::listen(std::string ip, int port)
{
    Logger::log("HTTP Gateway starting on " + ip + ":" + std::to_string(port));

    // if already running, don't run again
    if (serverThread.joinable()) return;

    // start the server in member thread
    serverThread = std::thread([this, ip, port]() {
        this->svr.listen(ip, port);
    });
}

void Gateway::stop() {
    svr.stop(); // stop the server

    // join the thread
    if (serverThread.joinable()) {
        serverThread.join();
        Logger::log("HTTP Gateway thread stopped");
    }
}