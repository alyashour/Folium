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
    std::vector<std::string> get_tables();

    /**
     * @brief Retrieve the class IDs associated with a specific user.
     * @param user_id The ID of the user whose class IDs are to be retrieved.
     */
    void get_class_ids(const unsigned int user_id);

    /**
     * @brief Retrieve the note IDs associated with a specific user.
     * @param user_id The ID of the user whose note IDs are to be retrieved.
     */
    void get_note_ids(const unsigned int user_id);

    /**
     * @brief Retrieve the file path for a specific note.
     * @param note_id The ID of the note whose file path is to be retrieved.
     * @return A string containing the file path of the note.
     */
    std::string get_note_file_path(const unsigned int note_id);

    ///////////
    /* FILES */
    ///////////

    /**
     * @brief Read the contents of a file.
     * @param file_path The path to the file to be read.
     * @return A string containing the contents of the file.
     */
    std::string read_file(const std::string& file_path);

    /**
     * @brief Write data to a file.
     * @param file_path The path to the file to be written.
     * @param data The data to write to the file.
     * @return True if the operation was successful, false otherwise.
     */
    bool write_file(const std::string& file_path, const std::string& data);

    //////////
    /* TEXT */
    //////////

    /**
     * @brief Read the contents of a plain text file.
     * @param file_path The path to the text file to be read.
     * @return A string containing the contents of the text file.
     */
    std::string read_txt_file(const std::string& file_path);

    //////////
    /* JSON */
    //////////

    /**
     * @brief Read the contents of a JSON file.
     * @param file_path The path to the JSON file to be read.
     * @return A JSON object containing the parsed data from the file.
     */
    nlohmann::json read_json_file(const std::string& file_path);

    /**
     * @brief Write data to a JSON file.
     * @param data The JSON object to write to the file.
     */
    void write_json_file(const nlohmann::json& data);

    // ===== AUTH-RELATED FUNCTIONS ===== //

    /**
     * @brief Retrieves a user by username.
     * @param username The username to search for.
     * @return An optional User object; if not found, std::nullopt is returned.
     */
    std::optional<User> getUserByUsername(const std::string& username);

    /**
     * @brief Creates a new user.
     * @param username The user's username.
     * @param hashedPassword The user's hashed password.
     * @return true if the user was created successfully.
     */
    bool createUser(const std::string& username, const std::string& hashedPassword);

    /**
     * @brief Updates a user's password.
     * @param username The user's username.
     * @param newHashedPassword The new hashed password.
     * @return true if the password was successfully updated.
     */
    bool updateUserPassword(const std::string& username, const std::string& newHashedPassword);
}

#endif // FOLSERV_DATA_ACCESS_LAYER_H_
