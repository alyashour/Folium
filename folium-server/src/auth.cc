/**
 * @file auth.cc
 *
 * @brief Authentication implementation for the Folium server.
 *
 * Implements login, logout, create user, change password, token validation, and token refresh.
 * All operations rely solely on generating or validating JWT tokens without storing any session data in the database.
 *
 * All functionality is keyed by username: a logged-in user (one that possesses a valid token)
 * is allowed to perform every operation.
 *
 * Changes in this version:
 *   - login returns the JWT token string without saving it to the database.
 *   - registerUser returns the new user's ID and throws a std::runtime_error on error.
 *   - check_credentials takes an extra std::string& parameter to report error messages.
 *   - refreshToken now generates and returns a new token (throwing on error) without performing any token deletion or database operations.
 *   - logout now simply logs the logout request, as tokens are stateless.
 *   - validateToken verifies the token's signature and expiration only.
 *   - isLoggedIn always returns false (since login state is not tracked server-side).
 */

#include "auth.h"
#include "data_access_layer.h"

#include <jwt-cpp/jwt.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include "logger.h"

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
    return true;
}

// -----------------------------------------------------------------------------
// login:
// Generates a JWT token for the specified user and returns the token.
// No token is stored in the database; this function simply generates a new token
// with updated issued-at and expiration times (24 hours from now).
std::string auth::login(const std::string& username) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        Logger::logErr("Login failed: user '" + username + "' not found.");
        return "";
    }
    int user_id = userOpt->id;

    // Generate a JWT token with a 24-hour expiry using the current time.
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(24);

    std::string token = jwt::create()
                            .set_type("JWT")
                            .set_issued_at(now)    // pass the time_point directly
                            .set_expires_at(exp)   // pass the time_point directly
                            .set_subject(std::to_string(user_id))
                            .sign(jwt::algorithm::hs256{jwt_secret});

    Logger::logDebug("Login successful for user '" + username + "'. Token generated.");
    return token;
}

// -----------------------------------------------------------------------------
// logout:
// In a stateless token system, logout is handled on the client side by discarding the token.
// Here we simply log the logout request.
void auth::logout(const std::string& username) {
    Logger::log("Logout requested for user '" + username + "'. No state is maintained server-side.");
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
// Validates a JWT token by verifying its signature and expiration.
// No database lookup is performed since tokens are not stored.
bool auth::validateToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{jwt_secret});
        verifier.verify(decoded);
        return true;
    } catch (const std::exception& e) {
        Logger::logErr("Token validation error: " + std::string(e.what()));
        return false;
    }
}

// -----------------------------------------------------------------------------
// refreshToken:
// Refreshes a token by generating a new token for the same user with updated issued-at
// and expiration times (24 hours from now). Throws std::runtime_error on error.
std::string auth::refreshToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        std::string subject = decoded.get_subject();

    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(24);

    std::string newToken = jwt::create()
        .set_type("JWT")
        .set_issued_at(now)   // pass time_point directly
        .set_expires_at(exp)  // pass time_point directly
        .set_subject(subject)
        .sign(jwt::algorithm::hs256{jwt_secret});

        return newToken;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error refreshing token: ") + e.what());
    }
}

// -----------------------------------------------------------------------------
// changePassword:
// Changes the user's password by verifying the old password, hashing the new password,
// updating the database via the DAL, and (since tokens are stateless) simply returns a success value.
bool auth::changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    auto userOpt = DAL::getUserByUsername(username);
    if (!userOpt.has_value()) {
        Logger::logErr("Change password failed: user '" + username + "' not found.");
        return false;
    }
    auto user = userOpt.value();
    if (!verifyPasswordImpl(user.password_hash, oldPassword)) {
        Logger::logErr("Change password failed: incorrect old password for user '" + username + "'.");
        return false;
    }
    std::string newHashed = hashPasswordImpl(newPassword);
    if (!DAL::updateUserPassword(username, newHashed)) {
        Logger::logErr("Change password failed: unable to update password for user '" + username + "'.");
        return false;
    }
    Logger::log("Password changed successfully for user '" + username + "'.");
    return true;
}

// -----------------------------------------------------------------------------
// isLoggedIn:
// In a stateless design, there's no server-side session tracking, so we cannot determine
// a user's logged-in state solely by username. This function returns false.

bool auth::isLoggedIn(const std::string& /*username*/) {
    return false; // @alyashour I am guessing like you said yea we don't need this function.
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
 * // Refresh the token:
 * try {
 *     std::string newToken = auth::refreshToken(token);
 *     std::cout << "Token refreshed successfully. New token: " << newToken << std::endl;
 * } catch (const std::exception& ex) {
 *     std::cerr << "Token refresh failed: " << ex.what() << std::endl;
 * }
 *
 * // Change the user's password:
 * if (auth::changePassword("alice", "secret123", "newSecret456")) {
 *     std::cout << "Password for 'alice' changed successfully." << std::endl;
 * } else {
 *     std::cerr << "Failed to change password for 'alice'." << std::endl;
 * }
 *
 * // Logout (no state is maintained on the server):
 * auth::logout("alice");
 *
 * // Note:
 * // All operations use the username to identify the user.
 * // Token-based validation (auth::validateToken) is used to check whether a token is valid.
 */
