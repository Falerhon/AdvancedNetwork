#include <iostream>

#include <falcon.h>

#include "spdlog/spdlog.h"

uint64_t CurrentUUID = -1;

void ComposeMessage(MessageType type, std::string& message);
void ConnectionEvent(bool success, uint64_t uuid);
void Disconnection();

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing;

    auto falcon = Falcon::Connect("127.0.0.1", 5556);
    falcon->OnConnectionEvent(ConnectionEvent);
    falcon->OnDisconnect(Disconnection);
    //Sending a connection request
    std::string message;
    ComposeMessage(CONNECT, message);
    std::span data(message.data(), message.size());
    falcon->SendTo("127.0.0.1", 5555, data);

    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;

    //TODO: Modify this loop, currently only used for connection verification
    while (true) {
        falcon->ReceiveFrom(from_ip, buffer);

        std::string strid = buffer.data();
        std::cout << buffer.data() << std::endl;

        //Get the message type
        int messageType = -1;
        //ASCII to numeral
        messageType = strid.front() - 48;
        strid.erase(strid.begin());

        switch (messageType) {
            //Connection
            case 0:

                break;
            //Connectio_ACK
            case 1: {
                uint64_t FoundUUID = -1;
                bool success = false;
                try {
                    success = (int)strid.front() - 48;
                }catch(const std::invalid_argument& e){}
                strid.erase(strid.begin());

                if (success) {
                    FoundUUID = std::stoull(strid);
                } else {
                    FoundUUID = -1;
                }

                falcon->clientConnectionHandler(success, FoundUUID);

                message = "Connected!";
                ComposeMessage(CONNECT_ACK, message);
            }
            break;
            //Disconnection
            case 2:
                std::cout << "Not implemented yet" << std::endl;
                break;
            //Disconnection_ACK
            case 3:
                std::cout << "Not implemented yet" << std::endl;
                break;
            default:
                std::cout << "Unknown message type " << messageType << std::endl;
                break;
        }

        falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});
        break;
    }


    return EXIT_SUCCESS;
}

void ComposeMessage(MessageType type, std::string& message) {
    message = std::to_string(type) + std::to_string(CurrentUUID) + message;
}

void ConnectionEvent(bool success, uint64_t uuid) {
    if (success) {
        CurrentUUID = uuid;
        std::cout << "Connection successful with UUID : " << CurrentUUID << std::endl;
    } else {
        std::cout << "Connection failed" << std::endl;
    }
}

void Disconnection() {
    std::cout << "Lost connection with the server" << std::endl;
    exit(0);
}