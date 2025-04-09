// test_auth.cc

#include <string>
#include <gtest/gtest.h>
#include <stdexcept>
#include <optional>
#include <unordered_map>

// Include the auth header which in turn uses the DAL functions.
#include "auth.h"

// We also need jwt-cpp for decoding tokens in our tests.
#include <jwt-cpp/jwt.h>

// -----------------------------------------------------------------------------
// Fake Data Access Layer implementation for testing auth.
// These definitions override the real DAL functions during testing.
namespace DAL {

struct User {
    int id;
    std::string username;
    std::string password_hash;
};

// Simple in-memory fake database mapping username to User.
static std::unordered_map<std::string, User> fake_db;
static int next_user_id = 1;

std::optional<User> getUserByUsername(const std::string& username) {
    auto it = fake_db.find(username);
    if (it != fake_db.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool createUser(const std::string& username, const std::string& passwordHash) {
    if (fake_db.find(username) != fake_db.end()) {
        return false; // Duplicate user
    }
    User newUser;
    newUser.id = next_user_id++;
    newUser.username = username;
    newUser.password_hash = passwordHash;
    fake_db[username] = newUser;
    return true;
}

bool updateUserPassword(const std::string& username, const std::string& newPasswordHash) {
    auto it = fake_db.find(username);
    if (it == fake_db.end())
        return false;
    it->second.password_hash = newPasswordHash;
    return true;
}

// Helper to clear the fake database for independent tests.
void clear() {
    fake_db.clear();
    next_user_id = 1;
}

}  // namespace DAL

// -----------------------------------------------------------------------------
// Test Fixture to clear DAL before each test.
class AuthTest : public ::testing::Test {
protected:
    void SetUp() override {
        DAL::clear();
    }
};

// -----------------------------------------------------------------------------
// Test cases for check_credentials.

// Valid credentials: register a user via auth::registerUser then verify credentials.
TEST_F(AuthTest, CheckCredentialsValid) {
    std::string message;
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    bool valid = auth::check_credentials("testuser", "secret", message);
    EXPECT_TRUE(valid) << "Expected valid credentials for registered user.";
    // In the success case, message should not be "User not found" nor "Bad password".
    EXPECT_NE(message, "User not found");
    EXPECT_NE(message, "Bad password");
}

// Non-existent user returns false with the proper message.
TEST_F(AuthTest, CheckCredentialsUserNotFound) {
    std::string message = "initial";
    bool valid = auth::check_credentials("nonexistent", "password", message);
    EXPECT_FALSE(valid);
    EXPECT_EQ(message, "User not found");
}

// Wrong password: existing user but with an incorrect password.
TEST_F(AuthTest, CheckCredentialsBadPassword) {
    std::string message;
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    bool valid = auth::check_credentials("testuser", "wrongpassword", message);
    EXPECT_FALSE(valid);
    EXPECT_EQ(message, "Bad password");
}

// -----------------------------------------------------------------------------
// Test cases for login.

// Successful login returns a non-empty token which validates correctly.
TEST_F(AuthTest, LoginSuccess) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    std::string token = auth::login("testuser");
    EXPECT_FALSE(token.empty()) << "Expected a valid token on successful login.";
    // The returned token should be valid.
    bool valid = auth::validateToken(token);
    EXPECT_TRUE(valid);
}

// Login with a non-existent user should return an empty token.
TEST_F(AuthTest, LoginUserNotFound) {
    std::string token = auth::login("nonexistent");
    EXPECT_TRUE(token.empty());
}

// -----------------------------------------------------------------------------
// Test for logout: since logout only logs the event, ensure it runs without error.
TEST_F(AuthTest, LogoutDoesNotCrash) {
    auth::logout("testuser");
    SUCCEED();
}

// -----------------------------------------------------------------------------
// Test cases for registerUser.

// Successful registration returns a new user ID and registers the user in DAL.
TEST_F(AuthTest, RegisterUserSuccess) {
    int uid = auth::registerUser("newuser", "mypassword");
    EXPECT_GT(uid, 0);
    auto userOpt = DAL::getUserByUsername("newuser");
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->id, uid);
}

// Attempting to register a duplicate user should throw a runtime_error.
TEST_F(AuthTest, RegisterUserDuplicate) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    EXPECT_THROW({
        auth::registerUser("testuser", "secret");
    }, std::runtime_error);
}

// -----------------------------------------------------------------------------
// Test cases for validateToken.

// A valid token (from login) should pass validation.
TEST_F(AuthTest, ValidateTokenValid) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    std::string token = auth::login("testuser");
    bool valid = auth::validateToken(token);
    EXPECT_TRUE(valid);
}

// An obviously invalid token should fail validation.
TEST_F(AuthTest, ValidateTokenInvalid) {
    bool valid = auth::validateToken("invalid.token.string");
    EXPECT_FALSE(valid);
}

// -----------------------------------------------------------------------------
// Test cases for refreshToken.

// Refreshing a valid token should return a new valid token with the same subject (user ID).
TEST_F(AuthTest, RefreshTokenSuccess) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    std::string token = auth::login("testuser");
    std::string newToken = auth::refreshToken(token);
    EXPECT_FALSE(newToken.empty());
    EXPECT_TRUE(auth::validateToken(newToken));

    // Decode tokens to verify that the subject (user ID) is unchanged.
    auto decodedOld = jwt::decode(token);
    auto decodedNew = jwt::decode(newToken);
    EXPECT_EQ(decodedOld.get_subject(), decodedNew.get_subject());
}

// Passing an invalid token to refreshToken should throw an exception.
TEST_F(AuthTest, RefreshTokenInvalidThrows) {
    EXPECT_THROW({
        auth::refreshToken("invalid.token.string");
    }, std::runtime_error);
}

// -----------------------------------------------------------------------------
// Test cases for changePassword.

// Successfully changing the password: the old password becomes invalid and the new one works.
TEST_F(AuthTest, ChangePasswordSuccess) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    bool changed = auth::changePassword("testuser", "secret", "newsecret");
    EXPECT_TRUE(changed);
    std::string message;
    bool validOld = auth::check_credentials("testuser", "secret", message);
    EXPECT_FALSE(validOld);
    EXPECT_EQ(message, "Bad password");

    bool validNew = auth::check_credentials("testuser", "newsecret", message);
    EXPECT_TRUE(validNew);
}

// Attempting to change password using an incorrect old password should fail.
TEST_F(AuthTest, ChangePasswordWrongOldPassword) {
    int uid = auth::registerUser("testuser", "secret");
    EXPECT_GT(uid, 0);
    bool changed = auth::changePassword("testuser", "wrongsecret", "newsecret");
    EXPECT_FALSE(changed);
    std::string message;
    bool validOld = auth::check_credentials("testuser", "secret", message);
    EXPECT_TRUE(validOld);
}

// Changing the password for a non-existent user should return false.
TEST_F(AuthTest, ChangePasswordUserNotFound) {
    bool changed = auth::changePassword("nonexistent", "any", "newpass");
    EXPECT_FALSE(changed);
}