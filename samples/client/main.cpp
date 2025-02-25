#include <iostream>

#include <falcon.h>
#include "spdlog/spdlog.h"

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

uint64_t CurrentUUID = -1;

void ConnectToServer();
void ConnectionEvent(bool success, uint64_t uuid);
void Disconnection();

std::unique_ptr<Falcon> falcon;

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing_ack = std::chrono::high_resolution_clock::now();

    bool doOnce = true;

    falcon = Falcon::Connect("127.0.0.1", 5556);
    falcon->SetBlocking(false);
    falcon->OnConnectionEvent(ConnectionEvent);
    falcon->OnDisconnect(Disconnection);
    //Sending a connection request
    ConnectToServer();

    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    uint32_t streamId = -1;

    while (true) {
        //Clearing the buffer
        buffer = {{}};

        int recv_size = falcon->ReceiveFrom(from_ip, buffer);

        if (recv_size > 0) {

            ClientMessage mess = ClientMessage(CurrentUUID);
            mess.readBuffer(buffer);

            switch (mess.MessType) {
                //Connection
                case 0:

                    break;
                //Connectio_ACK
                case 1: {
                    uint64_t FoundUUID = -1;
                    bool success;
                    //TODO : Make sure this works
                    success = mess.Data[0];

                    memcpy(&FoundUUID, &buffer[2], sizeof(uint64_t));

                    falcon->clientConnectionHandler(success, FoundUUID);
                }
                break;
                //Disconnection
                case 2: {
                    ClientMessage message = ClientMessage(CurrentUUID);
                    std::array<char, 65535> SendBuffer;
                    message.MessType = DISCONNECT_ACK;
                    message.WriteBuffer(SendBuffer);

                    falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

                    std::cout << "Server sent disconnection" << std::endl;
                    falcon->serverDisconnectionHandler();
                    break;
                }
                //Disconnection_ACK
                case 3:
                    std::cout << "Server acknowledged the disconnection" << std::endl;
                    falcon->serverDisconnectionHandler();
                break;
                //Ping
                case 4:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //Ping_ACK
                case 5:
                    //std::cout << "Ping acknowledged" << std::endl;
                    lastPing_ack = std::chrono::high_resolution_clock::now();
                break;
                //StreamCreate
                case 6: {
                    std::cout << "Server created a stream" << std::endl;

                    std::string dataStr = buffer.data();
                    //Remove message type
                    dataStr.erase(dataStr.begin());

                    uint32_t FoundStreamId = -1;
                    try {
                        FoundStreamId = std::stoull(dataStr);
                    }catch(const std::invalid_argument& e){}

                    if (FoundStreamId != -1) {
                        //TODO : Stream creation
                    }

                    break;
                }
                //StreamCreate_ACK
                case 7:
                    std::cout << "Acknowledged stream creation" << std::endl;
                break;
                //StreamData
                case 8:
                    std::cout << "Data recieved" << std::endl;
                    //TODO : STREAM READING DATA
                break;
                //StreamData_ACK
                case 9:
                    std::cout << "Acknowledged stream data" << std::endl;
                    uint32_t streamId;
                    memcpy(&streamId, &buffer[1], sizeof(streamId));
                    falcon->HandleAcknowledgeData(streamId, buffer);
                break;
                default:
                    std::cout << "Unknown message type " << mess.MessType << std::endl;
                break;
            }
        }

        if (std::chrono::high_resolution_clock::now() - lastPing > std::chrono::seconds(PINGTIME)) {
            //std::cout << "Sending ping to server" << std::endl;
            ClientMessage message = ClientMessage(CurrentUUID);
            std::array<char, 65535> SendBuffer;
            message.MessType = PING;
            message.WriteBuffer(SendBuffer);

            falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
            lastPing = std::chrono::high_resolution_clock::now();

            //TODO : MOVE THIS IN ANOTHER PLACE
            if (doOnce) {
                doOnce = false;

                uint32_t id = falcon->CreateStream(false);
                std::cout << "Stream Created : " << std::to_string(id) << std::endl;
                ClientMessage messageTemp = ClientMessage(CurrentUUID);
                std::array<char, 65535> SendBufferTemp;
                message.MessType = STREAM_CREATE;
                memcpy(&message.Data, &id, sizeof(id));
                message.WriteBuffer(SendBufferTemp);

                falcon->SendTo("127.0.0.1", 5555, std::span{SendBufferTemp.data(), static_cast<unsigned long>(std::strlen(SendBufferTemp.data()))});

                streamId = id;
            } else{

                if (streamId != -1) {
                    std::array<char, 65535> SendBufferTemp;
                    falcon->SendStreamData(SendBufferTemp, streamId);
                }

            }
        }

        if (std::chrono::high_resolution_clock::now() - lastPing_ack > std::chrono::seconds(TIMEOUTTIME)) {
            std::cout << "Server has timed out" << std::endl;

            //Telling the server we are disconnecting
            //Most likely won't reach the server
            ClientMessage messageTemp = ClientMessage(CurrentUUID);
            std::array<char, 65535> SendBufferTemp;
            messageTemp.MessType = DISCONNECT;
            messageTemp.WriteBuffer(SendBufferTemp);

            falcon->SendTo("127.0.0.1", 5555, std::span{SendBufferTemp.data(), static_cast<unsigned long>(std::strlen(SendBufferTemp.data()))});
            break;
        }
    }


    return EXIT_SUCCESS;
}

void ConnectToServer() {
    ClientMessage message = ClientMessage(CurrentUUID);
    std::array<char, 65535> SendBuffer;
    message.MessType = CONNECT;
    message.WriteBuffer(SendBuffer);
    std::string dataStr = "0";

    //falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
    falcon->SendTo("127.0.0.1", 5555, std::span{dataStr.data(), dataStr.length()});
}

void ConnectionEvent(bool success, uint64_t uuid) {
    if (success) {
        CurrentUUID = uuid;
        ClientMessage message = ClientMessage(CurrentUUID);
        std::array<char, 65535> SendBuffer;
        message.MessType = CONNECT_ACK;
        message.WriteBuffer(SendBuffer);

        falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
    } else {
        std::cout << "Connection failed" << std::endl;
    }
}

void Disconnection() {
    std::cout << "Lost connection with the server" << std::endl;
    exit(0);
}