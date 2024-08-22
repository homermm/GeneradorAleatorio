#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

class Server {
private:
    SOCKET server_fd;
    sockaddr_in address;
    int addrlen;
    vector<SOCKET> clients;

    //! CONNECTION
    // Initialize Winsock
    void initializeWinsock() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("Error initializing Winsock");
        }
    }

    // Create the server socket
    void createSocket() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET) {
            throw runtime_error("Socket creation failed: " + to_string(WSAGetLastError()));
        }
    }

    // Configure the server address and port
    void bindSocket(int port) {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        addrlen = sizeof(address);

        if (bind(server_fd, (struct sockaddr *)&address, addrlen) == SOCKET_ERROR) {
            throw runtime_error("Bind failed: " + to_string(WSAGetLastError()));
        }
    }

    // Prepare the socket to accept connections
    void listenForConnections() {
        if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
            throw runtime_error("Listen failed: " + to_string(WSAGetLastError()));
        }
    }

    //! REQUESTED FEATURES
    // Generate a username with the specified length
    void generateUsername(int length, string &username) {
        if (length < 5 || length > 15) {
            username = "Error: Invalid username length.";
            return;
        }

        static const string vowels = "aeiou";
        static const string consonants = "bcdfghjklmnpqrstvwxyz";
        srand(static_cast<unsigned>(time(nullptr)));

        bool startWithVowel = rand() % 2;
        username.clear();

        for (int i = 0; i < length; ++i) {
            if (startWithVowel) {
                username += vowels[rand() % vowels.size()];
                startWithVowel = false; // Flag between vowel and consonant
            } else {
                username += consonants[rand() % consonants.size()];
                startWithVowel = true;
            }
        }
    }

    // Generate a password with the specified length
    void generatePassword(int length, string &password) {
        if (length < 8 || length > 50) {
            password = "Error: Invalid password length.";
            return;
        }

        static const string alphanum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        srand(static_cast<unsigned>(time(nullptr)));

        password.clear();
        for (int i = 0; i < length; ++i) {
            password += alphanum[rand() % alphanum.size()];
        }
    }


    //! COMMUNICATION
    // Handle communication with a client
    void handleClient(SOCKET client_socket) {
        char buffer[1024];
        while (true) {
            int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                string request(buffer);
                string type;
                int length;
                istringstream iss(request);
                iss >> type >> length;

                string response;
                if (type == "username") { // Using this option from the client the message is sent starting with "username" and then the size
                    generateUsername(length, response);
                } else if (type == "password") {
                    generatePassword(length, response); // Uses the same logic as the username
                } else {
                    response = "Error: Invalid request type.";
                }

                send(client_socket, response.c_str(), response.length(), 0);
            } else if (bytesReceived == 0) {
                cout << "Client disconnected" << endl;
                break;
            } else {
                cerr << "Error receiving message: " << WSAGetLastError() << endl;
                break;
            }
        }
        closesocket(client_socket);
    }

    // Broadcast a message to all clients except the sender
    void broadcast(const string &msg, SOCKET sender_socket) {
        for (SOCKET client : clients) {
            if (client != sender_socket) {
                send(client, msg.c_str(), msg.length(), 0);
            }
        }
    }

    // Handle console input and broadcast messages
    void consoleInput() {
        string message;
        while (true) {
            getline(cin, message);
            if (message.empty()) {
                continue;
            }
            broadcast(message, INVALID_SOCKET);
        }
    }

public:
    // Server constructor
    Server(int port) {
        try {
            initializeWinsock();
            createSocket();
            bindSocket(port);
            listenForConnections();
            cout << "Server started and waiting for connections..." << endl;
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    // Start the server
    void start() {
        thread(&Server::consoleInput, this).detach();
        while (true) {
            SOCKET new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket != INVALID_SOCKET) {
                cout << "Client connected" << endl;
                clients.push_back(new_socket);
                thread(&Server::handleClient, this, new_socket).detach();
            }
        }
    }

    // Server destructor
    ~Server() {
        closesocket(server_fd);
        WSACleanup();
    }
};

int main() {
    try {
        Server server(8080);
        server.start();
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
