#include <iostream>

#include <falcon.h>
#include <random>

#include "spdlog/spdlog.h"

//Struct used to hold the user info
struct User {
    uint64_t UUID;
    std::string address;
    uint16_t port;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing;

    User(const uint64_t id, const std::string &add, const uint16_t &prt): UUID(id), address(add), port(prt) {
        lastPing = std::chrono::high_resolution_clock::now();
    };
    bool operator==(uint64_t id) const { return UUID == id; }
    bool operator==(const User &user) const { return user.UUID == UUID; }
};

struct ServerMessage : Message {
    uint64_t UserId;
    ServerMessage() { UserId = -1; };

    void readBuffer(std::array<char, 65535> &buff) override {
        MessType = buff[0] & 0x0F;
        memcpy(&UserId, &buff[1], sizeof(uint64_t));
        if (!buff.size() <= 10) {
            memcpy(&Data, &buff[9], buff.size() - 9);
        }
    };

    void WriteBuffer(std::array<char, 65535> &buff) {
        char chrType = MessType & 0x0F;
        memcpy(&buff[0], &chrType, sizeof(chrType));
        memcpy(&buff[1], &Data, Data.size());
    }
};

//List of known users
std::vector<User> knownUsers;

//Function declaration
uint64_t RegisterUser(std::string &address, uint16_t &port);
void ConnectionConfirmation(uint64_t UUID);
std::vector<uint64_t> CheckPing();
void ClientDisconnected(uint64_t UUID);
void CheckTimeout();
//Handling of the packet according to their type
void HandleConnection(const ServerMessage &mess, std::string &ip, uint16_t &port);
void HandleConnection_ACK(const ServerMessage &mess);
void HandleDisconnection(const ServerMessage &mess);
void HandleDisconnection_ACK(const ServerMessage &mess);
void HandlePing(const ServerMessage &mess);
void HandlePing_ACK(const ServerMessage &mess);
void HandleStreamCreate(const ServerMessage &mess);
void HandleStreamCreate_ACK(const ServerMessage &mess);
void HandleStreamData(std::array<char, 65535> &recieveBuffer);
void HandleStreamData_ACK(std::array<char, 65535> &recieveBuffer);

std::unique_ptr<Falcon> falcon;

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    //Users to remove from known users

    falcon = Falcon::Listen("127.0.0.1", 5555);
    falcon->SetBlocking(false);
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> recieveBuffer;

    falcon->OnClientConnected(ConnectionConfirmation);
    falcon->OnClientDisconnected(ClientDisconnected);

    while (true) {
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


    return EXIT_SUCCESS;
}

uint64_t RegisterUser(std::string &address, uint16_t &port) {
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

void ConnectionConfirmation(uint64_t UUID) {
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

void CheckTimeout() {
    //Disconnect timed out user
    std::vector<uint64_t> UsersToDisconnect = CheckPing();
    if (!UsersToDisconnect.empty()) {
        for (auto user: UsersToDisconnect) {
            falcon->clientDisconnectionHandler(user);
        }
    }
}


std::vector<uint64_t> CheckPing() {
    std::vector<uint64_t> TimedOutUsers;
    for (const auto &user: knownUsers) {
        if (std::chrono::high_resolution_clock::now() - user.lastPing > std::chrono::seconds(TIMEOUTTIME)) {
            std::cout << user.UUID << " has timed out" << std::endl;
            TimedOutUsers.push_back(user.UUID);
        }
    }
    return TimedOutUsers;
}

void ClientDisconnected(uint64_t UUID) {
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

void HandleConnection(const ServerMessage &mess, std::string &ip, uint16_t &port) {
    //Check if user is known
    if (std::find(knownUsers.begin(), knownUsers.end(), mess.UserId) == knownUsers.end()) {
        uint64_t NewUUID = RegisterUser(ip, port);

        std::cout << "New user id : " << NewUUID << std::endl;

        falcon->serverConnectionHandler(NewUUID);
    } else {
        std::cout << "User id is present : " << mess.UserId << std::endl;
    }
}

void HandleConnection_ACK(const ServerMessage &mess) {
    std::cout << "Client connection confirmed " << mess.Data.data() << std::endl;
}
void HandleDisconnection(const ServerMessage &mess) {
    std::cout << "Client sent disconnection : " << mess.UserId << std::endl;

    falcon->clientDisconnectionHandler(mess.UserId);
}

void HandleDisconnection_ACK(const ServerMessage &mess) {
    std::cout << "Client acknowledged disconnection" << std::endl;
}

void HandlePing(const ServerMessage &mess) {
    //Find the sending user
    const auto it = std::find(knownUsers.begin(), knownUsers.end(), mess.UserId);
    if (it == knownUsers.end()) {
        std::cout << "Unknown user tried to ping" << std::endl;
    } else {
        //For debug purposes
        //std::cout << SearchedUUID <<" sent a ping" << std::endl;
        it->lastPing = std::chrono::high_resolution_clock::now();

        ServerMessage message = ServerMessage();
        message.MessType = PING_ACK;
        std::array<char, 65535> SendBuffer;

        message.WriteBuffer(SendBuffer);
        falcon->SendTo(
            it->address, it->port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
    }
}

void HandlePing_ACK(const ServerMessage &mess) {

}

void HandleStreamCreate(const ServerMessage &mess) {
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

void HandleStreamCreate_ACK(const ServerMessage &mess) {
    std::cout << "Stream creation acknowledged! : " << mess.Data.data() << std::endl;
}

void HandleStreamData(std::array<char, 65535> &recieveBuffer) {
    int random = rand() % 101;
    if (random > 50) {
        //std::cout << "Recieving stream data" << mess.Data.data() << std::endl;
        uint32_t streamId;
        memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
        falcon->HandleStreamData(streamId, recieveBuffer);
    }
}

void HandleStreamData_ACK(std::array<char, 65535> &recieveBuffer) {
    std::cout << "Stream data acknowledged" << std::endl;
    uint32_t streamId;
    memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
    falcon->HandleAcknowledgeData(streamId, recieveBuffer);
}
