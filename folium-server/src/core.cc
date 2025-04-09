#include "core.h"
#include "data_access_layer.h" // Include the DAL header
#include <stdexcept>
#include <sstream>
#include <filesystem>  // For directory creation (C++17)

namespace Core {

// Retrieve a note by its class ID and user ID
std::string getNote(int classID, int userID) {
    try {
        // Verify user has access to this class note
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classID 
                         << " AND user_id = " << userID << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User is not enrolled in this class.");
        }
        
        // Use DAL to retrieve the file path for the class note
        std::ostringstream filePathQueryStream;
        filePathQueryStream << "SELECT file_path FROM notes WHERE class_id = " << classID << ";";
        
        std::string filePath;
        MYSQL* conn = DAL::create_connection();
        if (!conn) {
            throw std::runtime_error("Database connection failed.");
        }

        if (mysql_query(conn, filePathQueryStream.str().c_str())) {
            std::string error = mysql_error(conn);
            mysql_close(conn);
            throw std::runtime_error("Query failed: " + error);
        }

        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                filePath = row[0];
            }
            mysql_free_result(result);
        }
        mysql_close(conn);

        if (filePath.empty()) {
            throw std::runtime_error("Note file path could not be retrieved from database.");
        }

        // Use DAL to read the file content
        std::string noteContent = DAL::read_file(filePath);
        if (noteContent.empty()) {
            throw std::runtime_error("Note content is empty or file could not be read at path: " + filePath);
        }

        return noteContent;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to retrieve note: " + std::string(e.what()));
    }
}

// Create a new class note
void createNote(int classID, int userID, const std::string& title, const std::string& noteData) {
    try {
        // Verify user has access to create notes for this class (usually should be instructor/owner)
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classID 
                         << " AND user_id = " << userID << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User is not enrolled in this class.");
        }
        
        // Check if a note already exists for this class
        std::ostringstream noteExistsQuery;
        noteExistsQuery << "SELECT COUNT(*) FROM notes WHERE class_id = " << classID << ";";
        
        if (DAL::query_returns_results(noteExistsQuery.str())) {
            throw std::runtime_error("A note already exists for this class. Use editNote instead.");
        }
        
        // Ensure the notes directory exists
        std::filesystem::create_directories("notes");
        
        // Define a file path for the new note
        std::ostringstream filePathStream;
        filePathStream << "notes/class_" << classID << "_note.txt";
        std::string filePath = filePathStream.str();

        // Use DAL to write the note content to the file
        if (!DAL::write_file(filePath, noteData)) {
            throw std::runtime_error("Failed to write note content to file at path: " + filePath);
        }

        // Insert the note into the database
        std::ostringstream queryStream;
        queryStream << "INSERT INTO notes (class_id, title, file_path) VALUES ("
                   << classID << ", '" << title << "', '" << filePath << "');";
        std::string query = queryStream.str();

        if (!DAL::execute_query(query)) {
            // If the insertion failed, attempt to remove the created file
            std::remove(filePath.c_str());
            throw std::runtime_error("Failed to insert note into database.");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create note: " + std::string(e.what()));
    }
}

// Edit an existing class note
void editNote(int classID, int userID, const std::string& noteData) {
    try {
        // Verify user has access to this class
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classID 
                         << " AND user_id = " << userID << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User is not enrolled in this class.");
        }
        
        // Use DAL to retrieve the file path for the note
        std::ostringstream filePathQueryStream;
        filePathQueryStream << "SELECT file_path FROM notes WHERE class_id = " << classID << ";";
        
        std::string filePath;
        MYSQL* conn = DAL::create_connection();
        if (!conn) {
            throw std::runtime_error("Database connection failed.");
        }

        if (mysql_query(conn, filePathQueryStream.str().c_str())) {
            std::string error = mysql_error(conn);
            mysql_close(conn);
            throw std::runtime_error("Query failed: " + error);
        }

        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                filePath = row[0];
            }
            mysql_free_result(result);
        }
        mysql_close(conn);

        if (filePath.empty()) {
            throw std::runtime_error("Note file path could not be retrieved from database.");
        }

        // Use DAL to write the updated content to the file
        if (!DAL::write_file(filePath, noteData)) {
            throw std::runtime_error("Failed to write updated note content to file at path: " + filePath);
        }

        // Note: The updated_at timestamp will be updated automatically in the database
        // due to the ON UPDATE CURRENT_TIMESTAMP in the schema
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to edit note: " + std::string(e.what()));
    }
}

} // namespace Core