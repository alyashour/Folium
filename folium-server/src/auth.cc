/**
 * @file auth.cc
 *
 * @brief authentication implementation for the Folium server.
 *
 * Implements login, logout, create user, change password, token validation, and token refresh.
 * All operations rely entirely on the database via the DAL and do not store any session data in memory.
 *
 * All functionality is keyed by username: a logged-in user (one with an active token in the database)
 * is allowed to perform every operation.
 *
 * Changes in this version:
 *   - login returns the JWT token string.
 *   - registerUser returns the new user's ID and throws a std::runtime_error on error.
 *   - check_credentials takes a std::string& parameter to report error messages.
 */

#include "auth.h"
#include "data_access_layer.h"

#include <jwt-cpp/jwt.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <optional>
#include <stdexcept>

// Using declarations
using json = nlohmann::json;

// JWT secret key (in production, store securely and do not hard-code)
static const std::string jwt_secret = "operating_systems";

// -----------------------------------------------------------------------------
// Helper Function: Hash a password using SHA256.
// (For demonstration purposes only – in production use bcrypt, Argon2, or similar.)
std::string hashPasswordImpl(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// -----------------------------------------------------------------------------
// Helper Function: Verify that a plain-text password matches its hashed version.
bool verifyPasswordImpl(const std::string& hashedPassword, const std::string& plainPassword) {
    return hashPasswordImpl(plainPassword) == hashedPassword;
}

// -----------------------------------------------------------------------------
// check_credentials:
// Retrieves the user record via the DAL and verifies the provided password.
// Writes an error message to 'message' if the user is not found or the password is incorrect.
bool auth::check_credentials(const std::string& username, const std::string& password, std::string &message) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        message = "User not found";
        return false;
    }
    if (!verifyPasswordImpl(userOpt->password_hash, password)) {
        message = "Bad password";
        return false;
    }
    message = "Credentials valid";
    return true;
}

// -----------------------------------------------------------------------------
// login:
// Generates a JWT token for the specified user, stores it in the database, and returns the token.
// Returns an empty string if login fails.
std::string auth::login(const std::string& username) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        std::cerr << "Login failed: user '" << username << "' not found." << std::endl;
        return "";
    }
    int user_id = userOpt->id;

    // Generate a JWT token with a 24-hour expiry.
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto exp = now + std::chrono::hours(24);
    auto exp_time = std::chrono::system_clock::to_time_t(exp);

    std::string token = jwt::create()
                            .set_type("JWT")
                            .set_issued_at(now_time)
                            .set_expires_at(exp_time)
                            .set_subject(std::to_string(user_id))
                            .sign(jwt::algorithm::hs256{jwt_secret});

    // Store the token in the database.
    if (!DAL::insertToken(token, user_id, exp_time)) {
        std::cerr << "Error storing token for user '" << username << "'." << std::endl;
        return "";
    }
    return token;
}

// -----------------------------------------------------------------------------
// logout:
// Revokes the user's session by deleting all tokens associated with the user.
// Uses the username to retrieve the user record and then deletes tokens via the DAL.
void auth::logout(const std::string& username) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        std::cerr << "Logout failed: user '" << username << "' not found." << std::endl;
        return;
    }
    if (!DAL::deleteTokensForUser(userOpt->id)) {
        std::cerr << "Logout failed: unable to delete tokens for user '" << username << "'." << std::endl;
        return;
    }
    std::cout << "Logout successful for user '" << username << "'." << std::endl;
}

// -----------------------------------------------------------------------------
// registerUser:
// Registers a new user by hashing the provided password and storing the user via the DAL.
// Returns the new user's ID if registration is successful; otherwise, throws a std::runtime_error.
int auth::registerUser(const std::string& username, const std::string& password) {
    // Check if the user already exists.
    if (DAL::getUserByUsername(username).has_value()) {
        throw std::runtime_error("User already exists");
    }
    std::string hashed = hashPasswordImpl(password);
    if (!DAL::createUser(username, hashed)) {
        throw std::runtime_error("Failed to create user due to a database error");
    }
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        throw std::runtime_error("User creation succeeded but could not retrieve user ID");
    }
    return userOpt->id;
}

// -----------------------------------------------------------------------------
// validateToken:
// Validates a JWT token by verifying its signature and expiration, then confirms via the DAL
// that the token is still active in the database.
bool auth::validateToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{jwt_secret});
        verifier.verify(decoded);
        return DAL::tokenExists(token);
    } catch (const std::exception& e) {
        std::cerr << "Token validation error: " << e.what() << std::endl;
        return false;
    }
}

