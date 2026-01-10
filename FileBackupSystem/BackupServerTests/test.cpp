#include "pch.h"
#include <string>
#include <fstream>
#include <filesystem>
#include "FileStorage.h"

void cleanUp(const std::string& path) {
	if (std::filesystem::exists(path)) {
		std::filesystem::remove_all(path);
	}
}

TEST(FileStorageTests, SaveFile_NewUserAndFile_CreatesDirectoryAndFlie) {
	// Arrange
	FileStorage storage;
	unsigned int userId = 1234;
	std::string filename = "test_file.txt";
	std::string content = "This is a binary content representation";

	std::string expectedDir = "backupsvr\\" + std::to_string(userId);
	std::string expectedPath = expectedDir + "\\" + filename;
	cleanUp("backupsvr");

	// Act
	try {
		storage.saveFile(userId, filename, content);
	} catch (...) {
		FAIL() << "Exception thrown during saveFile";
	}

	// Assert
	EXPECT_TRUE(std::filesystem::exists(expectedDir)) << "User directory was not created.";
	EXPECT_TRUE(std::filesystem::exists(expectedPath)) << "File was not created.";

	std::ifstream inFile(expectedPath, std::ios::binary);
	std::stringstream buffer;
	buffer << inFile.rdbuf();
	EXPECT_EQ(buffer.str(), content) << "File content does not match.";
	cleanUp("backupsvr");
}