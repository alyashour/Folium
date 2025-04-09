#include "core.h"
#include "data_access_layer.h" // Include the DAL header
#include <stdexcept>
#include <sstream>
#include <filesystem>  // For directory creation (C++17)

namespace Core {

// Retrieve a note by its ID and user ID
std::string getNote(int noteID, int userID) {
    try {
        // Verify user has access to this note
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM notes n "
                         << "INNER JOIN user_classes uc ON n.class_id = uc.class_id "
                         << "WHERE n.id = " << noteID << " AND uc.user_id = " << userID << ";";
        
        // This is a hypothetical function that would need to be implemented in your DAL
        // to check if a query returns a non-zero count
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User does not have access to this note or note does not exist.");
        }
        
        // Use DAL to retrieve the file path for the note
        std::string filePath = DAL::get_note_file_path(noteID);
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

// Create a new note
void createNote(int noteID, int userID, int classID, const std::string& noteDATA) {
    try {
        // Ensure the notes directory exists
        std::filesystem::create_directories("notes");
        
        // Define a file path for the new note (e.g., based on noteID)
        std::ostringstream filePathStream;
        filePathStream << "notes/note_" << noteID << ".txt";
        std::string filePath = filePathStream.str();

        // Use DAL to write the note content to the file
        if (!DAL::write_file(filePath, noteDATA)) {
            throw std::runtime_error("Failed to write note content to file at path: " + filePath);
        }

        // Insert the note into the database
        std::ostringstream queryStream;
        queryStream << "INSERT INTO notes (id, class_id, file_path, created_at, last_modified) VALUES ("
                   << noteID << ", " << classID << ", '" << filePath << "', NOW(), NOW());";
        std::string query = queryStream.str();

        if (!DAL::execute_query(query)) {
            // If the insertion failed, attempt to remove the created file to maintain consistency
            std::remove(filePath.c_str());
            throw std::runtime_error("Failed to insert note into database.");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create note: " + std::string(e.what()));
    }
}

// Edit an existing note
void editNote(int noteID, int userID, const std::string& noteDATA) {
    try {
        // Verify user has access to this note
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM notes n "
                         << "INNER JOIN user_classes uc ON n.class_id = uc.class_id "
                         << "WHERE n.id = " << noteID << " AND uc.user_id = " << userID << ";";
        
        // Again, this is a hypothetical function
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User does not have access to this note or note does not exist.");
        }
        
        // Use DAL to retrieve the file path for the note
        std::string filePath = DAL::get_note_file_path(noteID);
        if (filePath.empty()) {
            throw std::runtime_error("Note file path could not be retrieved from database.");
        }

        // Use DAL to write the updated content to the file
        if (!DAL::write_file(filePath, noteDATA)) {
            throw std::runtime_error("Failed to write updated note content to file at path: " + filePath);
        }

        // Update the last_modified timestamp in the database
        std::ostringstream queryStream;
        queryStream << "UPDATE notes SET last_modified = NOW() WHERE id = " << noteID << ";";
        std::string query = queryStream.str();

        if (!DAL::execute_query(query)) {
            throw std::runtime_error("Failed to update note timestamp in database.");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to edit note: " + std::string(e.what()));
    }
}

} // namespace Core