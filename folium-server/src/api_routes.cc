#include "api_routes.h"
#include "auth.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void setup_routes(httplib::Server& svr) {

    // Register
    svr.Post("/auth/register", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string username = data["username"];
            std::string password = data["password"];

            if (Auth::registerUser(username, password)) {
                res.set_content(R"({"status": "registered"})", "application/json");
            } else {
                res.status = 409;
                res.set_content(R"({"error": "User already exists"})", "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    // Login
    svr.Post("/auth/login", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string username = data["username"];
            std::string password = data["password"];

            if (Auth::check_credentials(username, password)) {
                Auth::login(username);
                std::string token = "token_for_" + username;
                res.set_content(json({{"token", token}}).dump(), "application/json");
            } else {
                res.status = 401;
                res.set_content(R"({"error": "Invalid credentials"})", "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    // Logout
    svr.Post("/auth/logout", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string username = data["username"];
            Auth::logout(username);
            res.set_content(R"({"status": "logged out"})", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    // Token Validation
    svr.Post("/auth/validate", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string token = data["token"];
            bool valid = Auth::validateToken(token);
            res.set_content(json({{"valid", valid}}).dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    // Token Refresh
    svr.Post("/auth/refresh", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string token = data["token"];
            std::string refreshed = Auth::refreshToken(token);

            if (!refreshed.empty()) {
                res.set_content(json({{"refreshed_token", refreshed}}).dump(), "application/json");
            } else {
                res.status = 401;
                res.set_content(R"({"error": "Invalid token"})", "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    //
    svr.Post("/auth/change-password", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string username = data["username"];
            std::string oldPassword = data["old_password"];
            std::string newPassword = data["new_password"];
    
            if (Auth::changePassword(username, oldPassword, newPassword)) {
                res.set_content(R"({"status": "Password changed successfully"})", "application/json");
            } else {
                res.status = 403;
                res.set_content(R"({"error": "Password change failed"})", "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });

    svr.Post("/auth/secure-action", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = json::parse(req.body);
            std::string username = data["username"];
            std::string token = data["token"];
    
            if (!Auth::validateToken(token)) {
                res.status = 403;
                res.set_content(R"({"error": "Invalid token"})", "application/json");
                return;
            }
    
            if (Auth::checkPermissions(username, Auth::CAN_CREATE_NOTE, Auth::USER)) {
                res.set_content(R"({"status": "Permission granted"})", "application/json");
            } else {
                res.status = 403;
                res.set_content(R"({"error": "Permission denied"})", "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid request format"})", "application/json");
        }
    });
    
}
