#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <thread>
#include <random>
#define PORT "4950"
#pragma comment(lib, "Ws2_32.lib")

int getRandomNumber(int lower, int upper) {
	static std::random_device rd; 
	static std::mt19937 gen(rd()); 

	std::uniform_int_distribution<> dist(lower, upper);

	return dist(gen);
}

void fillTable(std::vector<std::vector<int>>& table,
	int startRow, int endRow, int cols) {
	for (int i = startRow; i < endRow; ++i) {
		for (int j = 0; j < cols; ++j) {
			table[i][j] = getRandomNumber(1,100);
		}
	}
}

void printTable(const std::vector<std::vector<int>>& matrix) {
	std::cout << "Generated Table:\n";
	for (const auto& row : matrix) {
		for (int val : row) {
			std::cout << val << "\t";
		}
		std::cout << "\n";
	}
}
int main() {
	WSADATA wsaData;
	WORD DllVersion = 2 | 2 << 8;
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		fprintf(stderr, "WSAStartup()\n");
		ExitProcess(EXIT_FAILURE);
	}

	struct addrinfo hints, * ai, * p;

	int listener;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	p = ai;

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener == -1) {
			perror("socket");
			std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
			continue;
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
			std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
			continue;
		}

		break; 
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		exit(2);
	}

	freeaddrinfo(ai);
	p = NULL;

	if (listen(listener, SOMAXCONN) == -1) {
		std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
		exit(1);
	}

	std::cout << "Server is listening on port 4950...\n";

	SOCKET client_socket = accept(listener, nullptr, nullptr);
	if (client_socket == INVALID_SOCKET) {
		std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
		closesocket(listener);
		WSACleanup();
		return 1;
	}

	// Receive threads number
	int numThreads;
	int bytes_received = recv(client_socket, reinterpret_cast<char*>(&numThreads), sizeof(numThreads), 0);
	if (bytes_received == SOCKET_ERROR) {
		std::cerr << "Receive failed for threads: " << WSAGetLastError() << "\n";
		closesocket(client_socket);
		closesocket(listener);
		return 1;
	}

	// Receive rows value
	int rows;
	bytes_received = recv(client_socket, reinterpret_cast<char*>(&rows), sizeof(rows), 0);
	if (bytes_received == SOCKET_ERROR) {
		std::cerr << "Receive failed for rows: " << WSAGetLastError() << "\n";
		closesocket(client_socket);
		closesocket(listener);
		return 1;
	}

	// Receive cols value
	int cols;
	bytes_received = recv(client_socket, reinterpret_cast<char*>(&cols), sizeof(cols), 0);
	if (bytes_received == SOCKET_ERROR) {
		std::cerr << "Receive failed for columns: " << WSAGetLastError() << "\n";
		closesocket(client_socket);
		closesocket(listener);
		return 1;
	}

	std::cout << "Received values: Threads = " << numThreads << ", Rows = " << rows << ", Columns = " << cols << "\n";

	std::vector<std::vector<int>> table(rows, std::vector<int>(cols));

	std::vector<std::thread> threads;
	int effectiveThreads = (numThreads < rows) ? numThreads : rows;
	int rowsPerThread = rows / effectiveThreads;
	int remainingRows = rows % effectiveThreads;

	int startRow = 0;
	for (int t = 0; t < effectiveThreads; ++t) {
		int endRow = startRow + rowsPerThread + (t < remainingRows ? 1 : 0); 
		std::thread th(fillTable, std::ref(table), startRow, endRow, cols);
		threads.push_back(std::move(th)); 
		startRow = endRow;
	}

	for (auto& thread : threads) {
		thread.join();		
	}

	// Send the table data row by row 
	for (int i = 0; i < rows; ++i) {
		send(client_socket, reinterpret_cast<char*>(table[i].data()), cols * sizeof(int), 0);
	}

	// Print the generated table
	printTable(table);

	closesocket(client_socket);
	closesocket(listener);
	WSACleanup();
	return 0;
}
