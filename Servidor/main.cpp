#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

class Server {
private:
    SOCKET server_fd;
    sockaddr_in address;
    int addrlen;
    vector<SOCKET> clients;

public:
    Server(int port) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET) {
            cerr << "Error al crear el socket: " << WSAGetLastError() << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        addrlen = sizeof(address);

        if (bind(server_fd, (struct sockaddr *)&address, addrlen) == SOCKET_ERROR) {
            cerr << "Error en bind: " << WSAGetLastError() << endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) == SOCKET_ERROR) {
            cerr << "Error en listen: " << WSAGetLastError() << endl;
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    void start() {
        thread(&Server::consoleInput, this).detach();
        cout << "Servidor iniciado y esperando conexiones..." << endl;
        while (true) {
            SOCKET new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket != INVALID_SOCKET) {
                cout << "Cliente conectado" << endl;
                clients.push_back(new_socket);
                thread(&Server::handleClient, this, new_socket).detach();
            }
        }
    }

    void handleClient(SOCKET client_socket) {
        char buffer[1024] = {0};
        while (true) {
            int valread = recv(client_socket, buffer, 1024, 0);
            if (valread > 0) {
                buffer[valread] = '\0';
                cout << "Cliente: " << buffer << endl;
                broadcast(buffer, client_socket);
            } else if (valread == 0) {
                cout << "Cliente desconectado" << endl;
                break;
            } else {
                cerr << "Error al recibir mensaje: " << WSAGetLastError() << endl;
                break;
            }
        }
        closesocket(client_socket);
    }

    void broadcast(const string &msg, SOCKET sender_socket) {
        for (SOCKET client : clients) {
            if (client != sender_socket) {
                send(client, msg.c_str(), msg.length(), 0);
            }
        }
    }

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

    ~Server() {
        closesocket(server_fd);
        WSACleanup();
    }
};

int main() {
    Server server(8080);
    server.start();
    return 0;
}
