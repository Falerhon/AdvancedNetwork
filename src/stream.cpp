//
// Created by alex- on 2025-02-18.
//

#include "stream.h"

#include <bitset>
#include <iostream>
#include <random>
#include <cstring>
#include "falcon.h"

//Server
std::unique_ptr<Stream> Stream::CreateStream(uint64_t client, std::string endpIp, int endpPort, bool reliable, Falcon &sock) {
    auto stream = std::make_unique<Stream>(sock);
    stream->isReliable = reliable;
    stream->id = GenerateId();
    stream->linkedClient = client;
    stream->endpointIp = endpIp;
    stream->endpointPort = endpPort;

    return stream;
}

std::unique_ptr<Stream> Stream::CreateStreamExternal(uint32_t id, uint64_t client, std::string endpIp, int endpPort,
                                                     bool reliable, Falcon &sock) {
    auto stream = std::make_unique<Stream>(sock);
    stream->isReliable = reliable;
    stream->id = id;
    stream->linkedClient = client;
    stream->endpointIp = endpIp;
    stream->endpointPort = endpPort;

    return stream;
}

//Client
std::unique_ptr<Stream> Stream::CreateStream(const bool reliable, Falcon &sock) {
    auto stream = std::make_unique<Stream>(sock);
    stream->isReliable = reliable;
    stream->id = GenerateId();
    stream->endpointIp = "127.0.0.1";
    stream->endpointPort = 5555;

    return stream;
}

std::unique_ptr<Stream> Stream::CreateStreamExternal(uint32_t id, bool reliable, Falcon &sock) {
    auto stream = std::make_unique<Stream>(sock);
    stream->isReliable = reliable;
    stream->id = id;
    stream->endpointIp = "127.0.0.1";
    stream->endpointPort = 5555;

    return stream;
}

void Stream::CloseStream(const Stream &stream) {
    //TODO : TELL THE CLIENT THIS STREAM IS CLOSING
}

void Stream::SendData(std::span<const char> Data) {
    std::cout << "Sending message via stream" << std::endl;

    int packetContent = 1000 + currentPacketId;
    std::array<char, 65535> SendBuffer;

    char chrType = MessageType::STREAM_DATA & 0x0F;

    //Type of the message
    memcpy(&SendBuffer[0], &chrType, sizeof(chrType));
    //Stream ID
    memcpy(&SendBuffer[1], &id, sizeof(id));
    //Package ID
    memcpy(&SendBuffer[6], &currentPacketId, sizeof(currentPacketId));
    //Rest of the data
    memcpy(&SendBuffer[7], &packetContent, sizeof(packetContent));

    if (isReliable) {
        std::cout << "Stream is reliable" << std::endl;
        previousData.insert(std::pair{currentPacketId, packetContent});

        //Reliability
    }

    socket.SendTo(endpointIp, endpointPort, std::span{SendBuffer.data(), static_cast<unsigned long>(std::strlen(SendBuffer.data()))});

    currentPacketId++;
}

void Stream::OnDataReceived(std::span<const char> Data) {
    std::cout << "Received message from client" << std::endl;

    char messageType;
    memcpy(&messageType, &Data[0], sizeof(messageType));
    uint8_t recId;
    memcpy(&recId, &Data[6], sizeof(recId));
    int packetContent;
    memcpy(&packetContent, &Data[7], sizeof(packetContent));

    std::cout << "Stream : " << std::to_string(id) << " packet ID : " << std::to_string(recId) << " message type : " << std::to_string(messageType) <<
            " Data : " << std::to_string(packetContent) << std::endl;

    if (receivedPackets.size() > 32)
        receivedPackets.erase(receivedPackets.begin());

    receivedPackets.push_back(recId);
    std::sort(receivedPackets.begin(), receivedPackets.end());

    SendAcknowledgment();
}

void Stream::OnAcknowledgedReceived(std::span<const char> Data) {
    char messageType;
    memcpy(&messageType, &Data[0], sizeof(messageType));
    uint8_t recId;
    memcpy(&recId, &Data[6], sizeof(recId));
    int packetContent;
    memcpy(&packetContent, &Data[7], sizeof(packetContent));
    std::array<uint8_t, 4> history;
    memcpy(&history, &Data[14], sizeof(history));

    std::cout << "Stream : " << std::to_string(id) << " packet ID : " << std::to_string(recId) << " message type : " << std::to_string(messageType) <<
            " Data : " << std::to_string(packetContent);
    // Convert history to bitset for easy visualization
    std::bitset<32> historyBits(
        (static_cast<uint32_t>(history[0]) << 24) |
        (static_cast<uint32_t>(history[1]) << 16) |
        (static_cast<uint32_t>(history[2]) << 8) |
        (static_cast<uint32_t>(history[3]))
    );

    std::cout << " History (Received Packets): " << historyBits << std::endl;

    if (receivedPackets.size() > 32)
        receivedPackets.erase(receivedPackets.begin());

    receivedPackets.push_back(recId);
    std::sort(receivedPackets.begin(), receivedPackets.end());

}

void Stream::SendAcknowledgment() {
    if (receivedPackets.empty()) return;

    uint8_t latestID = receivedPackets.back();
    std::array<char, 4> history{}; //History of the previous 32 messages

    for (uint8_t currentID: receivedPackets) {
        int bitIndex = latestID - currentID; //Distance from the latest
        if (bitIndex >= 0 && bitIndex < 32) {
            history[bitIndex / 8] |= (1 << (7 - (bitIndex % 8))); //Set the bit
        }
    }

    int packetContent = currentPacketId + 1000;
    std::array<char, 10> SendBuffer;
    char chrType = MessageType::STREAM_DATA_ACK & 0x0F;

    //Type of the message
    memcpy(&SendBuffer[0], &chrType, sizeof(chrType));
    //Stream ID
    memcpy(&SendBuffer[1], &id, sizeof(id));
    //Package ID
    memcpy(&SendBuffer[6], &currentPacketId, sizeof(currentPacketId));
    //History
    memcpy(&SendBuffer[7], history.data(), sizeof(history));

    socket.SendTo(endpointIp, endpointPort, std::span{SendBuffer.data(), SendBuffer.size()});

    currentPacketId++;
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
