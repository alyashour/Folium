// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "http_gateway.h"

#include <string>
#include <iostream>

#include "httplib.h"

#include "logger.h"

static httplib::Server svr;

void Gateway::listen(std::string ip, int port)
{
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(
            "Hello World!\n", 
            "text/plain"
        );
    });

    Logger::log("HTTP Gateway starting on " + ip + ":" + std::to_string(port));
    std::cout << "Type 'exit' to stop the server" << std::endl;
    svr.listen(ip, port);
    Logger::log("HTTP Gateway stopped");
}

void Gateway::stop() {
    svr.stop();
}