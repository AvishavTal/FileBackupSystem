#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>

class FileStorage {
public:
	void saveFile(unsigned int userId, const std::string& filename, const std::vector<std::byte>& content);


	void deleteFile(unsigned int userId, const std::string& filename);

	std::string listFiles(unsigned int userId) const;

	/**
	 * Retrieves the content of a specific file.
	 */
	std::vector<std::byte> restoreFile(unsigned int userId, const std::string& filename);
private:
	void validateFilename(const std::string& filename) const;
	std::filesystem::path getOrCreateUserDir(unsigned int userId) const;
	std::filesystem::path prepareFilePath(unsigned int userId, const std::string& filename) const;
	void writeToFile(const std::filesystem::path& path, const std::vector<std::byte>& content) const;
	std::string generateRandomFilename() const;

	void writeLinesToFile(const std::filesystem::path& path, const std::vector<std::string>& lines) const;
	std::vector<std::string> getAllFilenames(const std::filesystem::path& directoryPath) const;
	std::vector<std::byte> readFile(const std::filesystem::path& path) const;
};