#include "pch.h"
#include "BufferUtils.h"

namespace BufferUtils {

    void pushString(std::vector<std::byte>& buffer, const std::string& str) {
        for (char c : str) {
            buffer.push_back(static_cast<std::byte>(c));
        }
    }

    std::string extractString(const std::vector<std::byte>& buffer, size_t& offset, size_t length) {
        if (offset + length > buffer.size()) {
            throw std::runtime_error("Buffer underflow: Not enough bytes to read string");
        }
        std::string str;
        str.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            str += static_cast<char>(buffer[offset++]);
        }
        return str;
    }
}