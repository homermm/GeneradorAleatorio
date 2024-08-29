import socket
import threading

class Client:
    def __init__(self, ip, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_address = (ip, port)
        self.running = True

        # Connection
        try:
            self.sock.connect(self.server_address)
            print("Connected to the server")
        except Exception as e:
            print(f"Error connecting to server: {e}")
            self.sock.close()
            exit(1)

    def receive_messages(self):
        while self.running:
            try:
                data = self.sock.recv(1024).decode()
                if data:
                    print(f"Server: {data}")
                else:
                    print("Connection closed by the server")
                    self.running = False
            except Exception as e:
                print(f"Error receiving message: {e}")
                self.running = False

    def start(self):
        # Start thread to receive messages
        thread = threading.Thread(target=self.receive_messages)
        thread.daemon = True
        thread.start()

        # Menu
        while self.running:
            print("\nSelect an option:")
            print("1. Generate username")
            print("2. Generate password")
            print("0. Exit\n")

            option = input("Enter option: ").strip()

            if option == "0":
                break

            length = None
            if option == "1":
                length = int(input("Enter username length (5-15): ").strip())
                if length < 5 or length > 15:
                    print("Invalid length")
                    continue
                message = f"username {length}"
            elif option == "2":
                length = int(input("Enter password length (8-50): ").strip())
                if length < 8 or length > 50:
                    print("Invalid length")
                    continue
                message = f"password {length}"
            else:
                print("Invalid option")
                continue

            try:
                self.sock.sendall(message.encode())
            except Exception as e:
                print(f"Error sending message: {e}")
                self.running = False

        self.sock.close()

if __name__ == "__main__":
    client = Client("127.0.0.1", 8080)
    client.start()
