#include "pch.h"
#include "FileStorage.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace fs = std::filesystem;

// ==========================================
// Public Interface
// ==========================================

void FileStorage::saveFile(unsigned int userId, const std::string& filename, const std::vector<std::byte>& content)
{
	validateFilename(filename);
	fs::path fullPath = prepareFilePath(userId, filename);
	writeToFile(fullPath, content);
}

void FileStorage::deleteFile(unsigned int userId, const std::string& filename)
{
	validateFilename(filename);
	fs::path fullFilePath = prepareFilePath(userId, filename);

	if (!fs::exists(fullFilePath)) {
		throw std::runtime_error("File not found: " + filename);
	}

	if (!fs::remove(fullFilePath)) {
		throw std::runtime_error("Failed to delete file: " + filename);
	}
}

// ==========================================
// Private Helper Functions
// ==========================================

void FileStorage::validateFilename(const std::string& filename) const
{
	if (filename.empty()) {
		throw std::invalid_argument("Security Violation: Filename is empty");
	}
	if (filename.find("..") != std::string::npos) {
		throw std::invalid_argument("Security Violation: Filename contains '..'");
	}
	if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
		throw std::invalid_argument("Security Violation: Filename contains path separators");
	}
}

std::filesystem::path FileStorage::prepareFilePath(unsigned int userId, const std::string& filename) const
{
	const std::string baseDir = "backupsvr";
	fs::path userDir = fs::path(baseDir) / std::to_string(userId);

	try {
		if (!fs::exists(userDir)) {
			fs::create_directories(userDir);
		}
	}
	catch (const fs::filesystem_error& e) {
		throw std::runtime_error("Filesystem error: " + std::string(e.what()));
	}
	return userDir / filename;
}

void FileStorage::writeToFile(const std::filesystem::path& path, const std::vector<std::byte>& content) const
{
	std::ofstream outFile(path, std::ios::binary);
	if (!outFile) {
		throw std::runtime_error("Failed to open file for writing: " + path.string());
	}
	outFile.write(reinterpret_cast<const char*>(content.data()), content.size());
	outFile.close();

	if (!outFile.good()) {
		throw std::runtime_error("I/O Error occurred while writing to file.");
	}
}