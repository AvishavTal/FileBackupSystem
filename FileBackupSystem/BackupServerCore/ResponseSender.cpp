#include "pch.h"
#include "ResponseSender.h"
#include "BufferUtils.h"
#include <fstream>
#include <stdexcept>

ResponseSender::ResponseSender(SOCKET socket) : m_socket(socket) {}

void ResponseSender::sendResponse(uint8_t version, uint16_t status, const std::string& filename) {
    // 1. Send Header
    sendFixedHeader(version, status, static_cast<uint16_t>(filename.length()));

    // 2. Send Filename
    if (!filename.empty()) {
        std::vector<std::byte> nameBuf;
        BufferUtils::pushString(nameBuf, filename);
        sendBuffer(nameBuf);
    }

    // אין payload, אז לא שולחים size ולא שולחים תוכן
}

void ResponseSender::sendListResponse(uint8_t version, uint16_t status, const std::string& fileList) {
    // 1. Send Header (Empty filename usually for list)
    sendFixedHeader(version, status, 0);

    // 2. Send Size
    sendUint32(static_cast<uint32_t>(fileList.size()));

    // 3. Send Content
    std::vector<std::byte> dataBuf;
    BufferUtils::pushString(dataBuf, fileList);
    sendBuffer(dataBuf);
}

void ResponseSender::sendFileResponse(uint8_t version, uint16_t status, const std::string& filename, const std::string& physicalFilePath) {
    std::ifstream inFile(physicalFilePath, std::ios::binary | std::ios::ate); // Open at end to get size
    if (!inFile) {
        // אם לא הצלחנו לפתוח, כנראה צריך לשלוח שגיאה, אבל זה באחריות ה-Logic
        throw std::runtime_error("Cannot open file for sending: " + physicalFilePath);
    }

    uint32_t fileSize = static_cast<uint32_t>(inFile.tellg());
    inFile.seekg(0, std::ios::beg); // חזרה להתחלה

    // 1. Send Header
    sendFixedHeader(version, status, static_cast<uint16_t>(filename.length()));

    // 2. Send Filename
    if (!filename.empty()) {
        std::vector<std::byte> nameBuf;
        BufferUtils::pushString(nameBuf, filename);
        sendBuffer(nameBuf);
    }

    // 3. Send Size
    sendUint32(fileSize);

    // 4. Stream File Content
    const size_t CHUNK_SIZE = 4096;
    char buffer[CHUNK_SIZE];

    while (inFile.read(buffer, CHUNK_SIZE) || inFile.gcount() > 0) {
        int bytesToSend = static_cast<int>(inFile.gcount());
        int sent = send(m_socket, buffer, bytesToSend, 0);
        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("Socket error during file send");
        }
    }
}

void ResponseSender::sendFixedHeader(uint8_t version, uint16_t status, uint16_t nameLen) {
    std::vector<std::byte> buffer;
    buffer.push_back(static_cast<std::byte>(version));
    BufferUtils::pushInt<uint16_t>(buffer, status);
    BufferUtils::pushInt<uint16_t>(buffer, nameLen);
    sendBuffer(buffer);
}

void ResponseSender::sendUint32(uint32_t value) {
    std::vector<std::byte> buffer;
    BufferUtils::pushInt<uint32_t>(buffer, value);
    sendBuffer(buffer);
}

void ResponseSender::sendBuffer(const std::vector<std::byte>& buffer) {
    if (buffer.empty()) return;
    int sent = send(m_socket, reinterpret_cast<const char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
    if (sent == SOCKET_ERROR) {
        throw std::runtime_error("Socket error sending buffer");
    }
}