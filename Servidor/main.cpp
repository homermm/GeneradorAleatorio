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

    //! Funciones solicitadas

    // Función para generar nombres de usuario
    void generateUsername(int length, string &username) {
        if (length < 5 || length > 15) {
            username = "Error: Longitud de nombre de usuario inválida.";
            return;
        }

        const string vowels = "aeiou";
        const string consonants = "bcdfghjklmnpqrstvwxyz";
        int vowel_count = vowels.size();
        int consonant_count = consonants.size();

        srand(time(nullptr));
        bool startWithVowel = rand() % 2;
        username.clear();

        for (int i = 0; i < length; ++i) {
            if (startWithVowel) {
                username += vowels[rand() % vowel_count];
                startWithVowel = false; // Bandera para alternar
            } else {
                username += consonants[rand() % consonant_count];
                startWithVowel = true;
            }
        }
    }

    // Función para generar contraseñas
    void generatePassword(int length, string &password) {
        if (length < 8 || length > 50) {
            password = "Error: Longitud de contraseña inválida.";
            return;
        }

        const string alphanum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        int alphanum_count = alphanum.size();

        srand(time(nullptr));
        password.clear();

        for (int i = 0; i < length; ++i) {
            password += alphanum[rand() % alphanum_count];
        }
    }

//! Funciones primitivas
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
        string response;
        while (true) {
            int valread = recv(client_socket, buffer, 1024, 0);
            if (valread > 0) {
                buffer[valread] = '\0';
                string request(buffer);
                string type;
                int length;
                istringstream iss(request);
                iss >> type >> length;

                if (type == "username") { // Desde el cliente al usar esta opcion se manda el mensaje iniciando con "username" y luego el tamaño
                    generateUsername(length, response);
                } else if (type == "password") {
                    generatePassword(length, response); // La misma logica que en username
                } else {
                    response = "Error: Tipo de solicitud inválido.";
                }

                send(client_socket, response.c_str(), response.length(), 0);
            } else if (valread == 0) {
                cout << "Cliente desconectado" << endl;
                break;
            } else {
                cerr << "Error al recibir mensaje: " << WSAGetLastError() << endl; //! Fixear cierre de cliente
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
