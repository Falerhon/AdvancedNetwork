//
// Created by alex- on 2025-02-18.
//

#include "stream.h"

#include <iostream>
#include <random>
#include <cstring>
#include "falcon.h"

//Server
std::unique_ptr<Stream> Stream::CreateStream(uint64_t client,std::string endpIp, int endpPort, bool reliable, Falcon* sock) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = GenerateId();
    stream->linkedClient = client;
    stream->endpointIp = endpIp;
    stream->endpointPort = endpPort;

    return stream;
}

std::unique_ptr<Stream> Stream::CreateStreamExternal(uint32_t id, uint64_t client, std::string endpIp, int endpPort,
    bool reliable, Falcon* sock) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = id;
    stream->linkedClient = client;
    stream->endpointIp = endpIp;
    stream->endpointPort = endpPort;

    return stream;
}

//Client
std::unique_ptr<Stream> Stream::CreateStream(const bool reliable, Falcon* sock) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = GenerateId();
    stream->endpointIp  = "127.0.0.1";
    stream->endpointPort = 5555;

    return stream;
}

std::unique_ptr<Stream> Stream::CreateStreamExternal(uint32_t id, bool reliable, Falcon* sock) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = id;
    stream->endpointIp  = "127.0.0.1";
    stream->endpointPort = 5555;

    return stream;
}

void Stream::CloseStream(const Stream &stream) {
    //TODO : TELL THE CLIENT THIS STREAM IS CLOSING
}

void Stream::SendData(std::span<const char> Data) {
    std::cout << "Sending message to linked client" << std::endl;

    int packetContent = 1000 + currentPacketId;
    std::array<char, 65535> SendBuffer;
    //TODO : THIS IS HARD CODED TO TEST
    char chrType = 8 & 0x0F;

    memcpy(&SendBuffer[0], &chrType, sizeof(chrType));
    memcpy(&SendBuffer[1], &packetContent, sizeof(packetContent));

    if (isReliable) {
        std::cout << "Stream is reliable" << std::endl;
        previousData.insert(std::pair{currentPacketId, packetContent});
        //TODO : RELIABILITY
    }

    socket->SendTo(endpointIp, endpointPort, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

    currentPacketId++;
}

void Stream::OnDataReceived(std::span<const char> Data) {
    std::cout << "Received message from client" << std::endl;

}

uint32_t Stream::GenerateId() {
        //Generate a random identifier
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<unsigned long> dis(
            std::numeric_limits<std::uint32_t>::min(),
            std::numeric_limits<std::uint32_t>::max()
        );

        uint32_t ID = dis(gen);
        return ID;
}
