#include "auth.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>  // Make sure this header is available

using json = nlohmann::json;

namespace Auth {

    struct MySQLConfig {
        std::string host;
        int port;
        std::string user;
        std::string password;
        std::string database;
    };

    // Helper function to load MySQL configuration from dbConfig.json
    MySQLConfig loadDbConfig(const std::string& configFile = "../dbConfig.json") {
        MySQLConfig config;
        std::ifstream file(configFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file: " + configFile);
        }
        json j;
        file >> j;
        config.host = j.value("mysql_host", "127.0.0.1");
        config.port = j.value("mysql_port", 3306);
        config.user = j.value("mysql_user", "root");
        config.password = j.value("mysql_password", "");
        config.database = j.value("mysql_database", "folium");
        return config;
    }

    // Helper function to get a MySQL connection using configuration from dbConfig.json
    std::unique_ptr<sql::Connection> getConnection() {
        try {
            MySQLConfig config = loadDbConfig(); // load config from file
            sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
            std::string connectionString = "tcp://" + config.host + ":" + std::to_string(config.port);
            std::unique_ptr<sql::Connection> conn(
                driver->connect(connectionString, config.user, config.password)
            );
            conn->setSchema(config.database); // Use your database name
            return conn;
        } catch (sql::SQLException &e) {
            std::cerr << "SQLException in getConnection(): " << e.what() << std::endl;
            throw;
        }
    }

    bool check_credentials(const std::string& username, const std::string& password) {
        try {
            auto conn = getConnection();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT password FROM users WHERE username = ?")
            );
            pstmt->setString(1, username);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next()) {
                std::string storedPassword = res->getString("password");
                // For now, we simply compare plain text; in production, use proper hashing.
                return storedPassword == password;
            }
            return false;
        } catch (sql::SQLException &e) {
            std::cerr << "SQLException in check_credentials: " << e.what() << std::endl;
            return false;
        }
    }

    void login(const std::string& username) {
        std::cout << "User " << username << " logged in (MySQL backend)." << std::endl;
    }

    void logout(const std::string& username) {
        std::cout << "User " << username << " logged out (MySQL backend)." << std::endl;
    }

    bool registerUser(const std::string& username, const std::string& password) {
        try {
            auto conn = getConnection();
            // Check if user already exists
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT COUNT(*) AS count FROM users WHERE username = ?")
            );
            pstmt->setString(1, username);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next() && res->getInt("count") > 0) {
                return false;
            }
            // Insert new user with default role "user" (or "admin" if desired)
            pstmt.reset(conn->prepareStatement("INSERT INTO users (username, password, role) VALUES (?, ?, ?)"));
            pstmt->setString(1, username);
            pstmt->setString(2, password); // In production, hash the password
            pstmt->setString(3, (username == "admin") ? "admin" : "user");
            pstmt->executeUpdate();
            return true;
        } catch (sql::SQLException &e) {
            std::cerr << "SQLException in registerUser: " << e.what() << std::endl;
            return false;
        }
    }

    bool validateToken(const std::string& token) {
        // Placeholder implementation for JWT validation
        return true;
    }

    std::string refreshToken(const std::string& token) {
        // Placeholder: simply append a suffix
        return token + "_refreshed";
    }

    std::string hashPassword(const std::string& password) {
        // In production, implement a proper hashing function
        return password;
    }

    bool verifyPassword(const std::string& hashedPassword, const std::string& plainPassword) {
        return hashedPassword == plainPassword;
    }

    Role getUserRole(const std::string& username) {
        try {
            auto conn = getConnection();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn->prepareStatement("SELECT role FROM users WHERE username = ?")
            );
            pstmt->setString(1, username);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next()) {
                std::string roleStr = res->getString("role");
                if (roleStr == "admin")
                    return ADMIN;
            }
        } catch (sql::SQLException &e) {
            std::cerr << "SQLException in getUserRole: " << e.what() << std::endl;
        }
        return USER;
    }

    bool checkPermissions(const std::string& username, const Permission permission, const Role role) {
        if (getUserRole(username) == ADMIN)
            return true;
        return false;
    }
}
