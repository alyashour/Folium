#include "auth.h"
#include <iostream>
#include <string>
#include <unordered_map>

// A simple User structure to store the hashed password and role.
struct User {
    std::string password;  // In production, store securely hashed password.
    Auth::Role role;
};

// In-memory store for users.
static std::unordered_map<std::string, User> users;

// Helper to seed the default admin user at startup.
static void initDefaultUser() {
    if (users.find("admin") == users.end()) {
        // For the prototype, the default admin password is "password".
        users["admin"] = { Auth::hashPassword("password"), Auth::ADMIN };
        std::cout << "Default admin user seeded." << std::endl;
    }
}

namespace Auth {

    // Checks the credentials for the given user.
    bool check_credentials(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it != users.end()) {
            return verifyPassword(it->second.password, password);
        }
        return false;
    }

    // Begins a user's session.
    void login(const std::string& username) {
        std::cout << "User " << username << " logged in." << std::endl;
        // In a real system, you would start a session or log this event.
    }

    // Terminates the user's session.
    void logout(const std::string& username) {
        std::cout << "User " << username << " logged out." << std::endl;
        // Here you would clean up session data.
    }

    // Registers a new user. Returns true if registration succeeds.
    bool registerUser(const std::string& username, const std::string& password) {
        if (users.find(username) != users.end()) {
            std::cerr << "User " << username << " already exists." << std::endl;
            return false;
        }
        users[username] = { hashPassword(password), Auth::USER };
        std::cout << "User " << username << " registered successfully." << std::endl;
        return true;
    }

    // Validates a token.
    // For this prototype, a token is considered valid if it begins with "token_for_".
    bool validateToken(const std::string& token) {
        return token.find("token_for_") == 0;
    }

    // Refreshes an expired token by appending "_refreshed" to it.
    std::string refreshToken(const std::string& token) {
        if (validateToken(token)) {
            return token + "_refreshed";
        }
        return "";
    }

    // A naive hash function that prepends "hashed_" to a password.
    // WARNING: This is only for prototyping purposes.
    std::string hashPassword(const std::string& password) {
        return "hashed_" + password;
    }

    // Verifies a plaintext password against a stored hashed password.
    bool verifyPassword(const std::string& hashedPassword, const std::string& plainPassword) {
        return hashedPassword == hashPassword(plainPassword);
    }

    // Returns the role of a user. Defaults to USER if not found.
    Role getUserRole(const std::string& username) {
        auto it = users.find(username);
        if (it != users.end()) {
            return it->second.role;
        }
        return USER;
    }

    // Checks whether the given user has the required permission based on role.
    // For demonstration, ADMINs have all permissions; for USERs, only CAN_CREATE_NOTE is allowed.
    bool checkPermissions(const std::string& username, const Permission permission, const Role role) {
        auto it = users.find(username);
        if (it == users.end()) {
            return false;
        }
        if (it->second.role != role) {
            return false;
        }
        if (role == ADMIN) {
            return true;
        }
        if (role == USER) {
            // For now, only CAN_CREATE_NOTE is allowed for USER.
            return (permission == CAN_CREATE_NOTE);
        }
        return false;
    }

    // Additional function for changing the password.
    // Returns true if the password was changed successfully.
    bool changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "User " << username << " not found." << std::endl;
            return false;
        }
        if (!verifyPassword(it->second.password, oldPassword)) {
            std::cerr << "Old password is incorrect for user " << username << std::endl;
            return false;
        }
        it->second.password = hashPassword(newPassword);
        std::cout << "Password changed successfully for user " << username << std::endl;
        return true;
    }

} // namespace Auth

// Static initializer to seed the default admin user upon module load.
struct AuthInitializer {
    AuthInitializer() {
        initDefaultUser();
    }
};
static AuthInitializer initializer;
