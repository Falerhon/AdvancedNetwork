#include <iostream>
#include <Socket.h>

std::string TypeMessage() {
    std::string message;

    std::getline(std::cin, message);

    return message;

}

int main() {
    Socket* socket = new Socket();
    socket->CreateClientSocket("127.0.0.1");

    socket->MessageSend(*TypeMessage().c_str());
    socket->MessageRecieve();

    return 0;
}