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

bool authenticate(SOCKET clientSocket, bool& isReadOnly) {
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

     
    if (strstr(buffer, "Privilege: User")) {
        isReadOnly = true;
        return true;
    }
    else if (strstr(buffer, "Privilege: Admin")) {
        isReadOnly = false;
        return true;
    }

    cerr << "Authentication failed!" << endl;
    return false;
}


void handleCommands(SOCKET clientSocket, bool isReadOnly) {
    char buffer[1024];

    while (true) {
        if (isReadOnly) {
          cout << "Enter a command ('read <file_path>' or type 'exit' to end): ";
        }
        else {
          cout << "Enter a command ('read <file_path>', 'write <string> <file_path>', 'execute <command>', 'list <directory_path>', or type 'exit' to end): ";
        }
    cin.getline(buffer, sizeof(buffer));
    
        if (strcmp(buffer, "exit") == 0) {
            const char* exitMessage = "Client disconnected.";
            send(clientSocket, exitMessage, strlen(exitMessage), 0);
            break;
        }

        // Check command for read-only users
        if (isReadOnly) {
            if (strncmp(buffer, "read", 4) != 0) {
                cout << "Error: Unauthorized command." << endl;
                continue;
            }
        }

        // Send the command to the server
        send(clientSocket, buffer, strlen(buffer), 0);

        // Receive response from the server
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the buffer
            cout << "Server Response: " << buffer << endl;
        }
        else {
            cout << "Connection to server lost." << endl;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;

    if (!initializeWinsock(wsaData)) {
        return -1;
    }

    SOCKET clientSocket = createClientSocket();
    if (clientSocket == -1) {
        cleanup(clientSocket);
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    serverAddress.sin_port = htons(PORT);

    if (!connectToServer(clientSocket, serverAddress)) {
        cleanup(clientSocket);
        return -1;
    }

    cout << "Connected to the server." << endl;

    bool isReadOnly;
    if (authenticate(clientSocket, isReadOnly)) {
        handleCommands(clientSocket, isReadOnly);
    }

    cleanup(clientSocket);
    return 0;
}
