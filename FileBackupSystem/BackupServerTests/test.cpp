#include "pch.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstddef> // Required for std::byte
#include "FileStorage.h"
#include "Message.h" // וודא שה-Include הזה עובד (אולי צריך ../BackupServerCore/Message.h)


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

TEST(FileStorageTests, RestoreFile_FileExists_ReturnsCorrectContent) {
    FileStorage storage;
    unsigned int userId = 1234;
    std::string filename = "restore_me.bin";
    std::vector<std::byte> originalContent = { std::byte{0x10}, std::byte{0x20}, std::byte{0x30} };

    cleanUp("backupsvr");

    // 1. Setup: Save file
    storage.saveFile(userId, filename, originalContent);

    // 2. Act
    // בשלב ה-Red: השורה הזו תזרוק שגיאה והטסט ייכשל (וזה מה שאנחנו רוצים).
    // בשלב ה-Green: השורה הזו תחזיר את התוכן והטסט ימשיך ל-Assert.
    std::vector<std::byte> restoredContent = storage.restoreFile(userId, filename);

    // 3. Assert
    EXPECT_EQ(restoredContent, originalContent);

    cleanUp("backupsvr");
}

TEST(FileStorageTests, RestoreFile_FileDoesNotExist_ThrowsException) {
    FileStorage storage;
    unsigned int userId = 1234;
    cleanUp("backupsvr");

    storage.saveFile(userId, "setup.dat", { std::byte{0x00} });

    // כאן אנחנו מצפים לשגיאה ספציפית של "File not found"
    // כרגע זה "יעבור" טכנית כי הפונקציה זורקת "Not implemented" שזה גם runtime_error.
    // אבל זה בסדר לשלב הזה.
    EXPECT_THROW(storage.restoreFile(userId, "ghost_file.txt"), std::runtime_error);

    cleanUp("backupsvr");
}


// --- Protocol Parsing Tests (Client -> Server) ---

TEST(ProtocolTests, ParseRequest_SaveFile_ParsesCorrectly) {
    // בניית באפר ידני המדמה את מה שמגיע מהרשת עבור שמירת קובץ (Op 200)
    // Structure: UserID(4), Ver(1), Op(1), NameLen(2), Name(...), Size(4), Payload(...)

    std::vector<std::byte> buffer;

    // User ID: 1234 (0x04D2) -> Little Endian: D2 04 00 00
    buffer.push_back(std::byte{ 0xD2 }); buffer.push_back(std::byte{ 0x04 }); buffer.push_back(std::byte{ 0x00 }); buffer.push_back(std::byte{ 0x00 });

    // Version: 1
    buffer.push_back(std::byte{ 1 });

    // Op: 200 (0xC8)
    buffer.push_back(std::byte{ 200 });

    // Name Len: 4 ("test") -> 04 00
    buffer.push_back(std::byte{ 4 }); buffer.push_back(std::byte{ 0 });

    // Filename: "test"
    buffer.push_back(std::byte{ 't' }); buffer.push_back(std::byte{ 'e' }); buffer.push_back(std::byte{ 's' }); buffer.push_back(std::byte{ 't' });

    // Size: 3 -> 03 00 00 00
    buffer.push_back(std::byte{ 3 }); buffer.push_back(std::byte{ 0 }); buffer.push_back(std::byte{ 0 }); buffer.push_back(std::byte{ 0 });

    // Payload: ABC
    buffer.push_back(std::byte{ 'A' }); buffer.push_back(std::byte{ 'B' }); buffer.push_back(std::byte{ 'C' });

    // Act
    ParsedRequest req = Message::parseRequest(buffer);

    // Assert
    EXPECT_EQ(req.userId, 1234);
    EXPECT_EQ(req.version, 1);
    EXPECT_EQ(req.op, 200);
    EXPECT_EQ(req.filename, "test");
    ASSERT_EQ(req.payload.size(), 3);
    EXPECT_EQ(req.payload[0], std::byte{ 'A' });
}

TEST(ProtocolTests, ParseRequest_ListFiles_ParsesCorrectly) {
    // Op 203 (List) is special: No filename, no size, no payload.
    // Structure: UserID(4), Ver(1), Op(1) -> Total 6 bytes.

    std::vector<std::byte> buffer;
    // User: 1234
    buffer.push_back(std::byte{ 0xD2 }); buffer.push_back(std::byte{ 0x04 }); buffer.push_back(std::byte{ 0x00 }); buffer.push_back(std::byte{ 0x00 });
    // Version: 1
    buffer.push_back(std::byte{ 1 });
    // Op: 203
    buffer.push_back(std::byte{ 203 });

    ParsedRequest req = Message::parseRequest(buffer);

    EXPECT_EQ(req.userId, 1234);
    EXPECT_EQ(req.op, 203);
    EXPECT_TRUE(req.filename.empty()); // Should be empty
    EXPECT_TRUE(req.payload.empty());  // Should be empty
}

// --- Protocol Packing Tests (Server -> Client) ---

TEST(ProtocolTests, PackResponse_SuccessWithPayload_CreatesCorrectBuffer) {
    // Simulating response 210 (File Found)
    // Structure: Ver(1), Status(2), NameLen(2), Name, Size(4), Payload

    uint8_t version = 1;
    uint16_t status = 210;
    std::string filename = "hi";
    std::vector<std::byte> payload = { std::byte{0xFF} };

    std::vector<std::byte> result = Message::packResponse(version, status, filename, payload);

    // Checks
    // Ver
    EXPECT_EQ(result[0], std::byte{ 1 });
    // Status 210 (0x00D2) -> D2 00
    EXPECT_EQ(result[1], std::byte{ 0xD2 });
    EXPECT_EQ(result[2], std::byte{ 0x00 });
    // NameLen 2 -> 02 00
    EXPECT_EQ(result[3], std::byte{ 2 });
    EXPECT_EQ(result[4], std::byte{ 0 });
    // Name "hi"
    EXPECT_EQ(result[5], std::byte{ 'h' });
    EXPECT_EQ(result[6], std::byte{ 'i' });
    // Size 1 -> 01 00 00 00
    EXPECT_EQ(result[7], std::byte{ 1 });
    EXPECT_EQ(result[10], std::byte{ 0 });
    // Payload
    EXPECT_EQ(result[11], std::byte{ 0xFF });
}

TEST(ProtocolTests, PackResponse_ErrorNoFiles_CreatesShortBuffer) {
    // Status 1002 (No files) -> "Only status and version fields"
    // Structure: Ver(1), Status(2) -> Total 3 bytes.

    std::vector<std::byte> result = Message::packResponse(1, 1002);

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{ 1 });
    // Status 1002 (0x03EA) -> EA 03
    EXPECT_EQ(result[1], std::byte{ 0xEA });
    EXPECT_EQ(result[2], std::byte{ 0x03 });
}