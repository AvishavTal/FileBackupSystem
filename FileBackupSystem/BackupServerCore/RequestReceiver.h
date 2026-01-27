#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include <cstddef> // std::byte
#include "Protocol.h"

class RequestReceiver {
public:
    explicit RequestReceiver(SOCKET socket);

    // שלב 1: קורא את 8 הבייטים הראשונים ומפענח אותם לסטראקט
    RequestHeaderFixed readFixedHeader();

    // שלב 2: קורא את שם הקובץ לפי האורך שקיבלנו
    std::string readFilename(uint16_t length);

    // שלב 3 (אופציונלי): קורא את גודל התוכן (4 בייטים)
    uint32_t readPayloadSize();

    // שלב 4א: קורא תוכן קטן לתוך זיכרון (למשל עבור טסטים או הודעות קטנות)
    std::vector<std::byte> readPayloadToBuffer(uint32_t size);

    // שלב 4ב: מזרים תוכן ישירות לקובץ (עבור קבצים גדולים)
    void readPayloadToFile(const std::string& filePath, uint32_t size);

private:
    SOCKET m_socket;

    // פונקציית עזר שקוראת N בייטים לתוך באפר (חוסמת עד שהכל מגיע)
    void readExact(std::vector<std::byte>& buffer, size_t size);
};