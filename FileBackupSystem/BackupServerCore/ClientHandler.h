#pragma once
#include <winsock2.h>
#include "FileStorage.h"
#include "Protocol.h"

class ClientHandler {
public:
    // מקבל את הסוקט ואת ה-Storage (כדי לשמור/לקרוא קבצים)
    ClientHandler(SOCKET clientSocket, FileStorage& storage);

    // הפונקציה הראשית שמריצה את השיחה עם הלקוח
    void run();

private:
    SOCKET m_socket;
    FileStorage& m_storage;

    // פונקציות לוגיות לכל סוג בקשה
    void handleSaveFile(const RequestHeaderFixed& header);
    void handleRestoreFile(const RequestHeaderFixed& header);
    void handleDeleteFile(const RequestHeaderFixed& header);
    void handleListFiles(const RequestHeaderFixed& header);
};
