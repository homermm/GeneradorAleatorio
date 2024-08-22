#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

class Client {
private:
    SOCKET sock;
    sockaddr_in serv_addr;
    bool running;

    // Initialize Winsock
    void initializeWinsock() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("Error initializing Winsock");
        }
    }

    // Create the socket
    void createSocket() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            throw runtime_error("Socket creation failed: " + to_string(WSAGetLastError()));
        }
    }

    // Configure the server address
    void configureAddress(const string &ip, int port) {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
            throw invalid_argument("Invalid IP address");
        }
    }

    // Connect the socket to the server
    void connectToServer() {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            throw runtime_error("Connection failed: " + to_string(WSAGetLastError()));
        }
    }

    // Handle receiving messages from the server
    void receiveMessages() {
        char buffer[1024];
        while (running) {
            int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                cout << "Server: " << buffer << endl;
            } else if (bytesReceived == 0) {
                cout << "Connection closed by the server" << endl;
                running = false;
            } else {
                cerr << "Error receiving message: " << WSAGetLastError() << endl;
                running = false;
            }
        }
    }

public:
    // Client constructor
    Client(const string &ip, int port) : running(true) {
        try {
            initializeWinsock();
            createSocket();
            configureAddress(ip, port);
            connectToServer();
            cout << "Connected to the server" << endl;
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    // Start the client
    void start() {
        thread receiveThread(&Client::receiveMessages, this);
        receiveThread.detach();

        string message;
        int option;

        while (running) { // Menu
            cout << "\nSelect an option:" << endl;
            cout << "1. Generate username" << endl;
            cout << "2. Generate password" << endl;
            cout << "0. Exit\n" << endl;

            if (!(cin >> option)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number." << endl;
                continue;
            }

            cin.ignore(); // Clear input buffer

            if (option == 0) {
                break;
            }

            int length;
            switch (option) {
                case 1:
                    cout << "Enter the username length (5-15): ";
                    if (!(cin >> length) || length < 5 || length > 15) {
                        cout << "Invalid length" << endl;
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }
                    message = "username " + to_string(length);
                    break;

                case 2:
                    cout << "Enter the password length (8-50): ";
                    if (!(cin >> length) || length < 8 || length > 50) {
                        cout << "Invalid length" << endl;
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }
                    message = "password " + to_string(length);
                    break;

                default:
                    cout << "Invalid option" << endl;
                    continue;
            }

            if (send(sock, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                cerr << "Error sending message: " << WSAGetLastError() << endl;
                running = false;
            }
        }

        closesocket(sock);
        WSACleanup();
    }

    // Client destructor
    ~Client() {
        if (running) {
            closesocket(sock);
            WSACleanup();
        }
    }
};

int main() {
    try {
        Client client("127.0.0.1", 8080);
        client.start();
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
