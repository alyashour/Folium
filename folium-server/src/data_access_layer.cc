/**
 * @file data_access_layer.cc
 * @brief Implementation of the Data Access Layer (DAL) for the Folium server.
 *
 * This file defines functions to interact with the MySQL database and perform 
 * file I/O for plain text and JSON files. The database connection parameters 
 * are read from a local "dbConfig.json" file.
 *
 * The expected structure for dbConfig.json is:
 *
 * {
 *   "mysql_host": "127.0.0.1",
 *   "mysql_port": 3306,
 *   "mysql_user": "root",
 *   "mysql_password": "",
 *   "mysql_database": "folium"
 * }
 *
 * Note: Since this code uses MySQL's C API (or MySQL Connector/C++), you must
 * link against the MySQL library. If you wish to avoid manual linking, CMake
 * (as in your CMakeLists.txt) will automatically find and link the required
 * library using find_library()/target_link_libraries(), but an external library
 * is still required at build time.
 */

#include "data_access_layer.h"
#include <mysql/mysql.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace DAL {

/**
 * @brief A simple struct to hold our DB connection parameters.
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
 * This function loads the connection parameters from the configuration file.
 * It uses static variables to ensure the configuration is loaded only once.
 *
 * @return A DBConfig struct containing the loaded configuration.
 */
static DBConfig getDbConfig() {
    static bool loaded = false;
    static DBConfig config;

    // Load configuration only once, then reuse it.
    if (!loaded) {
        try {
            std::ifstream configFile("dbConfig.json");
            if (!configFile.is_open()) {
                throw std::runtime_error("Unable to open dbConfig.json. Please check the file path.");
            }
            nlohmann::json j;
            configFile >> j;

            config.host     = j.value("mysql_host", "127.0.0.1");
            config.port     = j.value("mysql_port", 3306);
            config.user     = j.value("mysql_user", "root");
            config.password = j.value("mysql_password", "");
            config.database = j.value("mysql_database", "folium");

            loaded = true;
        } 
        catch (std::exception &e) {
            std::cerr << "[ERROR] Failed to load dbConfig.json: " << e.what() << std::endl;
            // Optionally, exit or set default values.
            throw;
        }
    }
    return config;
}

/**
 * @brief Create and return a MySQL connection using the parameters from dbConfig.json.
 *
 * @return A MYSQL* connection object or nullptr if the connection fails.
 */
static MYSQL* create_connection() {
    // Load the DB config (host, port, user, password, database)
    DBConfig cfg;
    try {
        cfg = getDbConfig();
    } catch (...) {
        // If there's an error reading the config, we can't continue
        return nullptr;
    }

    MYSQL *conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "[ERROR] mysql_init() failed.\n";
        return nullptr;
    }

    // Connect using the loaded config
    if (!mysql_real_connect(
            conn,
            cfg.host.c_str(),
            cfg.user.c_str(),
            cfg.password.c_str(),
            cfg.database.c_str(),
            cfg.port,
            nullptr,
            0
        )) 
    {
        std::cerr << "[ERROR] mysql_real_connect() failed: " 
                  << mysql_error(conn) << "\n";
        mysql_close(conn);
        return nullptr;
    }
    return conn;
}

/**
 * @brief Retrieve the list of database tables.
 * @return A vector of strings containing the names of all tables in the database.
 */
std::vector<std::string> get_tables() {
    std::vector<std::string> tables;
    MYSQL* conn = create_connection();
    if (!conn) return tables;

    const char* query = "SHOW TABLES;";
    if (mysql_query(conn, query)) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return tables;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                tables.push_back(row[0]);
            }
        }
        mysql_free_result(result);
    }
    mysql_close(conn);
    return tables;
}

/**
 * @brief Retrieve the class IDs associated with a specific user.
 *
 * This function queries the user_classes table and prints the class IDs.
 *
 * @param user_id The ID of the user whose class IDs are to be retrieved.
 *
 * For demonstration, this function prints the class IDs to standard output.
 */
void get_class_ids(const unsigned int user_id) {
    MYSQL* conn = create_connection();
    if (!conn) return;

    std::stringstream ss;
    ss << "SELECT class_id FROM user_classes WHERE user_id = " << user_id << ";";
    std::string query = ss.str();

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result) {
        MYSQL_ROW row;
        std::cout << "Class IDs for user " << user_id << ": ";
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                std::cout << row[0] << " ";
            }
        }
        std::cout << std::endl;
        mysql_free_result(result);
    }
    mysql_close(conn);
}

/**
 * @brief Retrieve the note IDs associated with a specific user.
 *
 * This function joins the notes and user_classes tables to display the note IDs.
 *
 * @param user_id The ID of the user whose note IDs are to be retrieved.
 *
 * This function performs a join between the notes and user_classes tables
 * because the note is keyed to the class, and user_classes links the user to the class.
 */
