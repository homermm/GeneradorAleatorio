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

public:
    Client(const string &ip, int port) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            cerr << "Error al crear el socket: " << WSAGetLastError() << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
            cerr << "Dirección IP no válida" << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            cerr << "Error al conectar con el servidor: " << WSAGetLastError() << endl;
            closesocket(sock);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        cout << "Conectado al servidor" << endl;
    }

    void start() {
        thread(&Client::receiveMessages, this).detach();
        string message;
        while (true) {
            getline(cin, message);
            if (message.empty()) {
                continue;
            }
            int sent = send(sock, message.c_str(), message.length(), 0);
            if (sent == SOCKET_ERROR) {
                cerr << "Error al enviar mensaje: " << WSAGetLastError() << endl;
                break;
            }
        }
        closesocket(sock);
        WSACleanup();
    }

    void receiveMessages() {
        char buffer[1024] = {0};
        while (true) {
            int valread = recv(sock, buffer, 1024, 0);
            if (valread > 0) {
                buffer[valread] = '\0';
                cout << "Servidor: " << buffer << endl;
            } else if (valread == 0) {
                cout << "Conexión cerrada por el servidor" << endl;
                break;
            } else {
                cerr << "Error al recibir mensaje: " << WSAGetLastError() << endl;
                break;
            }
        }
    }

    ~Client() {
        closesocket(sock);
        WSACleanup();
    }
};

int main() {
    Client client("127.0.0.1", 8080);
    client.start();
    return 0;
}
