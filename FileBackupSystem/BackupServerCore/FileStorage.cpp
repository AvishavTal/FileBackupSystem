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
    // 1. Security Validation
    validateFilename(filename);

    // 2. Prepare Directory & Path
    fs::path fullPath = prepareFilePath(userId, filename);

    // 3. Perform the actual write
    writeToFile(fullPath, content);
}

// ==========================================
// Private Helper Functions
// ==========================================

void FileStorage::validateFilename(const std::string& filename) const
{
    if (filename.empty()) {
        throw std::invalid_argument("Security Violation: Filename is empty");
    }

    // Prevent Path Traversal (e.g., "../windows")
    if (filename.find("..") != std::string::npos) {
        throw std::invalid_argument("Security Violation: Filename contains '..'");
    }

    // Prevent absolute paths or directory switching
    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        throw std::invalid_argument("Security Violation: Filename contains path separators");
    }
}

std::filesystem::path FileStorage::prepareFilePath(unsigned int userId, const std::string& filename) const
{
    const std::string baseDir = "backupsvr";

    // Construct user directory path
    fs::path userDir = fs::path(baseDir) / std::to_string(userId);

    // Ensure the directory exists
    try {
        if (!fs::exists(userDir)) {
            fs::create_directories(userDir);
        }
    }
    catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Filesystem error while creating directory: " + std::string(e.what()));
    }

    // Return the full path including filename
    return userDir / filename;
}

void FileStorage::writeToFile(const std::filesystem::path& path, const std::vector<std::byte>& content) const
{
    // Open file in binary mode
    std::ofstream outFile(path, std::ios::binary);

    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " + path.string());
    }

    // Write data: reinterpret_cast is required to treat std::byte* as char* for fstream
    // storage.data() returns a pointer to the underlying array
    outFile.write(reinterpret_cast<const char*>(content.data()), content.size());

    outFile.close();

    // Verify write success
    if (!outFile.good()) {
        throw std::runtime_error("I/O Error occurred while writing to file.");
    }
}