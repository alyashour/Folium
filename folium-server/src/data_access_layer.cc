/**
 * @file data_access_layer.cc
 * @brief Implementation of the Data Access Layer (DAL) for the Folium server.
 *
 * This file implements functions to interact with a MySQL database and perform file I/O
 * (both plain text and JSON). The database connection parameters are read from the
 * local "dbConfig.json" file (see expected structure below).
 *
 * Expected dbConfig.json:
 * {
 *   "mysql_host": "127.0.0.1",
 *   "mysql_port": 3306,
 *   "mysql_user": "root",
 *   "mysql_password": "",
 *   "mysql_database": "folium"
 * }
 *
 * All functions use robust error handling:
 *   - Errors are logged via the new Logger instance.
 *   - Exceptions are thrown to ensure the caller is informed.
 *   - No function fails silently.
 *
 * File I/O operations now use per-file mutexes to ensure that each open file is independently
 * protected, enabling concurrent operations on different files.
 */

#include "data_access_layer.h"
#include "logger.h"
#include <mysql/mysql.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <memory>
#include <iostream> // Add this line for std::cerr

//----------------------------------------------------------------------
// Global per-file mutex management
//----------------------------------------------------------------------

namespace {
    // Global mutex protects access to the file mutex map.
    std::mutex globalFileMutexMapLock;
    // Map each file path to its dedicated mutex.
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> fileMutexMap;

    /**
     * @brief Retrieve a mutex for the given file path.
     *
     * This function returns the shared pointer to the mutex for the file.
     * If no mutex exists for the file, one is created.
     *
     * @param filePath The file path.
     * @return A shared_ptr to the mutex guarding the file.
     */
    std::shared_ptr<std::mutex> getFileMutex(const std::string& filePath) {
        std::lock_guard<std::mutex> lock(globalFileMutexMapLock);
        auto it = fileMutexMap.find(filePath);
        if (it == fileMutexMap.end()) {
            auto mtx = std::make_shared<std::mutex>();
            fileMutexMap[filePath] = mtx;
            return mtx;
        }
        return it->second;
    }
}

//----------------------------------------------------------------------
// Create a dedicated logger for DAL
//----------------------------------------------------------------------

static logger::Logger dalLogger("DAL");

//----------------------------------------------------------------------
// Database Configuration and Connection
//----------------------------------------------------------------------

namespace DAL {

    /**
     * @brief A simple struct to hold DB connection parameters.
     */
    struct DBConfig {
        std::string host;
        unsigned int port;
        std::string user;
        std::string password;
        std::string database;
    };

    /**
     * @brief Retrieve database configuration from dbConfig.json.
     *
     * Loads the connection parameters using static storage so the file is read only once.
     *
     * @return A DBConfig struct containing the configuration.
     */
    static DBConfig getDbConfig() {
        static bool loaded = false;
        static DBConfig config;
        if (!loaded) {
            std::ifstream configFile("dbConfig.json");
            if (!configFile.is_open()) {
                dalLogger.logErr("getDbConfig: Unable to open dbConfig.json. Check file path.");
                throw std::runtime_error("getDbConfig: Unable to open dbConfig.json.");
            }
            nlohmann::json j;
            try {
                configFile >> j;
            } catch (const std::exception &e) {
                dalLogger.logErr("getDbConfig: JSON parse error: " + std::string(e.what()));
                throw;
            }
            config.host     = j.value("mysql_host", "127.0.0.1");
            config.port     = j.value("mysql_port", 3306);
            config.user     = j.value("mysql_user", "root");
            config.password = j.value("mysql_password", "");
            config.database = j.value("mysql_database", "folium");
            loaded = true;
        }
        return config;
    }

    /**
     * @brief Create and return a MySQL connection.
     *
     * Uses connection parameters from dbConfig.json. If the connection fails,
     * an exception is thrown.
     *
     * @return MYSQL* pointer to the MySQL connection.
     */
    static MYSQL* createConnection() {
        DBConfig cfg;
        try {
            cfg = getDbConfig();
        } catch (...) {
            throw std::runtime_error("createConnection: Failed to load database configuration.");
        }
        MYSQL *conn = mysql_init(nullptr);
        if (!conn) {
            dalLogger.logErr("createConnection: mysql_init() failed.");
            throw std::runtime_error("createConnection: mysql_init() failed.");
        }
        if (!mysql_real_connect(
                conn,
                cfg.host.c_str(),
                cfg.user.c_str(),
                cfg.password.c_str(),
                cfg.database.c_str(),
                cfg.port,
                nullptr,
                0))
        {
            std::string errMsg = mysql_error(conn);
            dalLogger.logErr("createConnection: mysql_real_connect() failed: " + errMsg);
            mysql_close(conn);
            throw std::runtime_error("createConnection: mysql_real_connect() failed: " + errMsg);
        }
        return conn;
    }

