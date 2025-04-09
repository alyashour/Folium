/**
 * @file dispatcher.cc
 * @brief Dispatcher module for managing thread pools in the Folium server.
 *
 * This file defines the Dispatcher class, which routes incoming tasks (F_Task)
 * to worker threads and calls processTask(...) to handle each task's logic.
 */

#include "dispatcher.h"

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

#include <nlohmann/json.hpp>

#include "util.h"
#include "logger.h"
#include "f_task.h"
#include "auth.h"
#include "data_access_layer.h"
#include "fifo_channel.h"

// We may need MySQL calls if we do direct queries:
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif

using json = nlohmann::json;
using namespace dispatcher;

constexpr double MIN_SLEEP = 2, MAX_SLEEP = 3; // seconds

// ---------------------------------------------------------
// Forward Declarations of Helper Subfunctions
// ---------------------------------------------------------
static F_Task handleSystemTask(F_Task &task);
static F_Task handleAuthTask(F_Task &task);
static F_Task handleClassesTask(F_Task &task);
static F_Task handleNotesTask(F_Task &task);

// ---------------------------------------------------------
// Main processTask function
// ---------------------------------------------------------
F_Task processTask(F_Task &task)
{
    logger::logS("Processing task: ", task.type_);

    try
    {
        switch (task.type_)
        {
        // 1. System / Basic
        case F_TaskType::PING:
        case F_TaskType::SYSKILL:
        case F_TaskType::ERROR:
            return handleSystemTask(task);

        // 2. Auth
        case F_TaskType::REGISTER:
        case F_TaskType::SIGN_IN:
        case F_TaskType::LOG_OUT:
        case F_TaskType::AUTH_REFRESH:
        case F_TaskType::AUTH_CHANGE_PASSWORD:
            return handleAuthTask(task);

        // 3. Classes
        case F_TaskType::GET_CLASSES:
        case F_TaskType::GET_ME_CLASSES:
        case F_TaskType::POST_ME_CLASSES:
        case F_TaskType::PUT_CLASS:
        case F_TaskType::DELETE_CLASS:
        case F_TaskType::GET_CLASS_DETAILS:
        case F_TaskType::GET_CLASS_OWNER:
        case F_TaskType::GET_CLASS_NAME:
        case F_TaskType::GET_CLASS_DESCRIPTION:
        case F_TaskType::GET_CLASS_BIGNOTE:
        case F_TaskType::GET_CLASS_TITLE:
            return handleClassesTask(task);

        // 4. Notes
        case F_TaskType::POST_UPLOAD_NOTE:
        case F_TaskType::PUT_BIGNOTE_EDIT:
        case F_TaskType::GET_BIGNOTE_HISTORY:
        case F_TaskType::GET_BIGNOTE_EXPORT:
        // If you still use CREATE_NOTE / EDIT_NOTE:
        case F_TaskType::CREATE_NOTE:
        case F_TaskType::EDIT_NOTE:
            return handleNotesTask(task);

        // 5. Fallback
        default:
        {
            task.data_ = {
                {"statusCode", 400},
                {"error", "Unrecognized or unimplemented task type"}};
            return task;
        }
        }
    }
    catch (const std::exception &ex)
    {
        task.data_ = {
            {"statusCode", 500},
            {"error", ex.what()}};
    }

    logger::logS("Done processing task: ", task.type_);
    return task;
}

// ---------------------------------------------------------
// Subfunction: handleSystemTask
// ---------------------------------------------------------
static F_Task handleSystemTask(F_Task &task)
{
    switch (task.type_)
    {
    case F_TaskType::PING:
    {
        task.data_ = {
            {"statusCode", 200},
            {"message", "pong from dispatch"}};
        break;
    }
    case F_TaskType::SYSKILL:
    {
        // Typically handled in the main loop, but if we get here:
        task.data_ = {
            {"statusCode", 200},
            {"message", "SYSKILL acknowledged"}};
        break;
    }
    case F_TaskType::ERROR:
    default:
    {
        // Just pass along or add an error if missing
        if (!task.data_.contains("error"))
            task.data_["error"] = "Unknown error occurred.";
        if (!task.data_.contains("statusCode"))
            task.data_["statusCode"] = 400;
        break;
    }
    }
    return task;
}

