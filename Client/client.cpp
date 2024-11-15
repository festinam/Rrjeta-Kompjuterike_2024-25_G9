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


void handleCommands(SOCKET clientSocket) {
    char buffer[1024];

    while (true) {
        cout << "Enter a command ('read <file_path>', 'write <string> <file_path>', 'execute <command>', 'list <directory_path>', or type 'exit' to end): ";
        cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "exit") == 0) {
            const char* exitMessage = "Client disconnected.";
            send(clientSocket, exitMessage, strlen(exitMessage), 0);
            break;
        }

        // Send the command to the server
        send(clientSocket, buffer, strlen(buffer), 0);

        // Receive response from the server
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the buffer
            cout << "Server Response: " << buffer << endl;

            // Check if the server has reached the client limit
            if (strcmp(buffer, "Limit of clients reached!") == 0) {
                cout << "Disconnecting the client..." << endl;
                break;
            }
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

    try {
        authenticate(clientSocket);
        handleCommands(clientSocket);
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
    }

    cleanup(clientSocket);
    return 0;
}