    //----------------------------------------------------------------------    
    // SQL Query Functions
    //----------------------------------------------------------------------

    /**
     * @brief Retrieve the list of database tables.
     *
     * Executes "SHOW TABLES" on the MySQL server. If an error occurs,
     * an exception is thrown.
     *
     * @return A vector of strings containing table names.
     */
    std::vector<std::string> getTables() {
        MYSQL* conn = createConnection();
        const char* query = "SHOW TABLES;";
        if (mysql_query(conn, query)) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getTables: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("getTables: Query failed: " + err);
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getTables: Failed to retrieve result: " + err);
            mysql_close(conn);
            throw std::runtime_error("getTables: Failed to retrieve result: " + err);
        }
        std::vector<std::string> tables;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                tables.push_back(row[0]);
            }
        }
        mysql_free_result(result);
        mysql_close(conn);
        return tables;
    }

    /**
     * @brief Retrieve the class IDs associated with a specific user.
     *
     * Executes a SQL query against the "user_classes" table.
     *
     * @param user_id The ID of the user.
     * @return A vector of integer class IDs.
     */
    std::vector<int> getClassIds(const unsigned int user_id) {
        if (user_id == 0) {
            dalLogger.logErr("getClassIds: Invalid user ID (0) provided.");
            throw std::invalid_argument("getClassIds: user_id must be non-zero.");
        }
        MYSQL* conn = createConnection();
        std::stringstream ss;
        ss << "SELECT class_id FROM user_classes WHERE user_id = " << user_id << ";";
        std::string query = ss.str();
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getClassIds: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("getClassIds: Query failed: " + err);
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getClassIds: Failed to retrieve result: " + err);
            mysql_close(conn);
            throw std::runtime_error("getClassIds: Failed to retrieve result: " + err);
        }
        std::vector<int> classIds;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                int id = std::stoi(row[0]);
                classIds.push_back(id);
            }
        }
        mysql_free_result(result);
        mysql_close(conn);
        dalLogger.logDebug("getClassIds: Retrieved class ids for user " + std::to_string(user_id));
        return classIds;
    }

    /**
     * @brief Retrieve the note IDs associated with a specific user.
     *
     * Executes a join query between "notes" and "user_classes" to obtain note IDs.
     *
     * @param user_id The ID of the user.
     * @return A vector of integer note IDs.
     */
    std::vector<int> getNoteIds(const unsigned int user_id) {
        if (user_id == 0) {
            dalLogger.logErr("getNoteIds: Invalid user ID (0) provided.");
            throw std::invalid_argument("getNoteIds: user_id must be non-zero.");
        }
        MYSQL* conn = createConnection();
        std::stringstream ss;
        ss << "SELECT n.id FROM notes n INNER JOIN user_classes uc ON n.class_id = uc.class_id "
           << "WHERE uc.user_id = " << user_id << ";";
        std::string query = ss.str();
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getNoteIds: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("getNoteIds: Query failed: " + err);
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getNoteIds: Failed to retrieve result: " + err);
            mysql_close(conn);
            throw std::runtime_error("getNoteIds: Failed to retrieve result: " + err);
        }
        std::vector<int> noteIds;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                int id = std::stoi(row[0]);
                noteIds.push_back(id);
            }
        }
        mysql_free_result(result);
        mysql_close(conn);
        dalLogger.logDebug("getNoteIds: Retrieved note ids for user " + std::to_string(user_id));
        return noteIds;
    }

    /**
     * @brief Retrieve the file path for a specific note.
     *
     * Executes a query against the "notes" table to obtain the file path.
     *
     * @param note_id The ID of the note.
     * @return A string with the file path.
     */
    std::string getNoteFilePath(const unsigned int note_id) {
        if (note_id == 0) {
            dalLogger.logErr("getNoteFilePath: Invalid note id (0) provided.");
            throw std::invalid_argument("getNoteFilePath: note_id must be non-zero.");
        }
        MYSQL* conn = createConnection();
        std::stringstream ss;
        ss << "SELECT file_path FROM notes WHERE id = " << note_id << ";";
        std::string query = ss.str();
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getNoteFilePath: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("getNoteFilePath: Query failed: " + err);
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getNoteFilePath: Failed to retrieve result: " + err);
            mysql_close(conn);
            throw std::runtime_error("getNoteFilePath: Failed to retrieve result: " + err);
        }
        std::string filePath;
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            filePath = row[0];
        } else {
            dalLogger.logErr("getNoteFilePath: No file path found for note id " + std::to_string(note_id));
            mysql_free_result(result);
            mysql_close(conn);
            throw std::runtime_error("getNoteFilePath: File path not found for note id " + std::to_string(note_id));
        }
        mysql_free_result(result);
        mysql_close(conn);
        dalLogger.logDebug("getNoteFilePath: Retrieved file path for note " + std::to_string(note_id));
        return filePath;
    }

    //----------------------------------------------------------------------    
    // File and JSON I/O Functions (with per-file mutexes)
    //----------------------------------------------------------------------

    /**
     * @brief Read the contents of a file.
     *
     * Locks the mutex dedicated to the given file path.
     *
     * @param file_path The path of the file.
     * @return A string containing the file's content.
     */
    std::string readFile(const std::string& file_path) {
        auto fileMtx = getFileMutex(file_path);
        std::lock_guard<std::mutex> lock(*fileMtx);
        std::ifstream in(file_path);
        if (!in.is_open()) {
            dalLogger.logErr("readFile: Cannot open file for reading: " + file_path);
            throw std::runtime_error("readFile: Cannot open file: " + file_path);
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        dalLogger.logDebug("readFile: Successfully read file: " + file_path);
        return ss.str();
    }

    /**
     * @brief Write data to a file.
     *
     * Locks the mutex dedicated to the given file path.
     * On failure, an exception is thrown.
     *
     * @param file_path The file path to write.
     * @param data The data to write.
     * @return True if writing succeeded.
     */
    bool writeFile(const std::string& file_path, const std::string& data) {
        auto fileMtx = getFileMutex(file_path);
        std::lock_guard<std::mutex> lock(*fileMtx);
        std::ofstream out(file_path);
        if (!out.is_open()) {
            dalLogger.logErr("writeFile: Cannot open file for writing: " + file_path);
            throw std::runtime_error("writeFile: Cannot open file: " + file_path);
        }
        out << data;
        if (!out.good()) {
            dalLogger.logErr("writeFile: Error occurred while writing to file: " + file_path);
            throw std::runtime_error("writeFile: Failed to write file: " + file_path);
        }
        dalLogger.logDebug("writeFile: Successfully wrote file: " + file_path);
        return true;
    }

    /**
     * @brief Read the contents of a plain text file.
     *
     * Simply calls readFile.
     *
     * @param file_path The text file path.
     * @return The file contents as a string.
     */
    std::string readTxtFile(const std::string& file_path) {
        return readFile(file_path);
    }

    /**
     * @brief Read and parse a JSON file.
     *
     * Locks the per-file mutex and parses the JSON file.
     *
     * @param file_path The JSON file path.
     * @return A nlohmann::json object with the parsed content.
     */
    nlohmann::json readJsonFile(const std::string& file_path) {
        auto fileMtx = getFileMutex(file_path);
        std::lock_guard<std::mutex> lock(*fileMtx);
        std::ifstream in(file_path);
        if (!in.is_open()) {
            dalLogger.logErr("readJsonFile: Cannot open JSON file for reading: " + file_path);
            throw std::runtime_error("readJsonFile: Cannot open file: " + file_path);
        }
        nlohmann::json j;
        try {
            in >> j;
        } catch (const std::exception& e) {
            dalLogger.logErr("readJsonFile: Error parsing JSON from " + file_path + ": " + e.what());
            throw std::runtime_error("readJsonFile: Failed to parse JSON file: " + file_path);
        }
        dalLogger.logDebug("readJsonFile: Successfully read JSON file: " + file_path);
        return j;
    }

    /**
     * @brief Write a nlohmann::json object to a file.
     *
     * The JSON data must contain a "file_path" field. Locks the corresponding mutex.
     * Throws an exception on error.
     *
     * @param data The JSON object to write.
     */
    void writeJsonFile(const nlohmann::json& data) {
        if (!data.contains("file_path")) {
            dalLogger.logErr("writeJsonFile: JSON data does not contain a 'file_path' field.");
            throw std::runtime_error("writeJsonFile: Missing 'file_path' field in JSON data.");
        }
        std::string file_path = data["file_path"].get<std::string>();
        auto fileMtx = getFileMutex(file_path);
        std::lock_guard<std::mutex> lock(*fileMtx);
        std::ofstream out(file_path);
        if (!out.is_open()) {
            dalLogger.logErr("writeJsonFile: Cannot open file for writing JSON: " + file_path);
            throw std::runtime_error("writeJsonFile: Cannot open JSON file: " + file_path);
        }
        try {
            out << data.dump(4);
        } catch (const std::exception& e) {
            dalLogger.logErr("writeJsonFile: Error writing JSON data to " + file_path + ": " + e.what());
            throw std::runtime_error("writeJsonFile: Failed to write JSON file: " + file_path);
        }
        if (!out.good()) {
            dalLogger.logErr("writeJsonFile: I/O error occurred while writing JSON file: " + file_path);
            throw std::runtime_error("writeJsonFile: I/O error writing JSON file: " + file_path);
        }
        dalLogger.logDebug("writeJsonFile: Successfully wrote JSON file: " + file_path);
    }

    //----------------------------------------------------------------------    
    // Authentication Functions
    //----------------------------------------------------------------------

    /**
     * @brief Retrieve a user by username.
     *
     * Executes a query on the "users" table. Returns std::nullopt if not found.
     *
     * @param username The username to search for.
     * @return An optional User object.
     */
    std::optional<User> getUserByUsername(const std::string& username) {
        if (username.empty()) {
            dalLogger.logWarn("getUserByUsername: Empty username provided.");
            return std::nullopt;
        }
        MYSQL* conn = createConnection();
        // Note: For simplicity, building the query directly. Consider prepared statements for production.
        std::string query = "SELECT id, username, password_hash FROM users WHERE username = '" + username + "' LIMIT 1;";
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getUserByUsername: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("getUserByUsername: Query failed: " + err);
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("getUserByUsername: Failed to retrieve result: " + err);
            mysql_close(conn);
            throw std::runtime_error("getUserByUsername: Failed to retrieve result: " + err);
        }
        std::optional<User> user = std::nullopt;
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            User u;
            u.id = std::stoi(row[0]);
            u.username = row[1] ? row[1] : "";
            u.password_hash = row[2] ? row[2] : "";
            user = u;
            dalLogger.logDebug("getUserByUsername: Found user '" + username + "'");
        } else {
            dalLogger.logDebug("getUserByUsername: No user found for username: " + username);
        }
        mysql_free_result(result);
        mysql_close(conn);
        return user;
    }

    /**
     * @brief Create a new user.
     *
     * Inserts a new record into the "users" table. Throws an exception on failure.
     *
     * @param username The user's username.
     * @param hashedPassword The user's hashed password.
     * @return True if insertion succeeded.
     */
    bool createUser(const std::string& username, const std::string& hashedPassword) {
        if (username.empty() || hashedPassword.empty()) {
            dalLogger.logErr("createUser: Username or hashedPassword is empty.");
            throw std::invalid_argument("createUser: Username and hashedPassword cannot be empty.");
        }
        MYSQL* conn = createConnection();
        std::stringstream ss;
        ss << "INSERT INTO users (username, password_hash) VALUES ('" << username << "', '" << hashedPassword << "');";
        std::string query = ss.str();
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("createUser: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("createUser: Query failed: " + err);
        }
        mysql_close(conn);
        dalLogger.logDebug("createUser: User '" + username + "' created successfully.");
        return true;
    }

    /**
     * @brief Update a user's password.
     *
     * Executes an UPDATE on the "users" table. Throws an exception on failure.
     *
     * @param username The username.
     * @param newHashedPassword The new hashed password.
     * @return True if update succeeded.
     */
    bool updateUserPassword(const std::string& username, const std::string& newHashedPassword) {
        if (username.empty() || newHashedPassword.empty()) {
            dalLogger.logErr("updateUserPassword: Username or newHashedPassword is empty.");
            throw std::invalid_argument("updateUserPassword: Username and newHashedPassword cannot be empty.");
        }
        MYSQL* conn = createConnection();
        std::stringstream ss;
        ss << "UPDATE users SET password_hash = '" << newHashedPassword << "' WHERE username = '" << username << "';";
        std::string query = ss.str();
        if (mysql_query(conn, query.c_str())) {
            std::string err = mysql_error(conn);
            dalLogger.logErr("updateUserPassword: Query failed: " + err);
            mysql_close(conn);
            throw std::runtime_error("updateUserPassword: Query failed: " + err);
        }
        mysql_close(conn);
        dalLogger.logDebug("updateUserPassword: Password updated successfully for user: " + username);
        return true;
    }