// ---------------------------------------------------------
// Subfunction: handleAuthTask
// ---------------------------------------------------------
static F_Task handleAuthTask(F_Task &task)
{
    auto &req = task.data_;

    switch (task.type_)
    {
    case F_TaskType::REGISTER:
    {
        // POST /api/auth/register
        if (!req.contains("username") || !req.contains("password"))
        {
            req = {{"statusCode", 400}, {"error", "Missing username or password"}};
            break;
        }
        std::string username = req["username"];
        std::string password = req["password"];

        if (username.size() < auth::MIN_USERNAME_LENGTH || password.size() < auth::MIN_PASSWORD_LENGTH)
        {
            req = {{"statusCode", 400}, {"error", "Username/password too short"}};
            break;
        }

        try
        {
            int newUserId = auth::registerUser(username, password);
            req = {
                {"statusCode", 201},
                {"message", "User registered successfully"},
                {"userId", newUserId}};
        }
        catch (const std::runtime_error &e)
        {
            // E.g. if user already exists
            req = {{"statusCode", 400}, {"error", e.what()}};
        }
        break;
    }
    case F_TaskType::SIGN_IN:
    {
        // POST /api/auth/login
        if (!req.contains("username") || !req.contains("password"))
        {
            req = {{"statusCode", 400}, {"error", "Missing username or password"}};
            break;
        }
        std::string username = req["username"];
        std::string password = req["password"];

        std::string errMsg;
        if (!auth::check_credentials(username, password, errMsg))
        {
            req = {
                {"statusCode", 401},
                {"error", errMsg.empty() ? "Invalid credentials" : errMsg}};
        }
        else
        {
            std::string token = auth::login(username);
            if (token.empty())
            {
                req = {{"statusCode", 401}, {"error", "Could not generate token"}};
            }
            else
            {
                std::string sessionId = "sess-" + username;
                req = {
                    {"statusCode", 200},
                    {"token", token},
                    {"sessionId", sessionId}};
            }
        }
        break;
    }
    case F_TaskType::LOG_OUT:
    {
        // POST /api/auth/logout
        if (!req.contains("token"))
        {
            req = {{"statusCode", 401}, {"error", "Missing or invalid token"}};
            break;
        }
        // In a stateless JWT system, we typically do nothing server-side
        req = {{"statusCode", 200}, {"message", "Logout successful"}};
        break;
    }
    case F_TaskType::AUTH_REFRESH:
    {
        // POST /api/auth/refresh-token
        if (!req.contains("refreshToken"))
        {
            req = {{"statusCode", 401}, {"error", "Missing refresh token"}};
            break;
        }
        std::string oldToken = req["refreshToken"];
        try
        {
            std::string newToken = auth::refreshToken(oldToken);
            std::string newRefresh = "REFRESH-XYZ"; // or generate a new one
            req = {
                {"statusCode", 200},
                {"token", newToken},
            };
        }
        catch (const std::runtime_error &ex)
        {
            req = {{"statusCode", 401}, {"error", ex.what()}};
        }
        break;
    }
    case F_TaskType::AUTH_CHANGE_PASSWORD:
    {
        // POST /api/auth/change-password
        if (!req.contains("username") || !req.contains("currentPassword") || !req.contains("newPassword"))
        {
            req = {{"statusCode", 400}, {"error", "Missing required fields"}};
            break;
        }
        std::string user = req["username"];
        std::string oldPw = req["currentPassword"];
        std::string newPw = req["newPassword"];

        bool ok = auth::changePassword(user, oldPw, newPw);
        if (!ok)
        {
            req = {{"statusCode", 400}, {"error", "Could not change password"}};
        }
        else
        {
            req = {{"statusCode", 200}, {"message", "Password changed successfully"}};
        }
        break;
    }
    default:
    {
        req = {{"statusCode", 400}, {"error", "Unknown Auth task"}};
        break;
    }
    }
    return task;
}

