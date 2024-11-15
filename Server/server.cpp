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

bool startListening(SOCKET serverSocket) {
    if (listen(serverSocket, 5) == -1) {
        cerr << "Error listening on server socket." << endl;
        return false;
    }
    return true;
}

bool authenticate(const char* username, const char* password, bool& readOnly) {
    if (strcmp(username, "Administrator") == 0 && strcmp(password, "A123") == 0) {
        readOnly = false;
        return true;
    }
    else if (strcmp(username, "User") == 0 && strcmp(password, "U123") == 0) {
        readOnly = true;
        return true;
    }
    return false;
}

string executeCommand(const string& command) {
    string result;
    FILE* pipe = _popen(command.c_str(), "r");

    if (!pipe) {
        return "Error executing command.";
    }

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            result += buffer;
        }
    }

    _pclose(pipe);
    return result.empty() ? "Command executed successfully!" : result;
}
void handleClient(int clientSocket, sockaddr_in clientAddress) {
    char buffer[1024];
    int bytesRead;
    
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

    memset(buffer, 0, sizeof(buffer)); 
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); 
    if (bytesRead <= 0) return;
    buffer[bytesRead] = '\0'; 
    string username(buffer);

    memset(buffer, 0, sizeof(buffer));  
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);  
    if (bytesRead <= 0) return;
    buffer[bytesRead] = '\0';  
    string password(buffer);
    }