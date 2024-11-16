#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

#pragma warning(disable:4996)
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
namespace fs = std::filesystem;

const int PORT = 5555;
const char* IP_ADDRESS = "10.1.3.90"; // Server IP address

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

bool authenticate(const char* username, const char* password, bool& isReadOnly) {
    if (strcmp(username, "Administrator") == 0 && strcmp(password, "A123") == 0) {
        isReadOnly = false;
        return true;
    }
    else if (strcmp(username, "User") == 0 && strcmp(password, "U123") == 0) {
        isReadOnly = true;
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

    // Retrieve and format client IP address
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

    // Receive username
    memset(buffer, 0, sizeof(buffer));
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return;
    buffer[bytesRead] = '\0';
    string username(buffer);

    // Receive password
    memset(buffer, 0, sizeof(buffer));
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return;
    buffer[bytesRead] = '\0';
    string password(buffer);

    bool isReadOnly;
    if (authenticate(username.c_str(), password.c_str(), isReadOnly)) {
        string privilege = isReadOnly ? "Privilege: User" : "Privilege: Admin";
        string authMessage = "Authentication successful. Access granted. " + privilege;
        send(clientSocket, authMessage.c_str(), authMessage.length(), 0);
        clientcount++;

        // Check client limit
        if (clientcount > 4) {
            const char* noSpaceMessage = "Limit of clients reached!";
            send(clientSocket, noSpaceMessage, strlen(noSpaceMessage), 0);
            closesocket(clientSocket);
            clientcount--;
            return;
        }

        cout << (isReadOnly ? "User" : "Admin") << " client connected with IP: " << clientIP << endl;

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) break;

            buffer[bytesRead] = '\0';
            string command(buffer);
            istringstream iss(command);
            string action, content, fileName;
            iss >> action;

            if (action == "list") {
                string directory = ".";  // Default to current directory

                // Check if user inputted any additional arguments after "list"
                iss >> content;

                // If the user typed something after "list", ignore it (or use it as an error message)
                if (!content.empty() && content != "files") {
                    string errorMessage = "Error: Invalid command. Use 'list files'.";
                    send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
                    continue;
                }

                string result;
                try {
                    for (const auto& entry : fs::directory_iterator(directory)) {
                        result += entry.path().string() + "\n";
                    }
                    if (result.empty()) {
                        result = "Directory is empty.";
                    }
                }
                catch (const fs::filesystem_error& e) {
                    result = "Error: Unable to access directory.";
                }

                send(clientSocket, result.c_str(), result.length(), 0);
            }
            else if (action == "read") {
                iss >> fileName;

                ifstream file(fileName, ios::binary);
                if (file) {
                    string fileContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                    file.close();

                    send(clientSocket, fileContent.c_str(), fileContent.length(), 0);
                }
                else {
                    const char* errorMessage = "Error: File not found.";
                    send(clientSocket, errorMessage, strlen(errorMessage), 0);
                }
            }
            else if (action == "write" && !isReadOnly) {
                iss >> content >> fileName;

                ofstream file(fileName, ios::binary);
                if (file) {
                    file << content;
                    file.close();

                    const char* successMessage = "File was written successfully.";
                    send(clientSocket, successMessage, strlen(successMessage), 0);
                }
                else {
                    const char* errorMessage = "Error: Unable to write to file.";
                    send(clientSocket, errorMessage, strlen(errorMessage), 0);
                }
            }
            else if (action == "execute" && !isReadOnly) {
                getline(iss, content);
                string result = executeCommand(content);
                send(clientSocket, result.c_str(), result.length(), 0);
            }
            else {
                const char* errorMessage = "Error: Unauthorized or unsupported command.";
                send(clientSocket, errorMessage, strlen(errorMessage), 0);
            }
        }
    }
    else {
        const char* authFailureMessage = "Authentication failed. Access denied.";
        send(clientSocket, authFailureMessage, strlen(authFailureMessage), 0);
    }

    cout << (isReadOnly ? "User" : "Admin") << " client disconnected!" << endl;
    closesocket(clientSocket);
    clientcount--;
}

int main() {
    WSADATA wsaData;
    if (!initializeWinsock(wsaData)) {
        return -1;
    }

    SOCKET serverSocket = createServerSocket();
    if (serverSocket == -1) {
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    serverAddress.sin_port = htons(PORT);

    if (!bindServerSocket(serverSocket, serverAddress)) {
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    if (!startListening(serverSocket)) {
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    cout << "Server is listening on port " << PORT << "..." << endl;

    while (true) {
        sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            cerr << "Error accepting client connection." << endl;
            continue;
        }

        thread clientThread(handleClient, clientSocket, clientAddress);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
