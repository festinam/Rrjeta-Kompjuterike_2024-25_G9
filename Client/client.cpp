#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma warning(disable:4996)
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int PORT = 5555;
const char* IP_ADDRESS = "192.168.56.1"; // Server IP address

void cleanup(SOCKET& clientSocket) {
    closesocket(clientSocket);
    WSACleanup();
}

bool initializeWinsock(WSADATA& wsaData) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock." << endl;
        return false;
    }
    return true;
}

SOCKET createClientSocket() {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating client socket." << endl;
    }
    return clientSocket;
}

bool connectToServer(SOCKET clientSocket, sockaddr_in& serverAddress) {
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "Error connecting to server." << endl;
        return false;
    }
    return true;
}

void authenticate(SOCKET clientSocket) {
    char buffer[1024];

    // Prompt for username
    cout << "Enter username: ";
    cin.getline(buffer, sizeof(buffer));
    send(clientSocket, buffer, strlen(buffer), 0);  // Send username

    // Prompt for password
    cout << "Enter password: ";
    cin.getline(buffer, sizeof(buffer));
    send(clientSocket, buffer, strlen(buffer), 0);  // Send password

    // Receive authentication response
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "Server response: " << buffer << endl;

    if (strcmp(buffer, "Authentication successful. Access granted.") != 0) {
        cerr << "Authentication failed!" << endl;
        throw runtime_error("Authentication failed");
    }
}