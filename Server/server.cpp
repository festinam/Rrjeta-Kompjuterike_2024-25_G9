#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <filesystem>

#pragma warning(disable:4996)
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
namespace fs = std::filesystem;

const int PORT = 5555;
const char* IP_ADDRESS = "192.168.56.1";

int clientcount = 0;

bool initializeWinsock(WSADATA& wsaData) {
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cerr << "Failed to initialize Winsock." << endl;
		return false;
	}
	return true;
}

SOCKET createServerSocket() {
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		cerr << "Error creating server socket." << endl;
	}
	return serverSocket;
}

bool bindServerSocket(SOCKET serverSocket, sockaddr_in& serverAddress) {
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		cerr << "Error binding server socket to address and port." << endl;
		return false;
	}
	return true;
}