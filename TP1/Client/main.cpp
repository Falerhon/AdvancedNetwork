#include <iostream>
#include <Socket.h>

std::string TypeMessage() {
    std::string message;

    std::getline(std::cin, message);

    return message;

}

int main() {
    Socket* socket = Socket::CreateSocket();

    socket->CreateClientSocket("127.0.0.1");
    std::string msg = "Hello World!";
    socket->MessageSend(msg);
    char buffer[1024];
    socket->MessageReceived(buffer, sizeof(buffer));
    socket->CloseSocket();
    return 0;
}