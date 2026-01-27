#include "pch.h"
#include "ClientHandler.h"
#include "RequestReceiver.h"
#include "ResponseSender.h"
#include <iostream>
#include <fstream> // חובה בשביל Streaming

ClientHandler::ClientHandler(SOCKET clientSocket, FileStorage& storage)
    : m_socket(clientSocket), m_storage(storage) {
}

void ClientHandler::run() {
    try {
        RequestReceiver receiver(m_socket);
        RequestHeaderFixed header = receiver.readFixedHeader();

        // בדיקות בסיסיות שאפשר לעשות ברמת ה-Handler
        if (header.version != 1) {
            // אפשר להוסיף טיפול בשגיאת גרסה
        }

        switch (header.op) {
        case OP_SAVE_FILE:    handleSaveFile(header); break;
        case OP_RESTORE_FILE: handleRestoreFile(header); break;
        case OP_DELETE_FILE:  handleDeleteFile(header); break;
        case OP_LIST_FILES:   handleListFiles(header); break;
        default:
            std::cerr << "Unknown opcode: " << (int)header.op << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
        try {
            ResponseSender sender(m_socket);
            sender.sendResponse(1, STATUS_ERROR_GENERAL);
        }
        catch (...) {}
    }
}

void ClientHandler::handleSaveFile(const RequestHeaderFixed& header) {
    RequestReceiver receiver(m_socket);
    ResponseSender sender(m_socket);

    // 1. קריאת מטא-דאטה
    std::string filename = receiver.readFilename(header.nameLen);
    uint32_t payloadSize = receiver.readPayloadSize();

    // 2. קבלת נתיב מה-Storage (בטוח ומנוהל)
    // הערה: הפונקציה getAbsolutePath לא יוצרת תיקייה, אבל prepareFilePath שבתוכה כן אמורה (או getOrCreateUserDir)
    // הוספנו את getAbsolutePath ל-FileStorage כפי שסיכמנו
    std::string fullPath = m_storage.getAbsolutePath(header.userId, filename);

    // 3. הזרמה לדיסק (קריאה מהסוקט -> כתיבה לקובץ בבלוקים)
    // זה עוקף את הבעיה של saveFile שמקבלת וקטור
    receiver.readPayloadToFile(fullPath, payloadSize);

    // 4. שליחת אישור
    sender.sendResponse(header.version, STATUS_SUCCESS_FILE_FOUND, filename);
}

void ClientHandler::handleRestoreFile(const RequestHeaderFixed& header) {
    RequestReceiver receiver(m_socket);
    ResponseSender sender(m_socket);

    std::string filename = receiver.readFilename(header.nameLen);

    // בדיקה דרך ה-Storage אם הקובץ קיים (נסיון לפתוח אותו)
    // מכיוון שאין פונקציה exists(userId, file) ב-Public, נשתמש בנתיב:
    std::string fullPath = m_storage.getAbsolutePath(header.userId, filename);

    // בדיקה פיזית (כי ה-Storage מנהל את הנתיבים)
    std::ifstream f(fullPath);
    if (f.good()) {
        f.close(); // נסגור, ה-Sender יפתח מחדש
        // הזרמה מהדיסק לסוקט
        sender.sendFileResponse(header.version, STATUS_SUCCESS_FILE_FOUND, filename, fullPath);
    }
    else {
        sender.sendResponse(header.version, STATUS_ERROR_FILE_NOT_FOUND, filename);
    }
}

void ClientHandler::handleDeleteFile(const RequestHeaderFixed& header) {
    RequestReceiver receiver(m_socket);
    ResponseSender sender(m_socket);

    std::string filename = receiver.readFilename(header.nameLen);

    try {
        // שימוש בפונקציה המקורית של ה-Storage
        m_storage.deleteFile(header.userId, filename);
        sender.sendResponse(header.version, STATUS_SUCCESS_FILE_FOUND, filename);
    }
    catch (const std::exception&) {
        sender.sendResponse(header.version, STATUS_ERROR_FILE_NOT_FOUND, filename);
    }
}

void ClientHandler::handleListFiles(const RequestHeaderFixed& header) {
    ResponseSender sender(m_socket);

    try {
        // ה-Storage יוצר קובץ זמני עם הרשימה ומחזיר את שמו
        std::string listFileName = m_storage.listFiles(header.userId);

        // אנחנו צריכים לקרוא את התוכן של הקובץ הזה ולשלוח אותו
        // ה-Storage שומר אותו באותה תיקייה של המשתמש
        std::string listFullPath = m_storage.getAbsolutePath(header.userId, listFileName);

        // קריאת הקובץ לזיכרון (זה קובץ טקסט קטן, אין בעיית זיכרון)
        std::ifstream inFile(listFullPath);
        std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        // מחיקת הקובץ הזמני (ניקיון)
        std::filesystem::remove(listFullPath);

        sender.sendListResponse(header.version, STATUS_SUCCESS_LIST, content);
    }
    catch (...) {
        sender.sendResponse(header.version, STATUS_ERROR_NO_FILES);
    }
}