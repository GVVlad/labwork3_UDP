#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 27015
#define BUFFER_SIZE 1024

void startServer() {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SetConsoleOutputCP(CP_UTF8);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Не вдалося ініціалізувати Winsock. Код помилки: " << WSAGetLastError() << std::endl;
        return;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Не вдалося створити сокет. Код помилки: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Не вдалося виконати прив'язку. Код помилки: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Сервер запущений на порту " << SERVER_PORT << std::endl;

    char buffer[BUFFER_SIZE];
    int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Помилка при отриманні даних. Код помилки: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::string fileName = "Lab11Meregi.pdf";
    // std::string fileName = "file.txt";
    // std::string fileName = "file.docx";

    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Не вдалося відкрити файл." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    sendto(serverSocket, fileName.c_str(), fileName.size(), 0, (sockaddr*)&clientAddr, clientAddrSize);

    sendto(serverSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0, (sockaddr*)&clientAddr, clientAddrSize);

    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int bytesSent = sendto(serverSocket, buffer, file.gcount(), 0, (sockaddr*)&clientAddr, clientAddrSize);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Помилка під час відправки даних. Код помилки: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    std::cout << "Файл відправлено." << std::endl;

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