/**
 * @brief Execute an SQL query on the database
 * @param query The SQL query to execute
 * @return True if the query was executed successfully, false otherwise
 */
bool execute_query(const std::string& query) {
    MYSQL* conn = createConnection();
    if (!conn) return false;

    bool success = (mysql_query(conn, query.c_str()) == 0);
    if (!success) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
    }
    
    mysql_close(conn);
    return success;
}
/**
 * @brief Escape a string to make it safe for use in SQL queries
 * @param input The string to escape
 * @return The escaped string
 */
std::string escape_string(const std::string& input) {
    MYSQL* conn = createConnection();
    if (!conn) {
        std::cerr << "[ERROR] Failed to connect to database for string escaping." << std::endl;
        // Basic fallback escaping for single quotes
        std::string result = input;
        size_t pos = 0;
        while ((pos = result.find("'", pos)) != std::string::npos) {
            result.replace(pos, 1, "''");
            pos += 2;
        }
        return result;
    }

    char* escaped = new char[input.length() * 2 + 1];
    mysql_real_escape_string(conn, escaped, input.c_str(), input.length());
    std::string result(escaped);
    delete[] escaped;
    mysql_close(conn);
    return result;
}

/**
 * @brief Execute a query and return the first column of the first row
 * @param query The SQL query to execute
 * @return The result string, or empty if no results
 */
