#ifndef FOL_AUTH_H
#define FOL_AUTH_H

#include <iostream>
#include <string>


namespace Auth{
    // might not need this 
    enum Role {
        ADMIN,
        USER
    };
    // we can add more permissions if necessary 
    enum Permission {
        CAN_CREATE_NOTE,
        CAN_DELETE_NOTE,
        CAN_CREATE_USER
    };

    /// @brief 
    /// @param username 
    /// @param password 
    /// @return 
    bool login(const std::string& username, const std::string& password);

    /// @brief Function to terminate the user's session
    /// @param username 
    void logout(const std::string& username);
    
    // Function to register a new user
    bool registerUser(const std::string& username, const std::string& password);

    /// @brief Function to validate an authentication token
    /// @param token The JWT
    /// @return if the operation was a success
    bool validateToken(const std::string& token);

    // Function to refresh an expired token
    // security concern?
    std::string refreshToken(const std::string& token);

    // Utility function to hash a password
    std::string hashPassword(const std::string& password);

    // Function to verify a hashed password against a plain-text password
    bool verifyPassword(const std::string& hashedPassword, const std::string& plainPassword);

    // Function to get the role of a user (e.g., "admin" or "user")
    // NOTE: might not have roles
    Role getUserRole(const std::string& username);

    // Function to check if a user has the required permissions, considering their role
    bool checkPermissions(const std::string& username, const Permission permission, const Role role);
}

#endif