#include <iostream>
#include <Socket.h>

std::string TypeMessage() {
    std::string message;

    std::getline(std::cin, message);

    return message;

}

int main() {
    Socket* socket = Socket::CreateSocket();

    //Change the string here to change the destination IP
    const std::string IPAddress = "127.0.0.1";
    socket->CreateClientSocket(IPAddress);

    socket->MessageSend(TypeMessage());
    char buffer[1024];
    socket->MessageReceived(buffer, sizeof(buffer));
    socket->CloseSocket();
    return 0;
}