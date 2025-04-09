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
#include <condition_variable>

namespace Core
{
    std::string getNote(int noteID, int userID);                          // Retrieve data by ID           // Update data by ID
    void createNote(int noteID, int userID, const std::string &noteDATA); // Create new data
    void editNote(int noteID, int userID, const std::string &noteDATA);   // Edit data by ID
}

#endif // FOLSERV_CORE_H_