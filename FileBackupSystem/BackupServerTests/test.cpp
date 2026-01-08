#include "pch.h"
#include <string>

#include "FileStorage.h"

TEST(FileStorageTests, SaveFile_ValidContent_ReturnsTrue) {
	// Arrange
	FileStorage storage;
	std::string filename = "test_file.txt";
	std::string content = "Hello TDD";

	// Act
	bool result = storage.SaveFile(filename, content);

	// Assert
	EXPECT_TRUE(result);
}