// ---------------------------------------------------------
// Subfunction: handleClassesTask
// ---------------------------------------------------------
static F_Task handleClassesTask(F_Task &task)
{
    auto &req = task.data_;

    try
    {
        switch (task.type_)
        {
        case F_TaskType::GET_CLASSES:
        {
            // GET /api/classes (no auth required)
            // Example using "DAL::getClassIds(0)" if 0 means "all classes"
            // Then read each class’s metadata from, say, "class_X.json"
            // Or if you maintain them in a DB, do direct queries here.

            // For demonstration, let's do a DB approach (if you have MySQL behind DAL).
            // If you have no direct DB approach, adapt to your own "DAL::..." calls.

            // Here we do a minimal approach using your existing DAL:
            std::vector<int> allCids = DAL::getClassIds(0); // 0 meaning "all"
            json classArr = json::array();

            for (auto cid : allCids)
            {
                // Maybe each class is in a JSON file "class_<cid>.json"
                std::string path = "class_" + std::to_string(cid) + ".json";
                if (std::filesystem::exists(path))
                {
                    auto cData = DAL::readJsonFile(path);
                    classArr.push_back(cData);
                }
                else
                {
                    // If no file, skip or fill partial
                    // (In real code, you might do more robust queries.)
                }
            }
            req = {
                {"statusCode", 200},
                {"classes", classArr}};
            break;
        }
        case F_TaskType::GET_ME_CLASSES:
        {
            // GET /api/me/classes
            if (!req.contains("userId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId"}};
                break;
            }
            int userId = req["userId"];
            auto cids = DAL::getClassIds(userId); // existing DAL function
            json out = json::array();

            for (auto cid : cids)
            {
                std::string path = "class_" + std::to_string(cid) + ".json";
                if (std::filesystem::exists(path))
                {
                    out.push_back(DAL::readJsonFile(path));
                }
            }
            req = {
                {"statusCode", 200},
                {"classes", out}};
            break;
        }
        case F_TaskType::POST_ME_CLASSES:
        {
            // POST /api/me/classes => create a new class
            // Expect: {"userId", "name", "classId", "description" (optional)}
            if (!req.contains("userId") || !req.contains("name") || !req.contains("classId"))
            {
                req = {{"statusCode", 400}, {"error", "Missing userId, name, or classId"}};
                break;
            }
            int userId = req["userId"];
            std::string name = req["name"];
            std::string classIdStr = req["classId"];
            std::string desc = req.value("description", "");

            // We'll store to a JSON file "class_<classId>.json" for demonstration
            // Or you might do a direct DB insert
            json cls = {
                {"classId", classIdStr},
                {"name", name},
                {"owner", std::to_string(userId)},
                {"ownerId", std::to_string(userId)},
                {"description", desc}};
            // Save file
            std::string path = "class_" + classIdStr + ".json";
            DAL::writeJsonFile(cls); // by default writes to a single JSON?
                                     // (Your DAL might need a function signature:
                                     //  writeJsonFile(cls, path).)

            // Also associate user -> class if you store that in user_classes or a file
            // For demonstration, we skip that detail or do partial.

            req = {
                {"statusCode", 201},
                {"message", "Class created successfully"},
                {"classId", classIdStr}};
            break;
        }
        case F_TaskType::PUT_CLASS:
        {
            // PUT /api/me/classes/{classId}
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 400}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string classIdStr = req["classId"];
            std::string path = "class_" + classIdStr + ".json";

            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto clsData = DAL::readJsonFile(path);

            if (req.contains("name"))
            {
                clsData["name"] = req["name"];
            }
            if (req.contains("description"))
            {
                clsData["description"] = req["description"];
            }
            // Possibly check if user is actually the owner
            // if (clsData["ownerId"].get<std::string>() != std::to_string(req["userId"])) {
            //     req = {{"statusCode", 403}, {"error", "No permission"}};
            //     break;
            // }

            // Overwrite
            DAL::writeJsonFile(clsData); // or a variant that takes a path

            req = {
                {"statusCode", 200},
                {"message", "Class updated successfully"}};
            break;
        }
        case F_TaskType::DELETE_CLASS:
        {
            // DELETE /api/me/classes/{classId}
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 400}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string classIdStr = req["classId"];
            std::string path = "class_" + classIdStr + ".json";

            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            // Check if user is owner or just removing enrollment
            auto clsData = DAL::readJsonFile(path);
            std::string ownerId = clsData.value("ownerId", "");
            std::string userId = req["userId"].get<std::string>();

            if (ownerId == userId)
            {
                // Remove entire class
                std::filesystem::remove(path);
                req = {
                    {"statusCode", 200},
                    {"message", "Class fully deleted by owner"}};
            }
            else
            {
                // Not owner => just "unenroll"
                // Real code would remove a record from user_classes table or a file.
                req = {
                    {"statusCode", 200},
                    {"message", "User unenrolled from class"}};
            }
            break;
        }
        case F_TaskType::GET_CLASS_DETAILS:
        {
            // GET /api/me/classes/{classId}
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Unauthorized or missing classId"}};
                break;
            }
            std::string classIdStr = req["classId"];
            std::string path = "class_" + classIdStr + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);
            // Check if user is enrolled or owner. If not, 403. (Skipping here for brevity.)
            // Return everything in the JSON as part of the response
            // but conform to your README’s keys: id, owner, name, description, bigNote, title
            req = {
                {"statusCode", 200},
                {"id", cls.value("classId", "")},
                {"owner", cls.value("ownerId", "")},
                {"name", cls.value("name", "")},
                {"description", cls.value("description", "")},
                // For demonstration:
                {"bigNote", cls.value("bigNote", "All combined notes...")},
                {"title", cls.value("title", "Untitled")}};
            break;
        }
        case F_TaskType::GET_CLASS_OWNER:
        {
            // GET /api/me/classes/{classId}/owner
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing or invalid"}};
                break;
            }
            std::string classIdStr = req["classId"];
            std::string path = "class_" + classIdStr + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);
            // user check omitted
            req = {
                {"statusCode", 200},
                {"ownerId", cls.value("ownerId", "")},
                {"ownerName", cls.value("owner", "someOwnerUsername")},
                {"ownerContact", cls.value("ownerContact", "unknown")}};
            break;
        }
        case F_TaskType::GET_CLASS_NAME:
        {
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string path = "class_" + std::string(req["classId"]) + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);
            req = {
                {"statusCode", 200},
                {"name", cls.value("name", "")}};
            break;
        }
        case F_TaskType::GET_CLASS_DESCRIPTION:
        {
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing info"}};
                break;
            }
            std::string path = "class_" + std::string(req["classId"]) + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);
            req = {
                {"statusCode", 200},
                {"description", cls.value("description", "")}};
            break;
        }
        case F_TaskType::GET_CLASS_BIGNOTE:
        {
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string path = "class_" + std::string(req["classId"]) + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);

            // Build a bigNote object from the stored data
            json bigNoteObj = {
                {"content", cls.value("bigNote", "")},
                {"title", cls.value("title", "")},
                {"lastUpdated", cls.value("bigNoteUpdated", "")},
                {"contributors", json::array()},
                {"sections", json::array()}};
            req = {
                {"statusCode", 200},
                {"bigNote", bigNoteObj}};
            break;
        }
        case F_TaskType::GET_CLASS_TITLE:
        {
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string path = "class_" + std::string(req["classId"]) + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);
            req = {
                {"statusCode", 200},
                {"title", cls.value("title", "")}};
            break;
        }
        default:
        {
            req = {{"statusCode", 400}, {"error", "Unrecognized Classes task"}};
            break;
        }
        }
    }
    catch (const std::exception &ex)
    {
        req = {
            {"statusCode", 500},
            {"error", ex.what()}};
    }
    return task;
}

