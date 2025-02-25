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
    ServerMessage () {UserId = -1;};

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

uint64_t RegisterUser(std::string &address, uint16_t &port);

void ConnectionConfirmation(uint64_t UUID);

std::vector<uint64_t> CheckPing();

void ClientDisconnected(uint64_t UUID);

std::unique_ptr<Falcon> falcon;

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    //Users to remove from known users
    std::vector<uint64_t> UsersToDisconnect;

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
                case 0: {
                    //Check if user is known
                    if (std::find(knownUsers.begin(), knownUsers.end(), mess.UserId) == knownUsers.end()) {
                        uint64_t NewUUID = RegisterUser(ip, port);

                        std::cout << "New user id : " << NewUUID << std::endl;

                        falcon->serverConnectionHandler(NewUUID);

                    } else {
                        std::cout << "User id is present : " << mess.UserId << std::endl;
                    }
                }
                break;
                //Connectio_ACK
                case 1:
                    std::cout << "Client connection confirmed " << mess.Data.data() << std::endl;
                    break;
                //Disconnection
                case 2: {
                    std::cout << "Client sent disconnection : " << mess.UserId << std::endl;

                    ServerMessage message = ServerMessage();
                    message.MessType = DISCONNECT_ACK;
                    std::array<char, 65535> SendBuffer;

                    message.WriteBuffer(SendBuffer);
                    falcon->SendTo(
                    ip, port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

                    falcon->clientDisconnectionHandler(mess.UserId);

                    break;
                }
                //Disconnection_ACK
                case 3:
                    std::cout << "Client acknowledged disconnection" << std::endl;

                    falcon->clientDisconnectionHandler(mess.UserId);
                    break;
                //Ping
                case 4: {
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
                        ip, port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
                    }
                break;}
                //Ping_ACK
                case 5:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //StreamCreate
                case 6: {
                    const auto it = std::find(knownUsers.begin(), knownUsers.end(), mess.UserId);
                    if (it == knownUsers.end()) {
                        std::cout << "Unknown user tried to create stream" << std::endl;
                    } else {
                            uint32_t id;
                            memcpy(&id, &mess.Data[0], sizeof(id));
                            uint32_t idCreated = falcon->CreateStreamFromExternal(id, false);
                            std::cout << "Stream Created : " << std::to_string(idCreated) << std::endl;
                        }
                break;}
                //Stream_Create_ACK
                case 7:

                    std::cout << "Stream creation acknowledged! : " << recieveBuffer.data() << std::endl;
                break;
                //Stream_Data
                case 8: {

                    std::cout << "Recieving stream data" << mess.Data.data() << std::endl;
                    //TODO : STREAM READING DATA
                    break;
                }
                //Stream_Data_ACK
                case 9:
                    std::cout << "Stream data acknowledged" << std::endl;
                break;
                default:
                    std::cout << "Unknown message type " << messageType << std::endl;
                    break;
            }
            //Clean up the message
            mess.~ServerMessage();
        }
        //Disconnect timed out user
        UsersToDisconnect = CheckPing();
        if (UsersToDisconnect.size() > 0) {
            for (auto user: UsersToDisconnect) {
                falcon->clientDisconnectionHandler(user);
                //Telling the client we are disconnecting them
                //Most likely won't reach the client
                ServerMessage message = ServerMessage();
                message.MessType = DISCONNECT;
                std::array<char, 65535> SendBuffer;

                message.WriteBuffer(SendBuffer);

                falcon->SendTo(
                    ip, port, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
            }
        }
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

std::vector<uint64_t> CheckPing() {
    std::vector<uint64_t> TimedOutUsers;
    for (const auto& user: knownUsers) {
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
        knownUsers.erase(it);
    }
}