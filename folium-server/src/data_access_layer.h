/**
 * @file data_access_layer.h
 * @brief Data Access Layer (DAL) for the Folium server.
 *
 * This file defines the interface for interacting with the underlying data storage
 * systems, including database operations and file handling. It abstracts the details
 * of data retrieval, storage, and manipulation, providing a clean interface for
 * higher-level components of the server.
 *
 * @section Responsibilities
 * - Database operations: Retrieve tables, class IDs, note IDs, and file paths.
 * - File operations: Read and write plain text and JSON files.
 * - JSON handling: Read and write structured data in JSON format.
 */

#ifndef FOLSERV_DATA_ACCESS_LAYER_H_
#define FOLSERV_DATA_ACCESS_LAYER_H_

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <optional>

namespace DAL {

    // A simple User struct for demonstration.
    struct User {
        int id;
        std::string username;
        std::string password_hash;
    };

    //////////////
    /* DATABASE */
    //////////////

    /**
     * @brief Retrieve the list of database tables.
     * @return A vector of strings containing the names of all tables in the database.
     */
    std::vector<std::string> getTables();

    /**
     * @brief Retrieve the class IDs associated with a specific user.
     * @param userId The ID of the user whose class IDs are to be retrieved.
     * @return A vector of integer class IDs.
     */
    std::vector<int> getClassIds(const unsigned int userId);

    /**
     * @brief Retrieve the note IDs associated with a specific user.
     * @param userId The ID of the user whose note IDs are to be retrieved.
     * @return A vector of integer note IDs.
     */
    std::vector<int> getNoteIds(const unsigned int userId);

    /**
     * @brief Retrieve the file path for a specific note.
     * @param noteId The ID of the note whose file path is to be retrieved.
     * @return A string containing the file path of the note.
     */
    std::string getNoteFilePath(const unsigned int noteId);

    ///////////
    /* FILES */
    ///////////

    /**
     * @brief Read the contents of a file.
     * @param filePath The path to the file to be read.
     * @return A string containing the contents of the file.
     */
    std::string readFile(const std::string& filePath);

    /**
     * @brief Write data to a file.
     * @param filePath The path to the file to be written.
     * @param data The data to write to the file.
     * @return True if the operation was successful, false otherwise.
     */
    bool writeFile(const std::string& filePath, const std::string& data);

    //////////
    /* TEXT */
    //////////

    /**
     * @brief Read the contents of a plain text file.
     * @param filePath The path to the text file to be read.
     * @return A string containing the contents of the text file.
     */
    std::string readTxtFile(const std::string& filePath);

    //////////
    /* JSON */
    //////////

    /**
     * @brief Read the contents of a JSON file.
     * @param filePath The path to the JSON file to be read.
     * @return A JSON object containing the parsed data from the file.
     */
    nlohmann::json readJsonFile(const std::string& filePath);

    /**
     * @brief Write data to a JSON file.
     * @param data The JSON object to write to the file.
     */
    void writeJsonFile(const nlohmann::json& data);

    /**
     * @brief Execute an SQL query on the database
     * @param query The SQL query to execute
     * @return True if the query was executed successfully, false otherwise
     * @throws std::runtime_error if the database connection fails
     */
    bool execute_query(const std::string& query);

    /**
     * @brief Check if a query returns any results
     * @param query The SQL query to execute (should be a COUNT or similar query)
     * @return True if the query returns a non-zero result, false otherwise
     */
    bool query_returns_results(const std::string& query);

     /**
     * @brief Escape a string to make it safe for use in SQL queries
     * @param input The string to escape
     * @return The escaped string
     */
    std::string escape_string(const std::string& input);
    
    /**
     * @brief Execute a query and return the first column of the first row
     * @param query The SQL query to execute
     * @return The result string, or empty if no results
     */
    std::string get_single_result(const std::string& query);
    
    // ===== AUTH-RELATED FUNCTIONS ===== //

    /**
     * @brief Retrieve a user by username.
     * @param username The username to search for.
     * @return An optional User object; if not found, std::nullopt is returned.
     */
    std::optional<User> getUserByUsername(const std::string& username);

    /**
     * @brief Create a new user.
     * @param username The user's username.
     * @param hashedPassword The user's hashed password.
     * @return True if the user was created successfully.
     */
    bool createUser(const std::string& username, const std::string& hashedPassword);

    /**
     * @brief Update a user's password.
     * @param username The user's username.
     * @param newHashedPassword The new hashed password.
     * @return True if the password was successfully updated.
     */
    bool updateUserPassword(const std::string& username, const std::string& newHashedPassword);
}

#endif // FOLSERV_DATA_ACCESS_LAYER_H_
