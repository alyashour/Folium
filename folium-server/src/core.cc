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
        // Verify user has access to this class
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classId 
                         << " AND user_id = " << userId << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User does not have access to this class.");
        }
        
        // Check if the class has a note
        std::ostringstream noteExistsQuery;
        noteExistsQuery << "SELECT COUNT(*) FROM notes WHERE class_id = " << classId << ";";
        std::cout << "Executing query: " << noteExistsQuery.str() << std::endl;

        if (!DAL::query_returns_results(noteExistsQuery.str())) {
            throw std::runtime_error("No big note exists for this class.");
        }
        
        // Use DAL to retrieve the file path for the note
        std::ostringstream filePathQuery;
        filePathQuery << "SELECT file_path FROM notes WHERE class_id = " << classId << ";";
        std::cout << "Executing query: " << filePathQuery.str() << std::endl;

        std::string filePath = DAL::get_single_result(filePathQuery.str());
        std::cout << "Retrieved file path: " << filePath << std::endl;

        if (filePath.empty()) {
            throw std::runtime_error("Note file path could not be retrieved from database.");
        }

        // Check if the file exists
        if (!std::filesystem::exists(filePath)) {
            throw std::runtime_error("Note file does not exist at path: " + filePath);
        }

        // Use DAL to read the file content
        std::string noteContent = DAL::read_file(filePath);
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
        // Check if the user is enrolled in the class
        std::ostringstream query;
        query << "SELECT * FROM user_classes WHERE user_id = " << userId << " AND class_id = " << classId;
        std::cout << "Executing query: " << query.str() << std::endl;

        if (!DAL::query_returns_results(query.str())) {
            throw std::runtime_error("Failed to create big note: User is not enrolled in this class.");
        }

        // Create the note file
        std::string notePath = "notes/class_" + std::to_string(classId) + "_note.txt";
        std::ofstream noteFile(notePath);
        if (!noteFile) {
            throw std::runtime_error("Failed to create note file: " + notePath);
        }
        noteFile << content;
        noteFile.close();

        // Insert a record into the notes table
        std::ostringstream insertQuery;
        insertQuery << "INSERT INTO notes (class_id, file_path, title, created_at, updated_at) VALUES ("
                    << classId << ", '"
                    << DAL::escape_string(notePath) << "', '"
                    << DAL::escape_string(title) << "', NOW(), NOW());";

        if (!DAL::execute_query(insertQuery.str())) {
            throw std::runtime_error("Failed to insert note record into database.");
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in createBigNote: " << e.what() << std::endl;
        throw;
    }
}

// Upload and integrate a new note
bool uploadNote(int classId, int userId, const std::string& filePath, const std::string& title) {
    try {
        // Verify user has access to this class
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classId 
                         << " AND user_id = " << userId << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User is not enrolled in this class.");
        }
        
        // Read the uploaded file content
        std::string uploadedContent = DAL::read_file(filePath);
        if (uploadedContent.empty()) {
            throw std::runtime_error("Uploaded file is empty or could not be read.");
        }
        
        // Check if a big note already exists for this class
        std::ostringstream noteExistsQuery;
        noteExistsQuery << "SELECT COUNT(*) FROM notes WHERE class_id = " << classId << ";";
        
        if (!DAL::query_returns_results(noteExistsQuery.str())) {
            // If no big note exists, create one with this content
            return createBigNote(classId, userId, uploadedContent, title.empty() ? "Uploaded Note" : title);
        } else {
            // If a big note exists, integrate this content
            std::ostringstream filePathQuery;
            filePathQuery << "SELECT file_path FROM notes WHERE class_id = " << classId << ";";
            
            std::string existingFilePath = DAL::get_single_result(filePathQuery.str());
            if (existingFilePath.empty()) {
                throw std::runtime_error("Existing note file path could not be retrieved.");
            }
            
            // Read existing content
            std::string existingContent = DAL::read_file(existingFilePath);
            
            // Integrate the new content with existing content
            // This could be a simple append or a more sophisticated merge algorithm
            std::string integratedContent = existingContent + "\n\n--- New Upload ---\n\n" + uploadedContent;
            
            // Write the integrated content back to the file
            if (!DAL::write_file(existingFilePath, integratedContent)) {
                throw std::runtime_error("Failed to write integrated content to file.");
            }
            
            // Update the database timestamp
            std::ostringstream updateQuery;
            updateQuery << "UPDATE notes SET updated_at = NOW() WHERE class_id = " << classId << ";";
            
            if (!DAL::execute_query(updateQuery.str())) {
                throw std::runtime_error("Failed to update note timestamp in database.");
            }
            
            return true;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to upload note: " + std::string(e.what()));
    }
    return false;
}

// Edit the big note for a class
bool editBigNote(int classId, int userId, const std::string& content, const std::string& title) {
    try {
        // Verify user has access to this class
        std::ostringstream accessCheckStream;
        accessCheckStream << "SELECT COUNT(*) FROM user_classes "
                         << "WHERE class_id = " << classId 
                         << " AND user_id = " << userId << ";";
        
        if (!DAL::query_returns_results(accessCheckStream.str())) {
            throw std::runtime_error("User is not enrolled in this class.");
        }
        
        // Check if a note exists for this class
        std::ostringstream noteExistsQuery;
        noteExistsQuery << "SELECT COUNT(*) FROM notes WHERE class_id = " << classId << ";";
        
        if (!DAL::query_returns_results(noteExistsQuery.str())) {
            throw std::runtime_error("No big note exists for this class. Use createBigNote first.");
        }
        
        // Use DAL to retrieve the file path for the note
        std::ostringstream filePathQuery;
        filePathQuery << "SELECT file_path FROM notes WHERE class_id = " << classId << ";";
        
        std::string filePath = DAL::get_single_result(filePathQuery.str());
        if (filePath.empty()) {
            throw std::runtime_error("Note file path could not be retrieved from database.");
        }

        // Use DAL to write the updated content to the file
        if (!DAL::write_file(filePath, content)) {
            throw std::runtime_error("Failed to write updated note content to file at path: " + filePath);
        }

        // Update the database with the new title if provided
        if (!title.empty()) {
            std::ostringstream updateTitleQuery;
            updateTitleQuery << "UPDATE notes SET title = '" << DAL::escape_string(title) 
                            << "', updated_at = NOW() WHERE class_id = " << classId << ";";
            
            if (!DAL::execute_query(updateTitleQuery.str())) {
                throw std::runtime_error("Failed to update note title in database.");
            }
        } else {
            // Just update the timestamp
            std::ostringstream updateQuery;
            updateQuery << "UPDATE notes SET updated_at = NOW() WHERE class_id = " << classId << ";";
            
            if (!DAL::execute_query(updateQuery.str())) {
                throw std::runtime_error("Failed to update note timestamp in database.");
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to edit big note: " + std::string(e.what()));
    }
    return false;
}

} // namespace Core