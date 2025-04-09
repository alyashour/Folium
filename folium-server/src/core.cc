#include "core.h"
#include "data_access_layer.h"
#include <stdexcept>
#include <sstream>
#include <fstream> 
#include <nlohmann/json.hpp>
#include <filesystem>
#include <ctime>  // For std::time()

using json = nlohmann::json;

namespace Core {

// Retrieve the big note for a specific class
json getBigNote(int classId, int userId) {
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
            // Return an empty JSON object instead of throwing an error
            return json::object();
        }

        // Check if the file exists
        if (!std::filesystem::exists(filePath)) {
            throw std::runtime_error("Note file does not exist at path: " + filePath);
        }

        // Read the file content
        std::string fileContent = DAL::readFile(filePath);
        if (fileContent.empty()) {
            return json::object(); // Return empty JSON object
        }

        // Parse the file content as JSON
        json noteJson;
        try {
            noteJson = json::parse(fileContent);
            return noteJson;
        } catch (const json::parse_error& e) {
            throw std::runtime_error("Failed to parse note content as JSON: " + std::string(e.what()));
        }
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

        // Ensure the notes directory exists
        std::filesystem::create_directories("notes");

        // Create the note file as JSON
        std::string notePath = "notes/class_" + std::to_string(classId) + "_note.json";
        
        // Create a proper JSON structure
        json noteJson;
        try {
            // Try to parse the content as JSON first
            noteJson = json::parse(content);
        } catch (const json::parse_error& e) {
            // If parsing fails, wrap the content as a string in a JSON object
            noteJson = {
                {"title", title},
                {"units", json::array({
                    {
                        {"unitId", "unit_1"},
                        {"title", title},
                        {"content", content}
                    }
                })}
            };
        }
        
        // Write the JSON to file
        if (!DAL::writeFile(notePath, noteJson.dump())) {
            throw std::runtime_error("Failed to create note file: " + notePath);
        }

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
        std::string fileContent = DAL::readFile(filePath);
        if (fileContent.empty()) {
            throw std::runtime_error("Uploaded file is empty or could not be read.");
        }

        // Try to parse as JSON, fall back to text content
        json uploadedJson;
        try {
            uploadedJson = json::parse(fileContent);
        } catch (const json::parse_error& e) {
            // Create a basic JSON structure for text content
            uploadedJson = {
                {"title", title.empty() ? "Uploaded Note" : title},
                {"content", fileContent}
            };
        }

        // Check if a note already exists
        std::string existingFilePath = DAL::get_single_result("SELECT file_path FROM notes WHERE class_id = " 
                                    + std::to_string(classId) + ";");
        
        if (existingFilePath.empty()) {
            // Create a new note if none exists
            json newNote = {
                {"title", title.empty() ? "Uploaded Note" : title},
                {"units", json::array({
                    {
                        {"unitId", "unit_1"},
                        {"title", title.empty() ? "Uploaded Note" : title},
                        {"content", uploadedJson.dump()}
                    }
                })}
            };
            return createBigNote(classId, userId, newNote.dump(), title.empty() ? "Uploaded Note" : title);
        }

        // Read the existing note content
        std::string existingContent = DAL::readFile(existingFilePath);
        json existingJson;
        
        if (!existingContent.empty()) {
            try {
                existingJson = json::parse(existingContent);
            } catch (const json::parse_error& e) {
                // If existing content isn't valid JSON, create a new structure
                existingJson = {
                    {"title", title.empty() ? "Note Collection" : title},
                    {"units", json::array()}
                };
            }
        } else {
            // If file is empty or doesn't exist, create a new JSON structure
            existingJson = {
                {"title", title.empty() ? "Note Collection" : title},
                {"units", json::array()}
            };
        }

        // Ensure "units" exists as an array
        if (!existingJson.contains("units")) {
            existingJson["units"] = json::array();
        } else if (!existingJson["units"].is_array()) {
            // If units exists but isn't an array, replace it with an array
            existingJson["units"] = json::array();
        }

        // Generate a unique unit ID
        std::string unitId = "unit_" + std::to_string(
            existingJson["units"].is_array() ? existingJson["units"].size() + 1 : 1);

        // Create a new unit containing the uploaded content
        json newUnit = {
            {"unitId", unitId},
            {"title", title.empty() ? "Uploaded Note" : title},
            {"content", uploadedJson.dump()}
        };

        // Append the new unit to the "units" array
        existingJson["units"].push_back(newUnit);

        // Write the updated JSON back to the file
        if (!DAL::writeFile(existingFilePath, existingJson.dump())) {
            throw std::runtime_error("Failed to write integrated content to file.");
        }

        // Update the database timestamp
        std::string query = "UPDATE notes SET updated_at = NOW() WHERE class_id = " + std::to_string(classId) + ";";
        if (!DAL::execute_query(query)) {
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

        // Read the existing file content
        std::string existingContent = DAL::readFile(filePath);
        json noteJson;
        
        // Process the content
        try {
            // First, try to parse the new content as JSON
            noteJson = json::parse(content);
        } catch (const json::parse_error&) {
            // If the new content is not valid JSON, try to parse the existing file
            try {
                if (!existingContent.empty()) {
                    noteJson = json::parse(existingContent);
                    
                    // Update the title if provided
                    if (!title.empty()) {
                        noteJson["title"] = title;
                    }
                    
                    // Create or update the content in a new unit
                    std::string unitId = "unit_edited_" + std::to_string(std::time(nullptr));
                    
                    json newUnit = {
                        {"unitId", unitId},
                        {"title", title.empty() ? "Edited Note" : title},
                        {"content", content}
                    };
                    
                    // Ensure units array exists
                    if (!noteJson.contains("units")) {
                        noteJson["units"] = json::array();
                    } else if (!noteJson["units"].is_array()) {
                        noteJson["units"] = json::array();
                    }
                    
                    noteJson["units"].push_back(newUnit);
                } else {
                    // If there's no existing content, create a new JSON structure
                    noteJson = {
                        {"title", title.empty() ? "Edited Note" : title},
                        {"units", json::array({
                            {
                                {"unitId", "unit_1"},
                                {"title", title.empty() ? "Edited Note" : title},
                                {"content", content}
                            }
                        })}
                    };
                }
            } catch (const std::exception&) {
                // If anything goes wrong with the existing file, create a new structure
                noteJson = {
                    {"title", title.empty() ? "Edited Note" : title},
                    {"units", json::array({
                        {
                            {"unitId", "unit_1"},
                            {"title", title.empty() ? "Edited Note" : title},
                            {"content", content}
                        }
                    })}
                };
            }
        }

        // Write the updated content to the file
        if (!DAL::writeFile(filePath, noteJson.dump())) {
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