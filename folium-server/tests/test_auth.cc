#include <gtest/gtest.h>
#include "auth.h" // Include your Auth header

// Fixture for Auth tests if needed
class AuthTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Optional: clear state, mock database, etc.
    }

    void TearDown() override {
        // Optional: cleanup
    }
};

TEST_F(AuthTest, SeedDefaultUserIfNotExists) {
    // First ensure the user doesn't exist (if your system allows deleting users in tests)
    if (!Auth::check_credentials("admin", "password")) {
        ASSERT_TRUE(Auth::registerUser("admin", "password")) << "Failed to seed default user 'admin'";
    }

    // User should now exist
    EXPECT_TRUE(Auth::check_credentials("admin", "password")) << "Default user 'admin' not found after registration";
}

TEST_F(AuthTest, LoginWithDefaultCredentials) {
    // Ensure user exists (seed first if necessary)
    if (!Auth::check_credentials("admin", "password")) {
        ASSERT_TRUE(Auth::registerUser("admin", "password")) << "Failed to seed default user 'admin'";
    }

    // Attempt login
    Auth::login("admin");

    // You might need a function to check if "admin" is logged in
    // Replace `is_logged_in` with the correct function if available
    EXPECT_TRUE(Auth::isLoggedIn("admin")) << "Login failed for user 'admin'";
}
