#include <iostream>
#include <random>
#include "spdlog/spdlog.h"
#include "Server.h"


std::array<char, 65535> recieveBuffer;
std::string from_ip;

Server::Server() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");

    falcon = Falcon::Listen("127.0.0.1", 5555);
    falcon->SetBlocking(false);

    from_ip.resize(255);

    falcon->OnClientConnected([this](uint64_t uuid) {
        this->ConnectionConfirmation(uuid);
    });
    falcon->OnClientDisconnected([this](uint64_t uuid) {
        this->ClientDisconnected(uuid);
    });
}

void Server::Update() {
    //Clearing the buffer
    recieveBuffer = {{}};

    int recv_size = falcon->ReceiveFrom(from_ip, recieveBuffer);
    std::string ip = from_ip;
    uint16_t port = 0;
    auto pos = from_ip.find_last_of(':');
    if (pos != std::string::npos) {
        ip = from_ip.substr(0, pos);
        std::string port_str = from_ip.substr(++pos);
        port = atoi(port_str.c_str());
    }

    //Making sure there is a message to be read
    if (recv_size > 0) {
        //Convert string to uint64_t

        int messageType = -1;

        ServerMessage mess = ServerMessage();

        mess.readBuffer(recieveBuffer);

        switch (mess.MessType) {
            //Connection
            case 0:
                HandleConnection(mess, ip, port);
                break;
            //Connectio_ACK
            case 1:
                HandleConnection_ACK(mess);
                break;
            //Disconnection
            case 2: {
                HandleDisconnection(mess);
                break;
            }
            //Disconnection_ACK
            case 3:
                HandleDisconnection_ACK(mess);
                break;
            //Ping
            case 4: {
                HandlePing(mess);
                break;
            }
            //Ping_ACK
            case 5:
                std::cout << "Ping Acknowledged" << std::endl;
                break;
            //StreamCreate
            case 6: {
                HandleStreamCreate(mess);
                break;
            }
            //Stream_Create_ACK
            case 7:
                HandleStreamCreate_ACK(mess);

                break;
            //Stream_Data
            case 8: {
                HandleStreamData(recieveBuffer);
                break;
            }
            //Stream_Data_ACK
            case 9:
                HandleStreamData_ACK(recieveBuffer);
                break;
            default:
                std::cout << "Unknown message type " << messageType << std::endl;
                break;
        }
        //Clean up the message
        mess.~ServerMessage();
    }
    CheckTimeout();
}

uint64_t Server::RegisterUser(std::string &address, uint16_t &port) {
    //Generate a random identifier
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<unsigned long long> dis(
        std::numeric_limits<std::uint64_t>::min(),
        std::numeric_limits<std::uint64_t>::max()
    );

    uint64_t UUID = dis(gen);
    //Add user to known user list
    knownUsers.emplace_back(UUID, address, port);
    return UUID;
}

void Server::ConnectionConfirmation(uint64_t UUID) {
    std::cout << "Confirming the creation of new user : " << UUID << std::endl;
    std::array<char, 65535> sendBuffer;
    MessageType type = CONNECT_ACK;
    bool success = true;

    auto it = std::find(knownUsers.begin(), knownUsers.end(), UUID);

    if (it == knownUsers.end()) {
        success = false;
    } else {
        knownUsers[std::find(knownUsers.begin(), knownUsers.end(), UUID) == knownUsers.end()].lastPing =
                std::chrono::high_resolution_clock::now();
    }

    //Set the first 4 bits to be the message type
    char charType = type & 0x0F;
    char charSuccess = success;
    memcpy(&sendBuffer, &charType, sizeof(charType));
    memcpy(&sendBuffer[1], &charSuccess, sizeof(charSuccess));
    memcpy(&sendBuffer[2], &UUID, sizeof(UUID));

    falcon->SendTo(it->address, it->port, std::span{sendBuffer.data(), static_cast<unsigned long>(std::strlen(sendBuffer.data()))});
}

