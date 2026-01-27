
#pragma once
#include <winsock2.h>
#include <thread>
#include "FileStorage.h"
// #include "Message.h" <--- מחקנו את זה!

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    Server(int port = 1234);
    ~Server();
    void run();

private:
    int m_port;
    SOCKET m_serverSocket;
    FileStorage m_storage;

    // הפונקציה שתרוץ ב-Thread
    void clientHandlerHelper(SOCKET clientSocket);
};