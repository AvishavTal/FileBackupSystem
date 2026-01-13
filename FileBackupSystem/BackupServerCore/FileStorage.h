#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>

class FileStorage {
public:
	void saveFile(unsigned int userId, const std::string& filename, const std::vector<std::byte>& content);

	// זו השורה שהייתה חסרה לך ולכן הטסט צעק:
	void deleteFile(unsigned int userId, const std::string& filename);

private:
	void validateFilename(const std::string& filename) const;
	std::filesystem::path prepareFilePath(unsigned int userId, const std::string& filename) const;
	void writeToFile(const std::filesystem::path& path, const std::vector<std::byte>& content) const;
};