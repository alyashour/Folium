#include "core.h"
#include "data_access_layer.h" // Include the DAL header
#include <stdexcept>
#include <sstream>
#include <fstream> 
#include <filesystem>  // For directory creation (C++17)

namespace Core {

// Retrieve the big note for a specific class
std::string getBigNote(int classId, int userId) {
    try {
        // Verify user access
        std::string accessQuery = "SELECT 1 FROM user_classes WHERE class_id = " + std::to_string(classId) +
                                  " AND user_id = " + std::to_string(userId) + ";";
        if (!DAL::query_returns_results(accessQuery)) {
            throw std::runtime_error("User does not have access to this class.");
        }

        // Retrieve the file path for the note
        std::string filePathQuery = "SELECT file_path FROM notes WHERE class_id = " + std::to_string(classId) + ";";
        std::string filePath = DAL::get_single_result(filePathQuery);
        if (filePath.empty()) {
            throw std::runtime_error("No big note exists for this class.");
        }

        // Check if the file exists
        if (!std::filesystem::exists(filePath)) {
            throw std::runtime_error("Note file does not exist at path: " + filePath);
        }

        // Read the file content
        std::string noteContent = DAL::readTxtFile(filePath); // Use readTxtFile
        if (noteContent.empty()) {
            throw std::runtime_error("Note content is empty or file could not be read at path: " + filePath);
        }

        return noteContent;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to retrieve big note: " + std::string(e.what()));
    }
}

// Create a new big note for a class
bool createBigNote(int classId, int userId, const std::string& content, const std::string& title) {
    try {
        // Verify user enrollment
        if (!DAL::query_returns_results("SELECT 1 FROM user_classes WHERE user_id = " + std::to_string(userId) + 
                                        " AND class_id = " + std::to_string(classId) + ";")) {
            throw std::runtime_error("User is not enrolled in this class.");
        }

        // Create the note file
        std::string notePath = "notes/class_" + std::to_string(classId) + "_note.txt";
        std::ofstream noteFile(notePath);
        if (!noteFile) {
            throw std::runtime_error("Failed to create note file: " + notePath);
        }
        noteFile << content;
        noteFile.close();

        // Insert the note record into the database
        std::string query = "INSERT INTO notes (class_id, file_path, title, created_at, updated_at) VALUES (" +
                            std::to_string(classId) + ", '" + DAL::escape_string(notePath) + "', '" +
                            DAL::escape_string(title) + "', NOW(), NOW());";
        if (!DAL::execute_query(query)) {
            throw std::runtime_error("Failed to insert note record into database.");
        }

        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create big note: " + std::string(e.what()));
    }
}

// Upload and integrate a new note
bool uploadNote(int classId, int userId, const std::string& filePath, const std::string& title) {
    try {
        // Verify user access
        if (!DAL::query_returns_results("SELECT 1 FROM user_classes WHERE class_id = " + std::to_string(classId) + 
                                        " AND user_id = " + std::to_string(userId) + ";")) {
            throw std::runtime_error("User is not enrolled in this class.");
        }

        // Read the uploaded file content
        std::string uploadedContent = DAL::readTxtFile(filePath); // Use readTxtFile
        if (uploadedContent.empty()) {
            throw std::runtime_error("Uploaded file is empty or could not be read.");
        }

        // Check if a note already exists
        std::string existingFilePath = DAL::get_single_result("SELECT file_path FROM notes WHERE class_id = " + std::to_string(classId) + ";");
        if (existingFilePath.empty()) {
            // Create a new note if none exists
            return createBigNote(classId, userId, uploadedContent, title.empty() ? "Uploaded Note" : title);
        }

        // Append the uploaded content to the existing note
        std::string existingContent = DAL::readTxtFile(existingFilePath); // Use readTxtFile
        std::string integratedContent = existingContent + "\n\n--- New Upload ---\n\n" + uploadedContent;

        if (!DAL::writeFile(existingFilePath, integratedContent)) { // Use writeFile
            throw std::runtime_error("Failed to write integrated content to file.");
        }

        // Update the database timestamp
        if (!DAL::execute_query("UPDATE notes SET updated_at = NOW() WHERE class_id = " + std::to_string(classId) + ";")) {
            throw std::runtime_error("Failed to update note timestamp in database.");
        }

        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to upload note: " + std::string(e.what()));
    }
}

// Edit the big note for a class
bool editBigNote(int classId, int userId, const std::string& content, const std::string& title) {
    try {
        // Verify user access
        if (!DAL::query_returns_results("SELECT 1 FROM user_classes WHERE class_id = " + std::to_string(classId) + 
                                        " AND user_id = " + std::to_string(userId) + ";")) {
            throw std::runtime_error("User is not enrolled in this class.");
        }

        // Retrieve the file path for the note
        std::string filePath = DAL::get_single_result("SELECT file_path FROM notes WHERE class_id = " + std::to_string(classId) + ";");
        if (filePath.empty()) {
            throw std::runtime_error("No big note exists for this class. Use createBigNote first.");
        }

        // Write the updated content to the file
        if (!DAL::writeFile(filePath, content)) { // Use writeFile
            throw std::runtime_error("Failed to write updated note content to file at path: " + filePath);
        }

        // Update the title and timestamp in the database
        std::string query = title.empty() 
                            ? "UPDATE notes SET updated_at = NOW() WHERE class_id = " + std::to_string(classId) + ";"
                            : "UPDATE notes SET title = '" + DAL::escape_string(title) + "', updated_at = NOW() WHERE class_id = " + std::to_string(classId) + ";";
        if (!DAL::execute_query(query)) {
            throw std::runtime_error("Failed to update note in database.");
        }

        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to edit big note: " + std::string(e.what()));
    }
}

} // namespace Core