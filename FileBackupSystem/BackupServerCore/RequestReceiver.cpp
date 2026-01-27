#include "pch.h"
#include "RequestReceiver.h"
#include "BufferUtils.h"
#include <fstream>
#include <stdexcept>
#include <vector>

RequestReceiver::RequestReceiver(SOCKET socket) : m_socket(socket) {}

RequestHeaderFixed RequestReceiver::readFixedHeader() {
    std::vector<std::byte> buffer;
    readExact(buffer, sizeof(RequestHeaderFixed)); // קורא 8 בייטים

    RequestHeaderFixed header;
    size_t offset = 0;

    // שימוש ב-BufferUtils כדי להבטיח קריאה נכונה של Little Endian
    // גם אם השרת רץ על מכונה שהיא Big Endian
    header.userId = BufferUtils::extractInt<uint32_t>(buffer, offset);
    header.version = static_cast<uint8_t>(buffer[offset++]);
    header.op = static_cast<uint8_t>(buffer[offset++]);
    header.nameLen = BufferUtils::extractInt<uint16_t>(buffer, offset);

    return header;
}

std::string RequestReceiver::readFilename(uint16_t length) {
    if (length == 0) return "";

    std::vector<std::byte> buffer;
    readExact(buffer, length);

    // המרה מבייטים לסטרינג
    std::string name;
    name.reserve(length);
    for (auto b : buffer) {
        name += static_cast<char>(b);
    }
    return name;
}

uint32_t RequestReceiver::readPayloadSize() {
    std::vector<std::byte> buffer;
    readExact(buffer, sizeof(uint32_t)); // 4 bytes

    size_t offset = 0;
    return BufferUtils::extractInt<uint32_t>(buffer, offset);
}

std::vector<std::byte> RequestReceiver::readPayloadToBuffer(uint32_t size) {
    std::vector<std::byte> buffer;
    if (size > 0) {
        readExact(buffer, size);
    }
    return buffer;
}

void RequestReceiver::readPayloadToFile(const std::string& filePath, uint32_t size) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    const size_t CHUNK_SIZE = 4096; // 4KB chunks
    std::vector<std::byte> buffer(CHUNK_SIZE);
    uint32_t remaining = size;

    while (remaining > 0) {
        size_t toRead = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

        // קריאה מהסוקט (בצורה ישירה יותר כדי למנוע העתקות מיותרות)
        int bytesReceived = recv(m_socket, reinterpret_cast<char*>(buffer.data()), static_cast<int>(toRead), 0);

        if (bytesReceived <= 0) {
            throw std::runtime_error("Connection closed or error while receiving file payload");
        }

        // כתיבה לדיסק
        outFile.write(reinterpret_cast<char*>(buffer.data()), bytesReceived);
        remaining -= bytesReceived;
    }
}

void RequestReceiver::readExact(std::vector<std::byte>& buffer, size_t size) {
    buffer.resize(size);
    size_t totalRead = 0;

    while (totalRead < size) {
        int bytesRead = recv(m_socket,
            reinterpret_cast<char*>(buffer.data() + totalRead),
            static_cast<int>(size - totalRead),
            0);

        if (bytesRead == 0) {
            throw std::runtime_error("Client disconnected unexpectedly");
        }
        if (bytesRead < 0) {
            throw std::runtime_error("Socket error during read");
        }
        totalRead += bytesRead;
    }
}