std::string get_single_result(const std::string& query) {
    MYSQL* conn = createConnection();
    if (!conn) {
        std::cerr << "[ERROR] Failed to connect to database for query." << std::endl;
        return "";
    }

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return "";
    }

    MYSQL_RES* result = mysql_store_result(conn);
    std::string value;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            value = row[0];
        }
        mysql_free_result(result);
    }
    mysql_close(conn);
    return value;
}

/**
 * @brief Check if a query returns any results
 * @param query The SQL query to execute (should be a COUNT or similar query)
 * @return True if the query returns a non-zero result, false otherwise
 */
bool query_returns_results(const std::string& query) {
    MYSQL* conn = createConnection();
    if (!conn) return false;

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    bool has_results = false;
    
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            // Convert the first column to an integer
            has_results = (std::stoi(row[0]) > 0);
        }
        mysql_free_result(result);
    }
    
    mysql_close(conn);
    return has_results;
}

} // end namespace DAL

/* ---------------------------------------------------------------------------
   Example Usage (for integration reference)

   // The following commented code demonstrates how to use the Data Access Layer.
   // Place it in a separate source file (e.g., example.cc) for testing or integration.

   #include "data_access_layer.h"
   #include "logger.h"
   #include <iostream>
   #include <optional>

   int main() {
       try {
           // Retrieve and log all table names.
           auto tables = DAL::getTables();
           dalLogger.log("Tables in database:");
           for (const auto& table : tables) {
               dalLogger.log(" - " + table);
           }
           
           // Get class IDs for a user (e.g., user with ID 1).
           unsigned int userId = 1;
           auto classIds = DAL::getClassIds(userId);
           dalLogger.log("Class IDs for user " + std::to_string(userId) + ":");
           for (const auto& id : classIds) {
               dalLogger.log(" " + std::to_string(id));
           }
           
           // Get note IDs for the same user.
           auto noteIds = DAL::getNoteIds(userId);
           dalLogger.log("Note IDs for user " + std::to_string(userId) + ":");
           for (const auto& id : noteIds) {
               dalLogger.log(" " + std::to_string(id));
           }
           
           // Retrieve the file path for a specific note and perform file operations.
           if (!noteIds.empty()) {
               std::string notePath = DAL::getNoteFilePath(noteIds[0]);
               dalLogger.log("File path for note " + std::to_string(noteIds[0]) + ": " + notePath);
               DAL::writeFile(notePath, "This is sample note content stored in the file system.");
               std::string content = DAL::readTxtFile(notePath);
               dalLogger.log("Content of " + notePath + ": " + content);
           }
           
           // Write and read JSON data.
           nlohmann::json jsonData = { {"file_path", "data.json"}, {"name", "Folium"}, {"version", 1.0} };
           DAL::writeJsonFile(jsonData);
           dalLogger.log("JSON data written successfully.");
           nlohmann::json readData = DAL::readJsonFile("data.json");
           dalLogger.log("Read JSON data: " + readData.dump());
           
           // Demonstrate authentication functions.
           if (DAL::createUser("new_user", "new_hashed_password")) {
               dalLogger.log("User created successfully.");
           }
           std::optional<User> user = DAL::getUserByUsername("new_user");
           if (user.has_value()) {
               dalLogger.log("Found user: " + user->username);
           }
           if (DAL::updateUserPassword("new_user", "updated_hashed_password")) {
               dalLogger.log("User password updated successfully.");
           }
       } catch (const std::exception& e) {
           dalLogger.logErr(std::string("Exception caught: ") + e.what());
           return 1;
       }
       return 0;
   }
   --------------------------------------------------------------------------- */