void get_note_ids(const unsigned int user_id) {
    MYSQL* conn = create_connection();
    if (!conn) return;

    std::stringstream ss;
    ss << "SELECT n.id "
       << "FROM notes n "
       << "INNER JOIN user_classes uc ON n.class_id = uc.class_id "
       << "WHERE uc.user_id = " << user_id << ";";
    std::string query = ss.str();

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result) {
        MYSQL_ROW row;
        std::cout << "Note IDs for user " << user_id << ": ";
        while ((row = mysql_fetch_row(result))) {
            if (row[0] != nullptr) {
                std::cout << row[0] << " ";
            }
        }
        std::cout << std::endl;
        mysql_free_result(result);
    }
    mysql_close(conn);
}

/**
 * @brief Retrieve the file path for a specific note.
 * @param note_id The ID of the note whose file path is to be retrieved.
 * @return A string containing the file path of the note.
 */
std::string get_note_file_path(const unsigned int note_id) {
    std::string file_path;
    MYSQL* conn = create_connection();
    if (!conn) return file_path;

    std::stringstream ss;
    ss << "SELECT file_path FROM notes WHERE id = " << note_id << ";";
    std::string query = ss.str();

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[ERROR] Query failed: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return file_path;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            file_path = row[0];
        }
        mysql_free_result(result);
    }
    mysql_close(conn);
    return file_path;
}

/**
 * @brief Read the contents of a file.
 * @param file_path The path to the file to be read.
 * @return A string containing the contents of the file.
 */
std::string read_file(const std::string& file_path) {
    std::ifstream in(file_path);
    if (!in) {
        std::cerr << "[ERROR] Cannot open file for reading: " << file_path << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

/**
 * @brief Write data to a file.
 * @param file_path The path to the file to be written.
 * @param data The data to write to the file.
 * @return True if the operation was successful, false otherwise.
 */
bool write_file(const std::string& file_path, const std::string& data) {
    std::ofstream out(file_path);
    if (!out) {
        std::cerr << "[ERROR] Cannot open file for writing: " << file_path << std::endl;
        return false;
    }
    out << data;
    return true;
}

/**
 * @brief Read the contents of a plain text file.
 * @param file_path The path to the text file to be read.
 * @return A string containing the contents of the text file.
 */
std::string read_txt_file(const std::string& file_path) {
    // This function can simply call read_file (since it's plain text).
    return read_file(file_path);
}

/**
 * @brief Read the contents of a JSON file.
 * @param file_path The path to the JSON file to be read.
 * @return A JSON object containing the parsed data from the file.
 */
nlohmann::json read_json_file(const std::string& file_path) {
    nlohmann::json j;
    std::ifstream in(file_path);
    if (!in) {
        std::cerr << "[ERROR] Cannot open JSON file for reading: " << file_path << std::endl;
        return j;
    }
    try {
        in >> j;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Error parsing JSON: " << e.what() << std::endl;
    }
    return j;
}

/**
 * @brief Write data to a JSON file.
 * @param data The JSON object to write to the file. We look for "file_path" 
 *             within the JSON itself to decide where to save it.
 */
void write_json_file(const nlohmann::json& data) {
    if (!data.contains("file_path")) {
        std::cerr << "[ERROR] JSON data does not contain a 'file_path' field." << std::endl;
        return;
    }
    std::string file_path = data["file_path"].get<std::string>();
    std::ofstream out(file_path);
    if (!out) {
        std::cerr << "[ERROR] Cannot open file for writing JSON: " << file_path << std::endl;
        return;
    }
    // Pretty-print the JSON with an indent of 4 spaces.
    out << data.dump(4);
}

/**
 * @brief Execute an SQL query on the database
 * @param query The SQL query to execute
 * @return True if the query was executed successfully, false otherwise
 */
bool execute_query(const std::string& query) {
    MYSQL* conn = create_connection();
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
    MYSQL* conn = create_connection();
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
    MYSQL* conn = create_connection();
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
    MYSQL* conn = create_connection();
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

/***********************************************************
 * EXAMPLE USAGE:
 * 
 * In another source file (e.g., example.cc), you can include the 
 * data_access_layer.h header and use the functions as follows:
 * 
 *   #include "data_access_layer.h"
 *   #include <iostream>
 * 
 *   int main() {
 *       // Retrieve and print all table names:
 *       auto tables = DAL::get_tables();
 *       for (const auto& table : tables) {
 *           std::cout << "Table: " << table << std::endl;
 *       }
 * 
 *       // Get the file path for a specific note (e.g., note ID 1)
 *       std::string notePath = DAL::get_note_file_path(1);
 *       std::cout << "File path for note 1: " << notePath << std::endl;
 * 
 *       return 0;
 *   }
 * 
 * Note: Since the code uses MySQL's functions, you must ensure that 
 * the MySQL library is linked. As shown in your CMakeLists.txt, CMake 
 * automatically locates and links the MySQL Connector/C++ (or MySQL client)
 * library. There's no fully header-only replacement for MySQL's API.
 ***********************************************************/