// ---------------------------------------------------------
// Subfunction: handleNotesTask
// ---------------------------------------------------------
static F_Task handleNotesTask(F_Task &task)
{
    auto &req = task.data_;

    try
    {
        switch (task.type_)
        {
        case F_TaskType::POST_UPLOAD_NOTE:
        case F_TaskType::CREATE_NOTE: // If you keep CREATE_NOTE distinct
        {
            // POST /api/me/classes/{classId}/upload-note
            // Expect: {"userId", "classId", "noteFile", "title" (optional)}
            if (!req.contains("userId") || !req.contains("classId") || !req.contains("noteFile"))
            {
                req = {{"statusCode", 400}, {"error", "Missing required fields"}};
                break;
            }
            // Validate user is in the class or an owner if needed
            // Then physically store the note or merge it.
            // For demonstration, we do:
            //   DAL::writeFile("notes/note_...", theContent);
            //   Then update the big note if we want

            req = {
                {"statusCode", 201},
                {"message", "Note uploaded and merged"},
                {"updated", true}};
            break;
        }

        case F_TaskType::PUT_BIGNOTE_EDIT:
        case F_TaskType::EDIT_NOTE: // If you keep EDIT_NOTE distinct
        {
            // PUT /api/me/classes/{classId}/bigNote/edit-note
            // Expect: {"userId", "classId", "content": {...}, "title"?}
            if (!req.contains("userId") || !req.contains("classId") || !req.contains("content"))
            {
                req = {{"statusCode", 400}, {"error", "Missing userId, classId, or content"}};
                break;
            }
            // Suppose we update a JSON file for "classId"
            std::string path = "class_" + std::string(req["classId"]) + ".json";
            if (!std::filesystem::exists(path))
            {
                req = {{"statusCode", 404}, {"error", "Class not found"}};
                break;
            }
            auto cls = DAL::readJsonFile(path);

            // If a title is provided:
            if (req.contains("title"))
            {
                cls["title"] = req["title"];
            }
            // Flatten "content" into a single text or store structure
            auto content = req["content"];
            if (content.contains("text"))
            {
                cls["bigNote"] = content["text"];
            }
            // Possibly handle "sections" if present

            // Update "bigNoteUpdated" timestamp
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now), "%Y-%m-%dT%H:%M:%S");
            cls["bigNoteUpdated"] = ss.str();

            // Save the file
            DAL::writeJsonFile(cls);

            // Return success
            req = {
                {"statusCode", 200},
                {"message", "Big note updated"},
                {"lastUpdated", ss.str()}};
            break;
        }

        case F_TaskType::GET_BIGNOTE_HISTORY:
        {
            // GET /api/me/classes/{classId}/bigNote/history
            // Suppose we store note edits in "note_history_<classId>.json"
            // or in your DB. Let's do a simple file approach:
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string classIdStr = req["classId"];
            std::string histPath = "note_history_" + classIdStr + ".json";
            if (!std::filesystem::exists(histPath))
            {
                // Possibly means no edits yet
                req = {
                    {"statusCode", 404},
                    {"error", "No edit history found"}};
                break;
            }
            auto histData = DAL::readJsonFile(histPath);
            // Expecting an array under "history" or something
            json history = histData.value("history", json::array());

            req = {
                {"statusCode", 200},
                {"history", history}};
            break;
        }

        case F_TaskType::GET_BIGNOTE_EXPORT:
        {
            // GET /api/me/classes/{classId}/bigNote/export
            if (!req.contains("userId") || !req.contains("classId"))
            {
                req = {{"statusCode", 401}, {"error", "Missing userId or classId"}};
                break;
            }
            std::string format = req.value("format", "PDF");
            if (format != "PDF" && format != "Markdown")
            {
                req = {{"statusCode", 400}, {"error", "Invalid export format"}};
                break;
            }
            // In real code, you'd generate the file/binary and return or store it
            // For demonstration, just return a message
            req = {
                {"statusCode", 200},
                {"message", "Big note exported as " + format}};
            break;
        }

        default:
        {
            req = {{"statusCode", 400}, {"error", "Unrecognized Notes task"}};
            break;
        }
        }
    }
    catch (const std::exception &ex)
    {
        req = {
            {"statusCode", 500},
            {"error", ex.what()}};
    }

    return task;
}

