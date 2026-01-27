#include "pch.h"
#include "Server.h"
#include "ClientHandler.h" // <--- המחלקה החדשה
#include <iostream>
#include <ws2tcpip.h>

Server::Server(int port) : m_port(port), m_serverSocket(INVALID_SOCKET), m_storage() {
    // אתחול Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

Server::~Server() {
    if (m_serverSocket != INVALID_SOCKET) {
        closesocket(m_serverSocket);
    }
    WSACleanup();
}

void Server::run() {
    // 1. יצירת סוקט
    m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_serverSocket == INVALID_SOCKET) {
        throw std::runtime_error("Error creating socket");
    }

    // 2. Bind
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(m_port);

    if (bind(m_serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        throw std::runtime_error("Bind failed");
    }

    // 3. Listen
    if (listen(m_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        throw std::runtime_error("Listen failed");
    }

    std::cout << "Server listening on port " << m_port << "..." << std::endl;

    // 4. Accept Loop
    while (true) {
        SOCKET clientSocket = accept(m_serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // שימוש ב-Lambda (C++11/17) כפי שביקשת, לפתיחת ת'רד
        std::thread t([this, clientSocket]() {
            this->clientHandlerHelper(clientSocket);
            });
        t.detach(); // שחרור הת'רד לרוץ עצמאית
    }
}

void Server::clientHandlerHelper(SOCKET clientSocket) {
    // יצירת המטפל והרצתו
    ClientHandler handler(clientSocket, m_storage);
    handler.run();

    // בסיום השיחה, סוגרים את הסוקט
    closesocket(clientSocket);
}