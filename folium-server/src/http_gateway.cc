#include "http_gateway.h"

#include <string>
#include <thread>
#include <iostream>
#include <exception>

#include "httplib.h"
#include "nlohmann/json.hpp"

#include "logger.h"
#include "auth.h"
#include "jwt-cpp/jwt.h"

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
            "Hello World!\n",
            "text/plain"
        ); 
    });

    /* POST ROUTES */

    // register
    svr.Post("/api/auth/register", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/register");

        try {
            // get data
            json json_data = nlohmann::json::parse(req.body);

            // pull data from body
            std::string username = json_data["username"];
            std::string password = json_data["password"];

            // perform the registration
            const bool success = auth::registerUser(username, password);

            // send response
            json response;
            if (success) {
                res.status = 200;
                response = {
                    {"message", "Success!"},
                    // TODO: update auth to return a user id instead of success
                    {"userId", "NOT IMPLEMENTED YET!"}
                };
            } else {
                res.status = 400;
                response = {
                    {"error", "Failed to register user. No additional information."}
                };
            }

            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            const json response = {
                {"error", e.what()}
            };

            res.set_content(response.dump(), "application/json");
        }
    });

    // log in
    svr.Post("/api/auth/login", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/login");

        try {
            // get data
            json json_data = nlohmann::json::parse(req.body);

            // pull data from body
            std::string username = json_data["username"];
            std::string password = json_data["password"];

            // perform the registration
            // TODO: wait on hamza for jwt out of this function
            // auth::login(username, password);
            //...
        } catch (const std::exception& e) {
            //...
        }

        json response = {
            {"error", "Not yet implemented."}
        };

        res.status = 501;
        res.set_content(response.dump(), "application/json");
    });

    // log out
    svr.Post("/api/auth/logout", [](const httplib::Request& req, httplib::Response& res) {
        Logger::log("Gateway: POST /api/auth/logout");

        try {
            // get data
            json json_data = nlohmann::json::parse(req.body);

            res.set_content("", "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            const json response = {
                {"error", e.what()}
            };

            res.set_content(response.dump(), "application/json");
        }
    });
}

Gateway::Gateway()
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