#include <iostream>

#include <falcon.h>
#include <stream.h>
#include <random>

#include "spdlog/spdlog.h"

//Struct used to hold the user info
struct User {
    uint64_t UUID;
    std::string address;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing;

    User(const uint64_t id, const std::string &add): UUID(id), address(add) {
        lastPing = std::chrono::high_resolution_clock::now();
    };
    bool operator==(uint64_t id) const { return UUID == id; }
    bool operator==(const User &user) const { return user.UUID == UUID; }
};

//List of known users
std::vector<User> knownUsers;

uint64_t RegisterUser(std::string &address);

void ConnectionConfirmation(uint64_t UUID);

void ComposeMessage(MessageType type, std::string &message);

std::vector<uint64_t> CheckPing();

void ClientDisconnected(uint64_t UUID);

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    //Users to remove from known users
    std::vector<uint64_t> UsersToDisconnect;
    //Streams existing on this instance
    std::map<uint32_t, std::unique_ptr<Stream>> existingStream;

    auto falcon = Falcon::Listen("127.0.0.1", 5555);
    falcon->SetBlocking(false);
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;

    falcon->OnClientConnected(ConnectionConfirmation);
    falcon->OnClientDisconnected(ClientDisconnected);

    while (true) {
        //Clearing the buffer
        buffer = {{}};

        int recv_size = falcon->ReceiveFrom(from_ip, buffer);
        std::string ip = from_ip;
        uint16_t port = 0;
        auto pos = from_ip.find_last_of(':');
        if (pos != std::string::npos) {
            ip = from_ip.substr(0, pos);
            std::string port_str = from_ip.substr(++pos);
            port = atoi(port_str.c_str());
        }

        //Making sure there is a message to be read
        if (!buffer[0] == '\0') {
            //Converting buffer data to string
            std::string strid = buffer.data();
            //Convert string to uint64_t
            uint64_t SearchedUUID = 0;

            int messageType = -1;


            //ASCII to numeral
            messageType = strid.front() - 48;

            strid.erase(strid.begin());

            //Avoid error if returned ID is invalid
            try {
                //Get the message sender UUID
                SearchedUUID = std::stoull(strid);
            } catch (const std::invalid_argument &e) {
                SearchedUUID = -1;
            }

            switch (messageType) {
                //Connection
                case 0: {
                    //Check if user is known
                    if (std::find(knownUsers.begin(), knownUsers.end(), SearchedUUID) == knownUsers.end()) {
                        SearchedUUID = RegisterUser(ip);

                        std::cout << "New user id : " << SearchedUUID << std::endl;

                        falcon->serverConnectionHandler(SearchedUUID);
                        //Connection successful
                        std::string message = std::to_string(true);
                        //Adding UUID
                        message.append(std::to_string(SearchedUUID));
                        ComposeMessage(CONNECT_ACK, message);
                        falcon->SendTo(
                            ip, port, std::span{message.data(), static_cast<unsigned long>(message.length())});
                    } else {
                        std::cout << "User id is present : " << SearchedUUID << std::endl;
                    }
                }
                break;
                //Connectio_ACK
                case 1:
                    std::cout << "Client : " << buffer.data() << std::endl;
                    break;
                //TODO : Implement this
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

                    falcon->clientDisconnectionHandler(SearchedUUID);
                    break;
                //Ping
                case 4: {
                    //Find the sending user
                    const auto it = std::find(knownUsers.begin(), knownUsers.end(), SearchedUUID);
                    if (it == knownUsers.end()) {
                        std::cout << "Unknown user tried to ping" << std::endl;
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
                    const auto it = std::find(knownUsers.begin(), knownUsers.end(), SearchedUUID);
                    if (it == knownUsers.end()) {
                        std::cout << "Unknown user tried to ping" << std::endl;
                    } else {
                        auto stream = Stream::CreateStream(SearchedUUID, false);
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
                    std::cout << "Stream creation acknowledged!" << std::endl;
                break;
                //Stream_Data
                case 8:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //Stream_Data_ACK
                case 9:
                    std::cout << "Not implemented yet" << std::endl;
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

uint64_t RegisterUser(std::string &address) {
    //Generate a random identifier
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<unsigned long long> dis(
        std::numeric_limits<std::uint64_t>::min(),
        std::numeric_limits<std::uint64_t>::max()
    );

    uint64_t UUID = dis(gen);
    //Add user to known user list
    knownUsers.emplace_back(UUID, address);
    return UUID;
}

void ConnectionConfirmation(uint64_t UUID) {
    std::cout << "Confirming the creation of new user : " << UUID << std::endl;
    knownUsers[std::find(knownUsers.begin(), knownUsers.end(), UUID) == knownUsers.end()].lastPing =
            std::chrono::high_resolution_clock::now();
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
