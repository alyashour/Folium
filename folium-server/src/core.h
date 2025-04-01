#ifndef FOL_CORE_H
#define FOL_CORE_H


// Core functionality for the Folium server
#include <iostream>
#include <condition_variable>


namespace Core{

    std::string getNote(int noteID, int userID); // Retrieve data by ID
    void updateNote(int noteID, const std::string& noteDATA); // Update data by ID
    void createNote(int noteID, int userID, const std::string& noteDATA); // Create new data
    void editNote(int noteID, int userID, const std::string& noteDATA); // Edit data by ID
    
}

#endif // FOL_CORE_H