// -----------------------------------------------------------------------------
// refreshToken:
// Refreshes a token by generating a new token for the same user, revoking the old token,
// and storing the new token in the database.
std::string auth::refreshToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        std::string subject = decoded.get_subject();

        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto exp = now + std::chrono::hours(24);
        auto exp_time = std::chrono::system_clock::to_time_t(exp);

        std::string newToken = jwt::create()
                                .set_type("JWT")
                                .set_issued_at(now_time)
                                .set_expires_at(exp_time)
                                .set_subject(subject)
                                .sign(jwt::algorithm::hs256{jwt_secret});
        // Revoke the old token.
        DAL::deleteToken(token);
        int user_id = std::stoi(subject);
        if (!DAL::insertToken(newToken, user_id, exp_time)) {
            std::cerr << "Error storing refreshed token." << std::endl;
            return "";
        }
        return newToken;
    } catch (const std::exception& e) {
        std::cerr << "Error refreshing token: " << e.what() << std::endl;
        return "";
    }
}

// -----------------------------------------------------------------------------
// changePassword:
// Changes the user's password by verifying the old password, hashing the new password,
// updating the database via the DAL, and revoking all active tokens (forcing re-authentication).
bool auth::changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        std::cerr << "Change password failed: user '" << username << "' not found." << std::endl;
        return false;
    }
    auto user = userOpt.value();
    if (!verifyPasswordImpl(user.password_hash, oldPassword)) {
        std::cerr << "Change password failed: incorrect old password for user '" << username << "'." << std::endl;
        return false;
    }
    std::string newHashed = hashPasswordImpl(newPassword);
    if (!DAL::updateUserPassword(username, newHashed)) {
        std::cerr << "Change password failed: unable to update password for user '" << username << "'." << std::endl;
        return false;
    }
    // Revoke all tokens for the user so they must log in again.
    if (!DAL::deleteTokensForUser(user.id)) {
        std::cerr << "Change password warning: failed to revoke tokens for user '" << username << "'." << std::endl;
    }
    std::cout << "Password changed successfully for user '" << username << "'." << std::endl;
    return true;
}

// -----------------------------------------------------------------------------
// isLoggedIn:
// Checks if a user is logged in by verifying if the user has at least one active token in the database.
bool auth::isLoggedIn(const std::string& username) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        return false;
    }
    return DAL::userHasActiveToken(userOpt->id);
}

/*
 * Usage Example:
 *
 * try {
 *     // Register a new user:
 *     int userID = auth::registerUser("alice", "secret123");
 *     std::cout << "User 'alice' registered with userID: " << userID << std::endl;
 * } catch (const std::exception& ex) {
 *     std::cerr << "Registration failed for 'alice': " << ex.what() << std::endl;
 * }
 *
 * // Check credentials:
 * std::string credMsg;
 * if (auth::check_credentials("alice", "secret123", credMsg)) {
 *     std::cout << "Credentials for 'alice' are valid: " << credMsg << std::endl;
 * } else {
 *     std::cerr << "Credential check failed for 'alice': " << credMsg << std::endl;
 * }
 *
 * // Log in the user (token is returned):
 * std::string token = auth::login("alice");
 * if (!token.empty()) {
 *     std::cout << "Login successful for 'alice'. Token: " << token << std::endl;
 * }
 *
 * // Validate the token:
 * if (auth::validateToken(token)) {
 *     std::cout << "User 'alice' is logged in (token is valid)." << std::endl;
 * } else {
 *     std::cout << "User 'alice' is not logged in (token is invalid or expired)." << std::endl;
 * }
 *
 * // Check if a user is logged in by username:
 * if (auth::isLoggedIn("alice")) {
 *     std::cout << "User 'alice' is currently logged in." << std::endl;
 * }
 *
 * // Change the user's password:
 * if (auth::changePassword("alice", "secret123", "newSecret456")) {
 *     std::cout << "Password for 'alice' changed successfully." << std::endl;
 * } else {
 *     std::cerr << "Failed to change password for 'alice'." << std::endl;
 * }
 *
 * // Logout (revokes all tokens):
 * auth::logout("alice");
 *
 * // Note:
 * // All operations use the username to identify the user.
 * // Token-based validation (auth::validateToken) is used to check whether a user is logged in.
 */
