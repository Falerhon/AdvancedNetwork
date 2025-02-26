#include "spdlog/spdlog.h"
#include "Client.h"

std::string client_from_ip;
std::array<char, 65535> buffer;
uint32_t streamId = -1;
bool doOnce = true;

Client::Client() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    lastPing = std::chrono::high_resolution_clock::now();
    lastPing_ack = std::chrono::high_resolution_clock::now();

    falcon = Falcon::Connect("127.0.0.1", 5556);
    falcon->SetBlocking(false);
    falcon->OnConnectionEvent([this](bool success, uint64_t uuid) {
        this->ConnectionEvent(success, uuid);
    });
    falcon->OnDisconnect([this]() {
        this->Disconnection();
    });

    client_from_ip.resize(255);
}

void Client::Update() {
        //Clearing the buffer
        buffer = {{}};

        int recv_size = falcon->ReceiveFrom(client_from_ip, buffer);

        if (recv_size > 0) {

            ClientMessage mess = ClientMessage(CurrentUUID);
            mess.readBuffer(buffer);

            switch (mess.MessType) {
                //Connection
                case 0:
                    std::cout << "Connection to the client is not supported" << std::endl;
                    break;
                //Connectio_ACK
                case 1: {
                    HandleConnection_ACK(mess);
                }
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
                case 4:
                    HandlePing(mess);
                break;
                //Ping_ACK
                case 5:
                    std::cout << "Ping acknowledged" << std::endl;
                    HandlePing_ACK(mess);
                break;
                //StreamCreate
                case 6: {
                    HandleStreamCreate(mess);
                    break;
                }
                //StreamCreate_ACK
                case 7:
                    HandleStreamCreate_ACK(mess);
                break;
                //StreamData
                case 8:
                    HandleStreamData(buffer);
                break;
                //StreamData_ACK
                case 9:
                    HandleStreamData_ACK(buffer);
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

                uint32_t id = falcon->CreateStream(true);
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
            return;
        }
}

void Client::ConnectToServer() {
    ClientMessage message = ClientMessage(CurrentUUID);
    std::array<char, 65535> SendBuffer;
    message.MessType = CONNECT;
    message.WriteBuffer(SendBuffer);
    std::string dataStr = "0";

    //falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
    falcon->SendTo("127.0.0.1", 5555, std::span{dataStr.data(), dataStr.length()});
}

void Client::ConnectionEvent(bool success, uint64_t uuid) {
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

void Client::Disconnection() {
    std::cout << "Lost connection with the server" << std::endl;
    exit(0);
}

void Client::PingServer() {
    if (std::chrono::high_resolution_clock::now() - lastPing > std::chrono::seconds(PINGTIME)) {
        std::cout << "Client pinging server" << std::endl;
        ClientMessage message = ClientMessage(CurrentUUID);
        std::array<char, 65535> SendBuffer;
        message.MessType = PING;
        message.WriteBuffer(SendBuffer);

        falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});
        lastPing = std::chrono::high_resolution_clock::now();
    }
}

void Client::HandleConnection_ACK(const ClientMessage &mess) {
    uint64_t FoundUUID = -1;

    bool success = mess.Data[0];

    memcpy(&FoundUUID, &mess.Data[1], sizeof(uint64_t));

    falcon->clientConnectionHandler(success, FoundUUID);
}

void Client::HandleDisconnection(const ClientMessage &mess) {
    ClientMessage message = ClientMessage(CurrentUUID);
    std::array<char, 65535> SendBuffer;
    message.MessType = DISCONNECT_ACK;
    message.WriteBuffer(SendBuffer);

    falcon->SendTo("127.0.0.1", 5555, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

    std::cout << "Server sent disconnection" << std::endl;
    falcon->serverDisconnectionHandler();
}

void Client::HandleDisconnection_ACK(const ClientMessage &mess) {
    std::cout << "Server acknowledged the disconnection" << std::endl;
}

void Client::HandlePing(const ClientMessage &mess) {
    std::cout << "One-way ping is currently used, server pinging not supported" << std::endl;
}

void Client::HandlePing_ACK(const ClientMessage &mess) {
    lastPing_ack = std::chrono::high_resolution_clock::now();
}

void Client::HandleStreamCreate(const ClientMessage &mess) {
    std::cout << "Server created a stream" << std::endl;

    uint32_t id;
    memcpy(&id, &mess.Data[0], sizeof(id));
    uint32_t idCreated = falcon->CreateStreamFromExternal(id, true);
    std::cout << "Stream Created : " << std::to_string(idCreated) << std::endl;
}

void Client::HandleStreamCreate_ACK(const ClientMessage &mess) {
    std::cout << "Server acknowledged stream creation" << std::endl;
}

void Client::HandleStreamData(std::array<char, 65535> &recieveBuffer) {
    uint32_t streamId;
    memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
    falcon->HandleStreamData(streamId, recieveBuffer);
}

void Client::HandleStreamData_ACK(std::array<char, 65535> &recieveBuffer) {
    std::cout << "Acknowledged stream data" << std::endl;
    uint32_t streamId;
    memcpy(&streamId, &recieveBuffer[1], sizeof(streamId));
    falcon->HandleAcknowledgeData(streamId, recieveBuffer);
}