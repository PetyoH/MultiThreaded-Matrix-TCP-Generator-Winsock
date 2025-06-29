#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>


#pragma comment(lib, "Ws2_32.lib")

#define SERVERPORT "4950"

bool initialize_winsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return false;
    }
    return true;
}

void printTable(const std::vector<std::vector<int>>& table) {
    std::cout << "Recieved Table:\n";
    for (const auto& row : table) {
        for (int val : row) {
            std::cout << val << "\t";
        }
        std::cout << "\n";
    }
}

int main() {
    if (!initialize_winsock()) {
        return 1;
    }

    const char* server_address = "127.0.0.1";

    int sockfd;
    struct addrinfo hints, * ai, * p;

    int listener;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(server_address, SERVERPORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    p = ai;

    for (p = ai; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            continue; 
        }

        if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sockfd);
            std::cerr << "Connect failed: " << WSAGetLastError() << "\n";
            continue; 
        }

        break;
    }

    if (p == nullptr) {
        std::cerr << "Client: failed to connect to server\n";
        freeaddrinfo(ai);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(ai);

    std::string data_to_send = R"({"threads": 4, "size": [10, 10], "message": "Hello, server!"})";

    int threads, rows, cols;
    std::cout << "Enter the number of threads: ";
    std::cin >> threads;
    std::cout << "Enter the number of rows: ";
    std::cin >> rows;
    std::cout << "Enter the number of columns: ";
    std::cin >> cols;


    int send_result;

    // Send threads value
    send_result = send(sockfd, reinterpret_cast<char*>(&threads), sizeof(threads), 0);
    if (send_result == SOCKET_ERROR) {
        std::cerr << "Send failed for threads: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Send rows value
    send_result = send(sockfd, reinterpret_cast<char*>(&rows), sizeof(rows), 0);
    if (send_result == SOCKET_ERROR) {
        std::cerr << "Send failed for rows: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Send cols value
    send_result = send(sockfd, reinterpret_cast<char*>(&cols), sizeof(cols), 0);
    if (send_result == SOCKET_ERROR) {
        std::cerr << "Send failed for columns: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }


    std::vector<std::vector<int>> table(rows, std::vector<int>(cols));
   
    for (int i = 0; i < rows; ++i) {
        recv(sockfd, reinterpret_cast<char*>(table[i].data()), cols * sizeof(int), 0);
    }

    // Print the received table
    printTable(table);

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