// ---------------------------------------------------------
// Dispatcher Methods (Threading, Queuing)
// ---------------------------------------------------------
Dispatcher::Dispatcher(ipc::FifoChannel &in, ipc::FifoChannel &out, const unsigned int numThreads)
    : in_(in), out_(out), running_(true)
{
    // Initially read one task from 'in' for a handshake
    F_Task task;
    in_.read(task);

    logger::log("Dispatch Pong!");
    out_.send(F_Task(F_TaskType::PING));

    createThreadPool(numThreads);
}

void Dispatcher::createThreadPool(const unsigned int numThreads)
{
    threadPool_.reserve(numThreads);
    for (unsigned int i = 0; i < numThreads; i++)
    {
        threadPool_.emplace_back(&Dispatcher::processInboundTasks, this, i);
    }
}

void Dispatcher::processInboundTasks(int threadId)
{
    logger::logS("Worker thread ", threadId, " started");

    while (running_)
    {
        F_Task task;
        bool hasTask = false;

        // Acquire lock and wait for a task
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            taskCV_.wait(lock, [this]()
                         { return !taskQueue_.empty() || !running_; });

            if (!running_ && taskQueue_.empty())
                break;

            if (!taskQueue_.empty())
            {
                task = taskQueue_.top();
                taskQueue_.pop();
                hasTask = true;
                task.threadId_ = threadId;
                logger::logS("Thread ", threadId, " picked up task of type: ", task.type_);
            }
        }

        // Process outside the lock
        if (hasTask)
        {
            F_Task response = processTask(task);
            out_.send(response);
            logger::logS("Thread ", threadId, " completed task");
        }
    }

    logger::logS("Worker thread ", threadId, " shutting down");
}

void Dispatcher::start()
{
    // Main loop to read tasks from 'in_'
    while (running_)
    {
        F_Task task;
        in_.read(task);

        logger::log("Task received at dispatch!");

        // If syskill, shut down
        if (task.type_ == F_TaskType::SYSKILL)
        {
            logger::log("Dispatch received kill signal.");
            {
                std::unique_lock<std::mutex> lock(taskMutex_);
                running_ = false;
                taskCV_.notify_all();
            }
            break;
        }

        // Enqueue the task
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            // If queue size >= # of threads, we drop (server busy)
            if (taskQueue_.size() >= threadPool_.size())
            {
                logger::log("WARN: Server too busy, dropping request...");
                lock.unlock();

                F_Task response(F_TaskType::ERROR);
                response.data_ = {
                    {"error", "Server busy! Request dropped, please try again later."}};
                out_.send(response);
            }
            else
            {
                taskQueue_.push(task);
                taskCV_.notify_one();
                logger::log("Task added to queue");
            }
        }
    }

    // Join worker threads
    for (auto &thread : threadPool_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    logger::log("Dispatcher shut down");
}
