#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>

class FileStorage {
public:
    /**
     * Saves binary content to a file associated with a specific user.
     * Orchestrates the validation, directory preparation, and writing.
     */
    void saveFile(unsigned int userId, const std::string& filename, const std::vector<std::byte>& content);

    // Other public methods (delete, restore) will be added later...

private:
    // --- Private Helper Functions ---

    /**
     * 1. Security Checks
     * Validates that the filename is safe (no path traversal, no illegal chars).
     * @throws std::invalid_argument If the filename is unsafe.
     */
    void validateFilename(const std::string& filename) const;

    /**
     * 2. Directory Structure
     * Calculates the full path for a file and ensures the user's directory exists.
     * @return The absolute/relative path to the target file.
     */
    std::filesystem::path prepareFilePath(unsigned int userId, const std::string& filename) const;

    /**
     * 3. File Writing
     * Writes the binary data to the disk at the specified path.
     * @throws std::runtime_error If writing fails.
     */
    void writeToFile(const std::filesystem::path& path, const std::vector<std::byte>& content) const;
};