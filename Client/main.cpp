#include <iostream>
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 27015
#define BUFFER_SIZE 1024

void startClient() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    SetConsoleOutputCP(CP_UTF8);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Не вдалося ініціалізувати Winsock. Код помилки: " << WSAGetLastError() << std::endl;
        return;
    }

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Не вдалося створити сокет. Код помилки: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, SERVER_IP, &serverAddr.sin_addr.s_addr);
    serverAddr.sin_port = htons(SERVER_PORT);


    std::string request = "FILE_REQUEST";
    if (sendto(clientSocket, request.c_str(), request.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Не вдалося відправити запит на сервер. Код помилки: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    char fileName[BUFFER_SIZE];
    int serverAddrSize = sizeof(serverAddr);
    int fileNameSize = recvfrom(clientSocket, fileName, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrSize);
    if (fileNameSize <= 0) {
        std::cerr << "Не вдалося отримати назву файлу від сервера." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }
    fileName[fileNameSize] = '\0';

    std::streamsize fileSize;
    int bytesReceived = recvfrom(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0, (sockaddr*)&serverAddr, &serverAddrSize);
    if (bytesReceived != sizeof(fileSize)) {
        std::cerr << "Не вдалося отримати розмір файлу від сервера." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "Отримано назву файлу: " << fileName << ", Розмір файлу: " << fileSize << " байтів" << std::endl;

    std::ofstream outputFile(fileName, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Не вдалося відкрити файл для запису." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    char buffer[BUFFER_SIZE];
    std::streamsize totalBytesReceived = 0;
    while (totalBytesReceived < fileSize && (bytesReceived = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrSize)) > 0) {
        outputFile.write(buffer, bytesReceived);
        totalBytesReceived += bytesReceived;
    }

    std::cout << "Файл отримано і збережено як " << fileName << std::endl;
    outputFile.close();

    closesocket(clientSocket);
    WSACleanup();
}

int main() {
    startClient();
    return 0;
}
