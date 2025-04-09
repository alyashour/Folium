/**
 * @file auth.h
 *
 * @brief This file defines the authentication-related functionality for the Folium server.
 * All operations rely solely on generating and validating JWT tokens without storing session state in a database.
 *
 * Changes in this version:
 *   - login returns the JWT token string.
 *   - registerUser returns the new user's ID and throws a std::runtime_error on error.
 *   - check_credentials takes an extra std::string& parameter to report error messages.
 *   - Role and permission types/functions are removed.
 */

#ifndef FOLSERV_AUTH_H_
#define FOLSERV_AUTH_H_

#include <string>
#include <stdexcept>

namespace auth {

    // Minimum length constraints.
    const unsigned int MIN_USERNAME_LENGTH = 3;
    const unsigned int MIN_PASSWORD_LENGTH = 5;

    /**
     * @brief Checks a user's credentials.
     * @param username The username.
     * @param password The plain-text password.
     * @param message A reference to a string where an error message will be written if the credentials are invalid.
     * @return true if the credentials are valid, false otherwise.
     */
    bool check_credentials(const std::string& username, const std::string& password, std::string &message);

    /**
     * @brief Logs in a user by generating a new JWT token.
     * @param username The username.
     * @return The newly generated JWT token as a string.
     */
    std::string login(const std::string& username);

    /**
     * @brief Logs out a user.
     * Since this is a stateless token system, logout is handled on the client-side.
     * This function simply logs the logout request.
     * @param username The username.
     */
    void logout(const std::string& username);

    /**
     * @brief Registers a new user.
     * @param username The desired username.
     * @param password The plain-text password.
     * @return The new user's ID if registration is successful.
     * @throws std::runtime_error if registration fails.
     */
    int registerUser(const std::string& username, const std::string& password);

    /**
     * @brief Validates a JWT token.
     * @param token The JWT token.
     * @return true if the token is valid, false otherwise.
     */
    bool validateToken(const std::string& token);

    /**
     * @brief Refreshes an expired token by generating a new token.
     * @param token The expired JWT token.
     * @return A new, refreshed JWT token as a string.
     * @throws std::runtime_error if token refresh fails.
     */
    std::string refreshToken(const std::string& token);

    /**
     * @brief Utility function to hash a password.
     * @param password The plain-text password.
     * @return The hashed password.
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief Utility function to verify a hashed password against a plain-text password.
     * @param hashedPassword The hashed password.
     * @param plainPassword The plain-text password.
     * @return true if the passwords match, false otherwise.
     */
    bool verifyPassword(const std::string& hashedPassword, const std::string& plainPassword);

    /**
     * @brief Changes the user's password.
     * @param username The username.
     * @param oldPassword The old plain-text password.
     * @param newPassword The new plain-text password.
     * @return true if the password was successfully changed.
     */
    bool changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);

    /**
     * @brief Checks if a user is logged in.
     * In this stateless design, token presence is not tracked server-side.
     * @param username The username.
     * @return false always (logged-in state is determined by token validation).
     */
    bool isLoggedIn(const std::string& username);

    /**
     * @brief Function to change a user's password.
     * @param username The username of the user.
     * @param oldPassword The user's current password.
     * @param newPassword The new password to set.
     * @return true if the password was successfully changed, false otherwise.
     */
    bool changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);
}

#endif // FOLSERV_AUTH_H_
