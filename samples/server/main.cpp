#include <iostream>

#include <falcon.h>
#include <stream.h>
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

struct Message {
    int type;
    uint64_t UserId;
    std::array<char, 65535> data;
};

//List of known users
std::vector<User> knownUsers;

uint64_t RegisterUser(std::string &address, uint16_t &port);

void ConnectionConfirmation(uint64_t UUID);

void ComposeMessage(MessageType type, std::string &message);

std::vector<uint64_t> CheckPing();

void ClientDisconnected(uint64_t UUID);

std::unique_ptr<Falcon> falcon;

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    //Users to remove from known users
    std::vector<uint64_t> UsersToDisconnect;
    //Streams existing on this instance
    std::map<uint32_t, std::unique_ptr<Stream>> existingStream;

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
        if (!recieveBuffer[0] == '\0') {
            //Converting buffer data to string
            std::string strid = recieveBuffer.data();
            //Convert string to uint64_t
            uint64_t SearchedUUID = 0;

            int messageType = -1;

            Message mess;
            //ASCII to numeral
            mess.type = strid.front() - 48;

            strid.erase(strid.begin());

            std::string idStr = strid.substr(0, 20);

            // //Avoid error if returned ID is invalid
            try {
                //Get the message sender UUID
                mess.UserId = std::stoull(idStr);
            } catch (const std::invalid_argument &e) {
                SearchedUUID = -1;
            }

            strid.erase(0, 20);

            strcpy(mess.data.data(), strid.data());

            switch (mess.type) {
                //Connection
                case 0: {
                    //Check if user is known
                    if (std::find(knownUsers.begin(), knownUsers.end(), SearchedUUID) == knownUsers.end()) {
                        SearchedUUID = RegisterUser(ip, port);

                        std::cout << "New user id : " << SearchedUUID << std::endl;

                        falcon->serverConnectionHandler(SearchedUUID);

                    } else {
                        std::cout << "User id is present : " << SearchedUUID << std::endl;
                    }
                }
                break;
                //Connectio_ACK
                case 1:
                    std::cout << "Client : " << recieveBuffer.data() << std::endl;
                    break;
                //Disconnection
                case 2: {
                    std::cout << "Client sent disconnection : " << SearchedUUID << std::endl;

                    std::string message;
                    ComposeMessage(DISCONNECT_ACK, message);
                    falcon->SendTo(
                        ip, port, std::span{message.data(), static_cast<unsigned long>(message.length())});

                    falcon->clientDisconnectionHandler(SearchedUUID);

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
                        std::string message = "Ping recieved";
                        ComposeMessage(PING_ACK, message);
                        falcon->SendTo(ip, port, std::span{message.data(), static_cast<unsigned long>(message.length())});
                    } else {
                        //For debug purposes
                        //std::cout << SearchedUUID <<" sent a ping" << std::endl;
                        it->lastPing = std::chrono::high_resolution_clock::now();
                        std::string message = "Ping recieved";
                        ComposeMessage(PING_ACK, message);

                        falcon->SendTo(it->address, port, std::span{message.data(), static_cast<unsigned long>(message.length())});
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
                        std::cout << "Unknown user tried to ping" << std::endl;
                    } else {
                        auto stream = Stream::CreateStream(mess.UserId, false);
                        auto streamId = stream->id;
                        //Adding the stream
                        existingStream.insert(std::pair(stream->id, std::move(stream)));
                        //Preparing the stream created message
                        std::string message = std::to_string(streamId);
                        ComposeMessage(STREAM_CREATE, message);

                        falcon->SendTo(it->address, port, std::span{message.data(), static_cast<unsigned long>(message.length())});
                    }

                    std::cout << "Stream Created" << std::endl;
                break;}
                //Stream_Create_ACK
                case 7:

                    std::cout << "Stream creation acknowledged! : " << recieveBuffer.data() << std::endl;
                break;
                //Stream_Data
                case 8: {

                    std::cout << "Recieving stream data" << mess.data.data() << std::endl;
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
        }
        //Disconnect timed out user
        UsersToDisconnect = CheckPing();
        if (UsersToDisconnect.size() > 0) {
            for (auto user: UsersToDisconnect) {
                falcon->clientDisconnectionHandler(user);
                //Telling the client we are disconnecting them
                //Most likely won't reach the client
                std::string message;
                ComposeMessage(DISCONNECT, message);
                falcon->SendTo(
                    ip, port, std::span{message.data(), static_cast<unsigned long>(message.length())});
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
    char header = type & 0x0F;
    //Set the 5th bit to be the success
    header |= (success << 4);
    memcpy(&sendBuffer, &header, sizeof(header));
    memcpy(&sendBuffer[1], &UUID, sizeof(UUID));

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

void ComposeMessage(MessageType type, std::string &message) {
    message = std::to_string(type) + message;
}