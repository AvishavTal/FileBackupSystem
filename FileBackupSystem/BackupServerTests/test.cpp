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

TEST(FileStorageTests, DeleteFile_FileExists_DeletesFile) {
    FileStorage storage;
    unsigned int userId = 1234;
    std::string filename = "file_to_delete.txt";
    std::vector<std::byte> content = { std::byte{0x41} };
    std::string fullPath = "backupsvr\\" + std::to_string(userId) + "\\" + filename;

    cleanUp("backupsvr");

    // Setup: Create file first
    storage.saveFile(userId, filename, content);
    ASSERT_TRUE(std::filesystem::exists(fullPath));

    // Act
    storage.deleteFile(userId, filename);

    // Assert
    EXPECT_FALSE(std::filesystem::exists(fullPath)) << "File should have been deleted.";

    cleanUp("backupsvr");
}

TEST(FileStorageTests, DeleteFile_FileDoesNotExist_ThrowsException) {
    FileStorage storage;
    unsigned int userId = 1234;
    std::string filename = "missing.txt";

    cleanUp("backupsvr");
    // Ensure user directory exists
    storage.saveFile(userId, "setup.tmp", { std::byte{0x00} });

    // Expect runtime_error (logic for Status 1001)
    EXPECT_THROW(storage.deleteFile(userId, filename), std::runtime_error);

    cleanUp("backupsvr");
}

TEST(FileStorageTests, DeleteFile_PathTraversal_ThrowsException) {
    FileStorage storage;
    // Should fail validation before checking existence
    EXPECT_THROW(storage.deleteFile(1234, "..\\windows\\system32\\calc.exe"), std::invalid_argument);
}

TEST(FileStorageTests, ListFiles_Generates32CharRandomName) {
    FileStorage storage;
    unsigned int userId = 9999;
    cleanUp("backupsvr");

    // Create a dummy file so directory exists
    storage.saveFile(userId, "dummy.txt", { std::byte{0x00} });

    std::string listFileName = storage.listFiles(userId);

    // Verify length and alphanumeric
    EXPECT_EQ(listFileName.length(), 32);
    EXPECT_TRUE(std::all_of(listFileName.begin(), listFileName.end(), [](char c) {
        return std::isalnum(c);
        }));

    cleanUp("backupsvr");
}

TEST(FileStorageTests, ListFiles_ContentListsExistingFiles) {
    FileStorage storage;
    unsigned int userId = 1234;
    cleanUp("backupsvr");

    // Arrange: Create two files
    storage.saveFile(userId, "fileA.txt", { std::byte{0x1} });
    storage.saveFile(userId, "fileB.jpg", { std::byte{0x2} });

    // Act
    std::string listFileName = storage.listFiles(userId);
    std::string listFilePath = "backupsvr\\" + std::to_string(userId) + "\\" + listFileName;

    // Assert: Check content of the generated list file
    ASSERT_TRUE(std::filesystem::exists(listFilePath));

    // --- Scope Block Start ---
    // פותחים סקופ כדי ש-inFile ייסגר אוטומטית בסיומו
    {
        std::ifstream inFile(listFilePath);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(inFile, line)) {
            lines.push_back(line);
        }

        // בדיקה שהקבצים שיצרנו מופיעים ברשימה
        bool foundA = false, foundB = false;
        for (const auto& l : lines) {
            if (l == "fileA.txt") foundA = true;
            if (l == "fileB.jpg") foundB = true;
        }

        EXPECT_TRUE(foundA) << "List file missing fileA.txt";
        EXPECT_TRUE(foundB) << "List file missing fileB.jpg";
    }
    // --- Scope Block End ---
    // כאן הקובץ inFile נסגר פיזית, ולכן ה-cleanUp למטה יעבוד

    cleanUp("backupsvr");
}