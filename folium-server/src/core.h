/**
 * @file core.h
 * @brief Core functionality for managing notes in the Folium server.
 *
 * This file defines the core operations for handling notes, including
 * retrieval, creation, updating, and editing. 
 * 
 * These functions are the "core" (hence the name)
 * for the server's note management system and ensure proper interaction
 * with the database or storage layer.
 */

 #ifndef FOLSERV_CORE_H_
 #define FOLSERV_CORE_H_
 
 // Core functionality for the Folium server
 #include <iostream>
 #include <string>
 #include <vector>
 #include <map>
 
 namespace Core
 {
     /**
      * @brief Retrieves the big note for a specific class
      * @param classId The ID of the class
      * @param userId The ID of the requesting user (for access verification)
      * @return A string containing the note content (could be JSON formatted)
      */
     std::string getBigNote(int classId, int userId);
     
     /**
      * @brief Uploads and integrates a new note into a class's big note
      * @param classId The ID of the class
      * @param userId The ID of the user uploading the note
      * @param filePath Path to the uploaded file
      * @param title Optional title for the note
      * @return True if the upload and integration was successful
      */
     bool uploadNote(int classId, int userId, const std::string& filePath, const std::string& title = "");
     
     /**
      * @brief Directly edits the content of a class's big note
      * @param classId The ID of the class
      * @param userId The ID of the user editing the note
      * @param content The new content for the note
      * @param title Optional new title for the note
      * @return True if the edit was successful
      */
     bool editBigNote(int classId, int userId, const std::string& content, const std::string& title = "");
     
     /**
      * @brief Creates a new big note for a class (internal use, typically called by uploadNote for first-time uploads)
      * @param classId The ID of the class
      * @param userId The ID of the creator
      * @param content Initial content for the note
      * @param title Initial title for the note
      * @return True if creation was successful
      */
     bool createBigNote(int classId, int userId, const std::string& content, const std::string& title);
 }
 
 #endif // FOLSERV_CORE_H_