// test_dal.cc
// To enable transactional integration testing, compile with -DTESTING.
// If no Logger implementation is linked, fallback stub definitions are provided.

#include <gtest/gtest.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdio>      // For std::remove
#include "data_access_layer.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <mysql/mysql.h>


// -----------------------------------------------------------------------------
// File I/O Tests
// We create temporary files in the working directory for these tests.
class DalFileIOTest : public ::testing::Test {
protected:
    const std::string testTxtFile = "test_file.txt";
    const std::string testJsonFile = "test_json.json";

    void TearDown() override {
        // Remove temporary files after each test.
        std::remove(testTxtFile.c_str());
        std::remove(testJsonFile.c_str());
    }
};

TEST_F(DalFileIOTest, WriteAndReadFile) {
    std::string content = "Hello DAL!";
    // Write the file.
    EXPECT_NO_THROW({
        bool writeOk = DAL::writeFile(testTxtFile, content);
        EXPECT_TRUE(writeOk);
    });
    // Read the file (using readFile).
    std::string readContent = DAL::readFile(testTxtFile);
    EXPECT_EQ(readContent, content);
    // Also verify that readTxtFile returns the same content.
    std::string readTxt = DAL::readTxtFile(testTxtFile);
    EXPECT_EQ(readTxt, content);
}

TEST_F(DalFileIOTest, ReadNonExistentFileThrows) {
    EXPECT_THROW({
        DAL::readFile("nonexistent_file.txt");
    }, std::runtime_error);
}

TEST_F(DalFileIOTest, WriteAndReadJsonFile) {
    // Create a JSON object with a "file_path" field.
    nlohmann::json j = {
        {"file_path", testJsonFile},
        {"key", "value"},
        {"number", 42}
    };
    EXPECT_NO_THROW({
        DAL::writeJsonFile(j);
    });
    nlohmann::json jRead = DAL::readJsonFile(testJsonFile);
    EXPECT_EQ(jRead, j);
}

// -----------------------------------------------------------------------------
// Parameter Validation Tests for SQL Functions

TEST(DalParameterTest, GetClassIdsInvalidUserId) {
    EXPECT_THROW({
        DAL::getClassIds(0);
    }, std::invalid_argument);
}

TEST(DalParameterTest, GetNoteIdsInvalidUserId) {
    EXPECT_THROW({
        DAL::getNoteIds(0);
    }, std::invalid_argument);
}

TEST(DalParameterTest, GetNoteFilePathInvalidNoteId) {
    EXPECT_THROW({
        DAL::getNoteFilePath(0);
    }, std::invalid_argument);
}

TEST(DalParameterTest, CreateUserEmptyParams) {
    EXPECT_THROW({
        DAL::createUser("", "somepass");
    }, std::invalid_argument);
    EXPECT_THROW({
        DAL::createUser("someuser", "");
    }, std::invalid_argument);
}

TEST(DalParameterTest, UpdateUserPasswordEmptyParams) {
    EXPECT_THROW({
        DAL::updateUserPassword("", "newpass");
    }, std::invalid_argument);
    EXPECT_THROW({
        DAL::updateUserPassword("someuser", "");
    }, std::invalid_argument);
}

// -----------------------------------------------------------------------------
// Integration Tests (using a live MySQL connection)
// To avoid leaving permanent changes in the database, each integration test runs
// inside a transaction that is rolled back after the test.
// The following fixture assumes that when compiled with -DTESTING, DAL::createConnection()
// returns a persistent connection with autocommit disabled.
class DALIntegrationTransactionTest : public ::testing::Test {
protected:
    MYSQL* testConn; // Persistent connection for use during testing.

    void SetUp() override {
#ifdef TESTING
        // Obtain a persistent connection from your DAL.
        testConn = DAL::createConnection();
        // Start a transaction.
        if (mysql_query(testConn, "START TRANSACTION;") != 0) {
            throw std::runtime_error("Failed to start transaction in test fixture.");
        }
#else
        // In non-testing mode, no transaction wrapping is applied.
        testConn = nullptr;
#endif
    }
    
    void TearDown() override {
#ifdef TESTING
        // Roll back all changes to leave the DB unchanged.
        mysql_query(testConn, "ROLLBACK;");
#endif
    }
};

TEST_F(DALIntegrationTransactionTest, GetTables) {
    std::vector<std::string> tables = DAL::getTables();
    EXPECT_GT(tables.size(), 0) << "Expected some tables in the database.";
}

TEST_F(DALIntegrationTransactionTest, GetUserByUsername) {
    // Expect that your seed data contains the user 'admin'.
    auto userOpt = DAL::getUserByUsername("admin");
    ASSERT_TRUE(userOpt.has_value()) << "User 'admin' not found in the database.";
    EXPECT_EQ(userOpt->username, "admin");
}

TEST_F(DALIntegrationTransactionTest, CreateAndUpdateUser) {
    // Use a unique username for this test.
    std::string username = "integration_test_user";
    // Create a new user.
    bool created = DAL::createUser(username, "initial_hash");
    EXPECT_TRUE(created);
    
    auto userOpt = DAL::getUserByUsername(username);
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->username, username);
    
    // Update the user's password.
    bool updated = DAL::updateUserPassword(username, "updated_hash");
    EXPECT_TRUE(updated);
    
    auto updatedUserOpt = DAL::getUserByUsername(username);
    ASSERT_TRUE(updatedUserOpt.has_value());
    EXPECT_EQ(updatedUserOpt->password_hash, "updated_hash");
}

TEST_F(DALIntegrationTransactionTest, GetNoteIds) {
    // Use a seeded user (for example, 'admin').
    auto userOpt = DAL::getUserByUsername("admin");
    ASSERT_TRUE(userOpt.has_value()) << "User 'admin' not found.";
    unsigned int admin_id = static_cast<unsigned int>(userOpt->id);
    std::vector<int> noteIds = DAL::getNoteIds(admin_id);
    EXPECT_GT(noteIds.size(), 0) << "Expected at least one note id for 'admin'.";
}

TEST_F(DALIntegrationTransactionTest, GetNoteFilePath) {
    // Use a seeded user (for example, 'admin').
    auto userOpt = DAL::getUserByUsername("admin");
    ASSERT_TRUE(userOpt.has_value()) << "User 'admin' not found.";
    unsigned int admin_id = static_cast<unsigned int>(userOpt->id);
    std::vector<int> noteIds = DAL::getNoteIds(admin_id);
    ASSERT_FALSE(noteIds.empty()) << "No note IDs found for 'admin'.";
    std::string filePath = DAL::getNoteFilePath(noteIds.front());
    EXPECT_FALSE(filePath.empty());
}
