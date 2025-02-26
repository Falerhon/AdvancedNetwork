#ifndef CLIENT_H
#define CLIENT_H

#include <chrono>
#include <falcon.h>

struct ClientMessage : Message {
    uint64_t UserId;
    ClientMessage (uint64_t userId): UserId(userId) {};

    void readBuffer(std::array<char, 65535> &buff) override {
        MessType = buff[0] & 0x0F;
        if (!buff.size() <= 2) {
            memcpy(&Data, &buff[1], buff.size() - 1);
        }
    };

    void WriteBuffer(std::array<char, 65535> &buff) {
        char chrType = MessType & 0x0F;
        memcpy(&buff[0], &chrType, sizeof(chrType));
        memcpy(&buff[1], &UserId, sizeof(uint64_t));
        memcpy(&buff[9], &Data, Data.size());
    }
};

class Client {
public:
      Client();

    uint64_t CurrentUUID = -1;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing_ack;

    void Update();

    void ConnectToServer();
    void ConnectionEvent(bool success, uint64_t uuid);
    void Disconnection();
    //Handling of the packet according to their type
    void HandleConnection_ACK(const ClientMessage &mess);
    void HandleDisconnection(const ClientMessage &mess);
    void HandleDisconnection_ACK(const ClientMessage &mess);
    void HandlePing(const ClientMessage &mess);
    void HandlePing_ACK(const ClientMessage &mess);
    void HandleStreamCreate(const ClientMessage &mess);
    void HandleStreamCreate_ACK(const ClientMessage &mess);
    void HandleStreamData(std::array<char, 65535> &recieveBuffer);
    void HandleStreamData_ACK(std::array<char, 65535> &recieveBuffer);

    std::unique_ptr<Falcon> falcon;

};



#endif //CLIENT_H
