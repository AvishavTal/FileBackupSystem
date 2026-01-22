#include "pch.h"
#include "FileStorage.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <random>

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

std::string FileStorage::listFiles(unsigned int userId) const {
	fs::path userDir = getOrCreateUserDir(userId);
	std::vector<std::string> files = getAllFilenames(userDir);
	std::string listFileName = generateRandomFilename();
	fs::path listFilePath = userDir / listFileName;
	writeLinesToFile(listFilePath, files);
	return listFileName;
}

std::vector<std::byte> FileStorage::restoreFile(unsigned int userId, const std::string& filename) {
	validateFilename(filename);
	fs::path fullFilePath = prepareFilePath(userId, filename);
	if(!fs::exists(fullFilePath)) {
		throw std::runtime_error("File not found: " + filename);
	}
	return readFile(fullFilePath);
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

std::filesystem::path FileStorage::getOrCreateUserDir(unsigned int userId) const
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
	return userDir;
}

std::filesystem::path FileStorage::prepareFilePath(unsigned int userId, const std::string& filename) const
{
	return getOrCreateUserDir(userId) / filename;
}

std::string FileStorage::generateRandomFilename() const
{
	const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::random_device rd;
	std::mt19937 generator(rd());//why mt19937?
	std::uniform_int_distribution<int> distribution(0, static_cast<int>(charset.length() - 1));
	std::string result;
	result.resize(32);
	for(size_t i = 0; i < 32; ++i) {
		result[i] = charset[distribution(generator)];
	}
	return result;
}

std::vector<std::string> FileStorage::getAllFilenames(const std::filesystem::path& directoryPath) const
{
	std::vector<std::string> filenames;
	for (const auto& entry : fs::directory_iterator(directoryPath)) {
		if (entry.is_regular_file()) {
			filenames.push_back(entry.path().filename().string());
		}
	}
	return filenames;
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

void FileStorage::writeLinesToFile(const std::filesystem::path& path, const std::vector<std::string>& lines) const
{
	std::ofstream outFile(path);
	if(!outFile) {
		throw std::runtime_error("Failed to open file for writing: " + path.string());
	}
	for (const auto& line : lines) {
		outFile << line << '\n';
	}
	outFile.close();
	if (!outFile.good()) {
		throw std::runtime_error("I/O Error occurred while writing to file.");
	}
}

std::vector<std::byte> FileStorage::readFile(const std::filesystem::path& path) const
{
	std::ifstream inFile(path, std::ios::binary | std::ios::ate);//open at end to get size
	if (!inFile) {
		throw std::runtime_error("Failed to open file for reading: " + path.string());
	}
	std::streamsize size = inFile.tellg();
	if(size==0) {
		return {};//empty file
	}
	inFile.seekg(0, std::ios::beg);//move to beginning
	std::vector<std::byte> buffer(size);
	if(!inFile.read(reinterpret_cast<char*>(buffer.data()), size)) {
		throw std::runtime_error("I/O Error occurred while reading from file.");
	}
	return buffer;
}