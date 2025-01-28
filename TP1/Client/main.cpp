#include <iostream>
#include <Socket.h>

std::string TypeMessage() {
    std::string message;

    std::getline(std::cin, message);

    return message;

}

int main() {
    Socket* socket = Socket::CreateSocket();

    std::string IPAddress;
    std::string AddressType;

    std::cout << "Type of destination address : IPV4 (4) or IPV6 (6)";

    while (true) {
        std::cin >> AddressType;

        //String comparison is bad
        if (AddressType == "4") {
            IPAddress = "127.0.0.1";
            break;
        } else if (AddressType == "6") {
            IPAddress = "::1";
            break;
        } else {
            std::cin.clear();
            std::cout << "Input invalid. Valid inputs : 4 - 6";
        }
    }

    //Clear the cin
    std::cin.clear();
    std::cin.ignore();

    socket->CreateClientSocket(IPAddress);

    socket->MessageSend(TypeMessage());
    char buffer[1024];
    socket->MessageReceived(buffer, sizeof(buffer));
    socket->CloseSocket();
    return 0;
}