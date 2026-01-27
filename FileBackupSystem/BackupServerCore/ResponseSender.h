#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include <cstddef>
#include "Protocol.h"

class ResponseSender {
public:
    explicit ResponseSender(SOCKET socket);

    // שליחת תשובה ללא קובץ תוכן (למשל: File Not Found, או הצלחה בשמירה)
    void sendResponse(uint8_t version, uint16_t status, const std::string& filename = "");

    // שליחת תשובה שמכילה רשימת קבצים (תוכן בזיכרון)
    void sendListResponse(uint8_t version, uint16_t status, const std::string& fileList);

    // שליחת תשובה שמכילה קובץ פיזי (Restore File - Streaming)
    void sendFileResponse(uint8_t version, uint16_t status, const std::string& filename, const std::string& physicalFilePath);

private:
    SOCKET m_socket;

    void sendFixedHeader(uint8_t version, uint16_t status, uint16_t nameLen);
    void sendBuffer(const std::vector<std::byte>& buffer);
    void sendUint32(uint32_t value); // שליחת גודל ה-Payload
};