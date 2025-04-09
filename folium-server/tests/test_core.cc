#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <data_access_layer.h>

#include "core.h"

// Helper function to create a test file
bool create_test_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) return false;
    file << content;
    return file.good();
}

// Setup fixture for core tests
class CoreTest : public ::testing::Test {
protected:
    int testClassId = 1;  // Use a high number to avoid conflicts
    int testUserId = 1;
    std::string testFilePath = "test_upload.txt";
    std::string testContent = "This is test content for unit testing.";
    std::string testTitle = "Test Note Title";

    void SetUp() override {
        // Set up the database with test data
        // This would typically be done via SQL or mocks
        // For now, we'll assume the test database is already set up with:
        // 1. A user with ID testUserId
        // 2. A class with ID testClassId
        // 3. The user is enrolled in the class
        
        // Create test upload file
        create_test_file(testFilePath, testContent);
    }

    void TearDown() override {
        // Clean up test data
        try {
            // Try to delete the test note if it exists
            std::ostringstream cleanupQuery;
            cleanupQuery << "DELETE FROM notes WHERE class_id = " << testClassId << ";";
            DAL::execute_query(cleanupQuery.str());
        } catch (...) {
            // Ignore errors in cleanup
        }
        
        // Remove test file if it exists
        if (std::filesystem::exists(testFilePath)) {
            std::filesystem::remove(testFilePath);
        }
        
        // Clean up notes directory files for this test
        std::string noteFile = "notes/class_" + std::to_string(testClassId) + "_note.txt";
        if (std::filesystem::exists(noteFile)) {
            std::filesystem::remove(noteFile);
        }
    }
};

// Test creating a big note
TEST_F(CoreTest, CreateBigNote) {
    // Test the createBigNote function
    ASSERT_NO_THROW({
        bool result = Core::createBigNote(testClassId, testUserId, testContent, testTitle);
        EXPECT_TRUE(result);
    });
    
    // Verify the note was created
    std::string notePath = "notes/class_" + std::to_string(testClassId) + "_note.txt";
    ASSERT_TRUE(std::filesystem::exists(notePath));
    
    // Verify content was written correctly
    std::ifstream noteFile(notePath);
    std::string content((std::istreambuf_iterator<char>(noteFile)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, testContent);
}

// Test retrieving a big note
TEST_F(CoreTest, GetBigNote) {
    // First create a note
    ASSERT_NO_THROW(Core::createBigNote(testClassId, testUserId, testContent, testTitle));
    
    // Now test retrieving it
    std::string retrievedContent;
    ASSERT_NO_THROW({
        retrievedContent = Core::getBigNote(testClassId, testUserId);
    });
    
    // Check content matches
    EXPECT_EQ(retrievedContent, testContent);
}

// Test editing a big note
TEST_F(CoreTest, EditBigNote) {
    // First create a note
    ASSERT_NO_THROW(Core::createBigNote(testClassId, testUserId, testContent, testTitle));
    
    // New content to update with
    std::string updatedContent = "This is updated content.";
    std::string updatedTitle = "Updated Title";
    
    // Test editing the note
    ASSERT_NO_THROW({
        bool result = Core::editBigNote(testClassId, testUserId, updatedContent, updatedTitle);
        EXPECT_TRUE(result);
    });
    
    // Retrieve and verify the updated content
    std::string retrievedContent;
    ASSERT_NO_THROW({
        retrievedContent = Core::getBigNote(testClassId, testUserId);
    });
    
    EXPECT_EQ(retrievedContent, updatedContent);
}

// Test uploading a note
TEST_F(CoreTest, UploadNote) {
    // Test uploading a note (creating if it doesn't exist)
    ASSERT_NO_THROW({
        bool result = Core::uploadNote(testClassId, testUserId, testFilePath, testTitle);
        EXPECT_TRUE(result);
    });
    
    // Retrieve and verify content
    std::string retrievedContent;
    ASSERT_NO_THROW({
        retrievedContent = Core::getBigNote(testClassId, testUserId);
    });
    
    // Content should match the test file
    EXPECT_EQ(retrievedContent, testContent);
}

// Test uploading to an existing note
TEST_F(CoreTest, UploadToExistingNote) {
    // First create a note
    ASSERT_NO_THROW(Core::createBigNote(testClassId, testUserId, "Original content", testTitle));
    
    // Create a different test file
    std::string appendFilePath = "append_test.txt";
    std::string appendContent = "This is content to be appended.";
    create_test_file(appendFilePath, appendContent);
    
    // Upload to the existing note
    ASSERT_NO_THROW({
        bool result = Core::uploadNote(testClassId, testUserId, appendFilePath, "Appended Note");
        EXPECT_TRUE(result);
    });
    
    // Retrieve and verify content
    std::string retrievedContent;
    ASSERT_NO_THROW({
        retrievedContent = Core::getBigNote(testClassId, testUserId);
    });
    
    // The content should now contain both pieces
    EXPECT_TRUE(retrievedContent.find("Original content") != std::string::npos);
    EXPECT_TRUE(retrievedContent.find("This is content to be appended") != std::string::npos);
    
    // Clean up the extra test file
    std::filesystem::remove(appendFilePath);
}

// Test error handling - accessing class without enrollment
TEST_F(CoreTest, AccessErrorHandling) {
    int unauthorizedUserId = 9999; // A user ID that's not enrolled
    
    EXPECT_THROW({
        Core::getBigNote(testClassId, unauthorizedUserId);
    }, std::runtime_error);
}

// Test error handling - non-existent note
TEST_F(CoreTest, NonExistentNoteErrorHandling) {
    int nonExistentClassId = 8888; // A class ID that doesn't exist or have a note
    
    EXPECT_THROW({
        Core::getBigNote(nonExistentClassId, testUserId);
    }, std::runtime_error);
}