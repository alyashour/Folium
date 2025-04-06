/**
 * @file auth.h
 *
 * @brief This file defines the authentication-related functionality for the Folium server.
 * Responsible for the auth, rbac, and session management (jwt) api.
 */

#ifndef FOLSERV_AUTH_H_
#define FOLSERV_AUTH_H_

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

    /**
     * @brief Checks a user's credentials.
     * @param username 
     * @param password 
     * @return true if they can successfully log in.
     */
    bool check_credentials(const std::string& username, const std::string& password);

    /**
     * @brief Begins a user's session.
     * Might not be needed. idk
     * @param username 
     */
    void login(const std::string& username);

    /**
     * @brief Function to terminate the user's session
     * @param username 
     */
    void logout(const std::string& username);
    
    /**
     * @brief Register a new user.
     * @param username
     * @param password unhashed.
     * @return true if the registration was successful.
     */
    bool registerUser(const std::string& username, const std::string& password);

    /**
     * @brief Function to validate an authentication token
     * @param token The JWT
     * @return true if the operation was a success
     */
    bool validateToken(const std::string& token);

    /**
     * @brief Function to refresh an expired token
     * @param token The expired JWT
     * @return A new refreshed token
     */
    std::string refreshToken(const std::string& token);

    /**
     * @brief Utility function to hash a password
     * @param password The plain-text password
     * @return The hashed password
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief Function to verify a hashed password against a plain-text password
     * @param hashedPassword The hashed password
     * @param plainPassword The plain-text password
     * @return true if the passwords match
     */
    bool verifyPassword(const std::string& hashedPassword, const std::string& plainPassword);

    /**
     * @brief Function to get the role of a user (e.g., "admin" or "user")
     * @param username The username of the user
     * @return The role of the user
     */
    Role getUserRole(const std::string& username);

    /**
     * @brief Function to check if a user has the required permissions, considering their role
     * @param username The username of the user
     * @param permission The required permission
     * @param role The role of the user
     * @return true if the user has the required permissions
     */
    bool checkPermissions(const std::string& username, const Permission permission, const Role role);
}

#endif