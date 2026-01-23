#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace BufferUtils {

    // --- Templates ---

    template <typename T>
    void pushInt(std::vector<std::byte>& buffer, T value) {
        for (size_t i = 0; i < sizeof(T); ++i) {
            buffer.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
        }
    }

    template <typename T>
    T extractInt(const std::vector<std::byte>& buffer, size_t& offset) {
        if (offset + sizeof(T) > buffer.size()) {
            throw std::runtime_error("Buffer underflow: Not enough bytes to read integer");
        }
        T value = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            value |= (static_cast<T>(buffer[offset + i]) << (i * 8));
        }
        offset += sizeof(T);
        return value;
    }

    // --- Regular Functions ---

    void pushString(std::vector<std::byte>& buffer, const std::string& str);

    std::string extractString(const std::vector<std::byte>& buffer, size_t& offset, size_t length);
}