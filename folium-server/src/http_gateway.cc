#include "http_gateway.h"
#include "api_routes.h"

#include <string>
#include <thread>
#include <iostream>
#include <exception>

#include "httplib.h"
#include "nlohmann/json.hpp"

#include "logger.h"
#include "auth.h"
#include "fifo_channel.h"

using namespace gateway;
using json = nlohmann::json;

/**
 * @brief Extracts a JWT from a request.
 * 
 *
 * @param req 
 * @return The jwt (string) if found. 
 */
std::string extractJWT(const httplib::Request& req) {
    auto authHeader = req.get_header_value("Authorization");

    if (authHeader.empty()) {
        throw std::exception();
    }

    // check if it's a bearer token
    if (authHeader.substr(0, 7) == "Bearer ") {
        return authHeader.substr(7);
    }

    return "";
}

/**
 * @brief Helper initialize routes func.
 * 
 *
 * @param svr The HTTP server instance
 */
void initializeRoutes(httplib::Server& svr)
{
    /* GET ROUTES */

    // ping
    svr.Get("/ping", [](const httplib::Request&, httplib::Response& res) { 
        Logger::log("Gateway: GET /ping.");

        res.set_content(
            "Pong!\n",
            "text/plain"
        ); 
    });

    /* POST ROUTES */

    // register
    svr.Post("/api/auth/register", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/register");


    });

    // log in
    svr.Post("/api/auth/login", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/login");
    });

    // log out
    svr.Post("/api/auth/logout", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/logout");

    });
}

Gateway::Gateway(ipc::FifoChannel in, ipc::FifoChannel out)
    :in_(in), out_(out)
{
    initializeRoutes(svr);
}

Gateway::~Gateway() {
    this->stop();
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