void Server::CheckTimeout() {
    //Disconnect timed out user
    std::vector<uint64_t> UsersToDisconnect = CheckPing();
    if (!UsersToDisconnect.empty()) {
        for (auto user: UsersToDisconnect) {
            falcon->clientDisconnectionHandler(user);
        }
    }
}


std::vector<uint64_t> Server::CheckPing() {
    std::vector<uint64_t> TimedOutUsers;
    for (const auto &user: knownUsers) {
        if (std::chrono::high_resolution_clock::now() - user.lastPing > std::chrono::seconds(TIMEOUTTIME)) {
            std::cout << user.UUID << " has timed out" << std::endl;
            TimedOutUsers.push_back(user.UUID);
        }
    }
    return TimedOutUsers;
}

void Server::ClientDisconnected(uint64_t UUID) {
    const auto it = std::find(knownUsers.begin(), knownUsers.end(), UUID);
    if (it != knownUsers.end()) {
        ServerMessage message = ServerMessage();
        message.MessType = DISCONNECT;
        std::array<char, 65535> SendBuffer;

        message.WriteBuffer(SendBuffer);
        falcon->SendTo(
            it->address, it->port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

        knownUsers.erase(it);
    }
}

void Server::HandleConnection(const ServerMessage &mess, std::string &ip, uint16_t &port) {
    //Check if user is known
    if (std::find(knownUsers.begin(), knownUsers.end(), mess.UserId) == knownUsers.end()) {
        uint64_t NewUUID = RegisterUser(ip, port);

        std::cout << "New user id : " << NewUUID << std::endl;

        falcon->serverConnectionHandler(NewUUID);
    } else {
        std::cout << "User id is present : " << mess.UserId << std::endl;
    }
}

void Server::HandleConnection_ACK(const ServerMessage &mess) {
    std::cout << "Client connection confirmed " << mess.Data.data() << std::endl;
}

void Server::HandleDisconnection(const ServerMessage &mess) {
    std::cout << "Client sent disconnection : " << mess.UserId << std::endl;

    falcon->clientDisconnectionHandler(mess.UserId);
}

void Server::HandleDisconnection_ACK(const ServerMessage &mess) {
    std::cout << "Client acknowledged disconnection" << std::endl;
}

void Server::HandlePing(const ServerMessage &mess) {
    //Find the sending user
    const auto it = std::find(knownUsers.begin(), knownUsers.end(), mess.UserId);
    if (it == knownUsers.end()) {
        std::cout << "Unknown user tried to ping" << std::endl;
    } else {
        it->lastPing = std::chrono::high_resolution_clock::now();
        std::cout << "Server Received ping : " << std::endl;
        ServerMessage message = ServerMessage();
        message.MessType = PING_ACK;
        std::array<char, 65535> SendBuffer;

        message.WriteBuffer(SendBuffer);
        falcon->SendTo(
            it->address, it->port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
    }
}

void Server::HandlePing_ACK(const ServerMessage &mess) {
}

void Server::HandleStreamCreate(const ServerMessage &mess) {
    const auto it = std::find(knownUsers.begin(), knownUsers.end(), mess.UserId);
    if (it == knownUsers.end()) {
        std::cout << "Unknown user tried to create stream" << std::endl;
    } else {
        uint32_t id;
        memcpy(&id, &mess.Data[0], sizeof(id));
        uint32_t idCreated = falcon->CreateStreamFromExternal(id, it->UUID, it->address, it->port, true);
        std::cout << "Stream Created : " << std::to_string(idCreated) << std::endl;
    }
}

void Server::HandleStreamCreate_ACK(const ServerMessage &mess) {
    std::cout << "Stream creation acknowledged! : " << mess.Data.data() << std::endl;
}

void Server::HandleStreamData(std::array<char, 65535> &recieveBuffer) {
    uint32_t streamId;
    memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
    falcon->HandleStreamData(streamId, recieveBuffer);
}

void Server::HandleStreamData_ACK(std::array<char, 65535> &recieveBuffer) {
    std::cout << "Stream data acknowledged" << std::endl;
    uint32_t streamId;
    memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
    falcon->HandleAcknowledgeData(streamId, recieveBuffer);
}
