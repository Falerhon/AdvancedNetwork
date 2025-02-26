#include <falcon.h>
#include <cstring>
#include <chrono>

#ifndef SERVER_H
#define SERVER_H

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
        memcpy(&buff[1], &Data, std::strlen(Data.data()));
    }
};

class Server {
  public:
    Server();

    ~Server();

    Server(const Server &) = default;

    Server &operator=(const Server &) = default;

    Server(Server &&) = default;

    Server &operator=(Server &&) = default;

    void Update();

    uint64_t RegisterUser(std::string &address, uint16_t &port);

    void ConnectionConfirmation(uint64_t UUID);

    std::vector<uint64_t> CheckPing();

    void ClientDisconnected(uint64_t UUID);

    void CreateStream(uint64_t UUID, bool isReliable);

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

    //List of known users
    std::vector<User> knownUsers;
};



#endif //SERVER_H
