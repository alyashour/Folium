/**
 * NOT THREAD SAFE.
 * There can only be 1 gateway running at a time in a single process.
 */
#include "http_gateway.h"

#include <string>
#include <thread>
#include <iostream>
#include <exception>

#include "httplib.h"
#include "nlohmann/json.hpp"

#include "logger.h"
#include "auth.h"
#include "fifo_channel.h"

using json = nlohmann::json;

using namespace gateway;

/**
 * @brief Extracts a JWT from a request.
 *
 *
 * @param req
 * @return The jwt (string) if found.
 */
std::string extractJWT(const httplib::Request &req)
{
    auto authHeader = req.get_header_value("Authorization");

    if (authHeader.empty())
    {
        throw std::exception();
    }

    // check if it's a bearer token
    if (authHeader.substr(0, 7) == "Bearer ")
    {
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
void Gateway::initializeRoutes(httplib::Server &svr)
{
    Logger::log("Instantiating Routes...");

    /* GET ROUTES */

    // ping
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res)
    { 
        Logger::log("Gateway: GET /ping.");

        res.set_content(
            "Pong!\n",
            "text/plain"
        ); 
    });

    // ping-core
    svr.Get("/ping-core", [this](const httplib::Request &, httplib::Response &res)
    { 
        Logger::log("Gateway: GET /ping-core.");

        // create task
        F_Task task;
        task.type_ = F_TaskType::PING;

        F_Task outputTask = processTaskAndWaitForResponse(task);

        if (outputTask.type_ == F_TaskType::ERROR) {
            res.status = 400;
        } else {
            res.status = 200;
        }

        // TEMP
        json response = {
            {"message", "pong!"}
        };

        res.set_content(response.dump(), "application/json");
    });

    /* POST ROUTES */

    // register
    svr.Post("/api/auth/register", [this](const httplib::Request &req, httplib::Response &res) {
        Logger::log("Gateway: POST /api/auth/register");

        try {
            json json_data = json::parse(req.body);

            std::string username = json_data["username"];
            std::string password = json_data["password"];

            // create task
            F_Task task;
            task.type_ = F_TaskType::REGISTER;
            task.data_ = {
                {"username", username},
                {"password", password}
            };

            F_Task outputTask = processTaskAndWaitForResponse(task);

            if (outputTask.type_ == F_TaskType::ERROR) {
                res.status = 400;
            } else {
                res.status = 200;
            }
            
            // check presence of necessary outputs
            if (!outputTask.data_.contains("message") || !outputTask.data_.contains("userId")) {
                Logger::logErr("FATAL! WRONG FORMAT. MAKE SURE REGISTER TASK OUTPUTS CORRECT VALUES!");
                throw std::runtime_error("Fatal");
            }

            json response = {
                {"message", outputTask.data_["message"]},
                {"userId", outputTask.data_["userId"]}
            };

            res.set_content(response.dump(), "application/json");
            
        } catch (const std::exception& e) {
            res.status = 400;
            const json response = {
                {"error", e.what()}
            };

            res.set_content(response.dump(), "application/json");

            return;
        } 
    });

    // log in
    svr.Post("/api/auth/login", [this](const httplib::Request &req, httplib::Response &res){ 
        Logger::log("Gateway: POST /api/auth/login"); 

        try {
            json json_data = json::parse(req.body);

            std::string username = json_data["username"];
            std::string password = json_data["password"];

            // create task
            F_Task task;
            task.type_ = F_TaskType::REGISTER;
            task.data_ = {
                {"username", username},
                {"password", password}
            };

            // TEMP should have 2 threads instead
            F_Task response = processTaskAndWaitForResponse(task);

            if (response.type_ == F_TaskType::ERROR) {
                res.status = 400;
            } else {
                res.status = 200;
            }

            res.set_content(response.data_.dump(), "application/json");
            
        } catch (const std::exception& e) {
            res.status = 400;
            const json response = {
                {"error", e.what()}
            };

            res.set_content(response.dump(), "application/json");

            return;
        } 
    });

    // log out
    svr.Post("/api/auth/logout", [](const httplib::Request &req, httplib::Response &res)
             { Logger::log("Gateway: POST /api/auth/logout"); });

    Logger::log("Done instantiating routes.");
}

Gateway::Gateway(ipc::FifoChannel& in, ipc::FifoChannel& out)
    : in_(in), out_(out)
{
    // should be debug
    Logger::log("Gateway constructor called.");

    // try to ping the dispatch to make sure connection is made
    Logger::log("Gateway Ping!");
    out_.send(F_Task(F_TaskType::PING));

    F_Task task;
    bool success = in_.read(task);

    if (!success) {
        throw std::runtime_error("Couldn't connect to dispatch");
    }
    Logger::log("Gateway-Dispatch handshake complete!");

    initializeRoutes(svr);
}

Gateway::~Gateway()
{
    this->stop();
}

void Gateway::listen(std::string ip, int port)
{
    Logger::log("HTTP Gateway starting on " + ip + ":" + std::to_string(port));

    // if already running, don't run again
    if (serverThread.joinable())
        return;

    // start the server in member thread
    serverThread = std::thread([this, ip, port]()
                               { this->svr.listen(ip, port); });
}

void Gateway::stop()
{
    svr.stop(); // stop the server

    // join the thread
    if (serverThread.joinable())
    {
        serverThread.join();
        Logger::log("HTTP Gateway thread stopped");
    }
}

/**
 * @brief Processes a task by sending it to the user.
 * @param task
 * @param timeoutMs
 */
F_Task Gateway::processTaskAndWaitForResponse(const F_Task &task, int timeoutMs)
{
    // Send task to out channel
    bool sent = out_.send(task);
    if (!sent)
    {
        Logger::log("Gateway: Failed to send task to processing service");
        F_Task errorTask;
        errorTask.type_ = F_TaskType::ERROR;
        errorTask.data_ = {{"status", "error"}, {"message", "IPC communication failure"}};
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
            timeoutTask.type_ = F_TaskType::ERROR;
            timeoutTask.data_ = {{"status", "error"}, {"message", "Response timeout"}};
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
                readErrorTask.type_ = F_TaskType::ERROR;
                readErrorTask.data_ = {{"status", "error"}, {"message", "Failed to read response"}};
                return readErrorTask;
            }
            return response; // Return the complete F_Task
        }

        // Short sleep to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Gateway::signal_shutdown() {
    out_.send(F_Task(F_TaskType::SYSKILL));
}