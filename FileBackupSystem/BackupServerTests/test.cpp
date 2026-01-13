#include "pch.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstddef> // Required for std::byte
#include "FileStorage.h"

// Helper function to clean up test artifacts
void cleanUp(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

TEST(FileStorageTests, SaveFile_BinaryData_SavesCorrectly) {
    // Arrange
    FileStorage storage;
    unsigned int userId = 1234;
    std::string filename = "test_image.png";

    // Create binary content with mixed values and null bytes
    std::vector<std::byte> content = {
        std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF},
        std::byte{0x00}, std::byte{0xFF}, std::byte{0xAA}
    };

    // Define expected paths (relative)
    std::string expectedDir = "backupsvr\\" + std::to_string(userId);
    std::string expectedPath = expectedDir + "\\" + filename;

    cleanUp("backupsvr");

    // Act
    try {
        storage.saveFile(userId, filename, content);
    }
    catch (const std::exception& e) {
        FAIL() << "Exception thrown: " << e.what();
    }

    // Assert
    EXPECT_TRUE(std::filesystem::exists(expectedPath)) << "File was not created on disk.";

    // Verify content integrity
    // We use a scope block { } to ensure 'inFile' is closed/destroyed BEFORE we call cleanUp
    {
        std::ifstream inFile(expectedPath, std::ios::binary);
        ASSERT_TRUE(inFile.is_open()) << "Failed to open created file.";

        // FIX: Read file manually and cast char to std::byte explicitly
        // The iterator constructor caused a compilation error because char !-> std::byte implicitly
        std::vector<std::byte> savedContent;
        char c;
        while (inFile.get(c)) {
            savedContent.push_back(static_cast<std::byte>(c));
        }

        EXPECT_EQ(savedContent, content) << "Binary content mismatch.";
    } // inFile is closed here automatically

    // Cleanup
    cleanUp("backupsvr");
}

TEST(FileStorageTests, SaveFile_PathTraversal_ThrowsException) {
    FileStorage storage;
    unsigned int userId = 666;
    std::string maliciousFilename = "..\\..\\hack.exe";
    std::vector<std::byte> content = { std::byte{0x01} };

    // Expect invalid_argument due to ".." in filename
    EXPECT_THROW(storage.saveFile(userId, maliciousFilename, content), std::invalid_argument);
}