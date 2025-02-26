//
// Created by alex- on 2025-02-18.
//

#include "stream.h"

#include <bitset>
#include <iostream>
#include <random>
#include <cstring>
#include <algorithm>
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
    size_t offset = 0;
    //Type of the message
    memcpy(&SendBuffer[offset], &chrType, sizeof(chrType));
    offset += sizeof(chrType);

    //Stream ID
    memcpy(&SendBuffer[offset], &id, sizeof(id));
    offset += sizeof(id);

    //Amount of package to read
    int numPackage = 1;
    memcpy(&SendBuffer[offset], &numPackage, sizeof(numPackage));
    offset += sizeof(numPackage);

    //Packet Size
    size_t packetSize = sizeof(currentPacketId) + sizeof(packetContent);
    memcpy(&SendBuffer[offset], &packetSize, sizeof(packetSize));
    offset += sizeof(packetSize);

    //Package ID
    memcpy(&SendBuffer[offset], &currentPacketId, sizeof(currentPacketId));
    offset += sizeof(currentPacketId);

    //Rest of the data
    memcpy(&SendBuffer[offset], &packetContent, sizeof(packetContent));
    offset += sizeof(packetContent);
    if (isReliable) {
        previousData.insert(std::pair{currentPacketId, packetContent});
    }

    socket.SendTo(endpointIp, endpointPort, std::span{SendBuffer.data(), offset});
    currentPacketId++;
}

void Stream::OnDataReceived(std::span<const char> Data) {
    std::cout << "Received message from client" << std::endl;

    size_t offset = 0;

    //Message Type
    char chrType;
    memcpy(&chrType, &Data[offset], sizeof(chrType));
    offset += sizeof(chrType);

    //StreamID
    uint32_t streamID;
    memcpy(&streamID, &Data[offset], sizeof(streamID));
    offset += sizeof(streamID);

    //Number of packets to read
    int numOfPackets;
    memcpy(&numOfPackets, &Data[offset], sizeof(numOfPackets));
    offset += sizeof(numOfPackets);

    for (int i = 0; i < numOfPackets; i++) {
        size_t packetSize = sizeof(uint8_t) + sizeof(int);
        offset += sizeof(packetSize);

        //Packet ID
        uint8_t packetID;
        memcpy(&packetID, &Data[offset], sizeof(packetID));
        offset += sizeof(packetID);

        if (i  == 0) {
            amountOfPacketsInData[packetID] = numOfPackets;
        }

        //Packet Data
        int packetData;
        memcpy(&packetData, &Data[offset], sizeof(packetData));
        offset += sizeof(packetData);

        std::cout << "Message type : " << std::to_string(chrType) << " stream : " << std::to_string(streamID) << " num of packets : " <<
                std::to_string(numOfPackets) << " packet size : " << std::to_string(packetSize) << " packet ID : " <<
                std::to_string(packetID) << " data : " << std::to_string(packetData) << std::endl;

        //Add the packets to the receivedPackets
        if (receivedPackets.size() > 32)
            receivedPackets.erase(receivedPackets.begin());

        if (std::find(receivedPackets.begin(), receivedPackets.end(), packetID) == receivedPackets.end()) {
            receivedPackets.push_back(packetID);
        }

    }

    std::sort(receivedPackets.begin(), receivedPackets.end());

    SendAcknowledgment();
}

void Stream::OnAcknowledgedReceived(std::span<const char> Data) {
    char messageType;
    memcpy(&messageType, &Data[0], sizeof(messageType));

    uint32_t streamID;
    memcpy(&streamID, &Data[1], sizeof(streamID));

    uint8_t recId;
    memcpy(&recId, &Data[6], sizeof(recId));

    std::array<uint8_t, 4> history;
    memcpy(&history, &Data[7], history.size());


    // Convert history to bitset for easy visualization
    std::bitset<32> historyBits(
        (static_cast<uint32_t>(history[0]) << 24) |
        (static_cast<uint32_t>(history[1]) << 16) |
        (static_cast<uint32_t>(history[2]) << 8) |
        (static_cast<uint32_t>(history[3]))
    );

    std::cout << "MsgType : " << std::to_string(messageType) << " streamID : " << std::to_string(id) << " packet ID : " << std::to_string(recId) <<
            " History (Received Packets): " << historyBits << std::endl;

    if (isReliable)
        HandleReliability(recId, history);
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

    std::array<char, 32> SendBuffer;
    char chrType = MessageType::STREAM_DATA_ACK & 0x0F;

    //Type of the message
    memcpy(&SendBuffer[0], &chrType, sizeof(chrType));
    //Stream ID
    memcpy(&SendBuffer[1], &id, sizeof(id));
    //Package ID
    memcpy(&SendBuffer[6], &currentPacketId, sizeof(currentPacketId));
    //History
    memcpy(&SendBuffer[7], &history, sizeof(history));

    socket.SendTo(endpointIp, endpointPort, std::span{SendBuffer.data(), SendBuffer.size()});

    currentPacketId++;
}

void Stream::HandleReliability(uint8_t LatestID, std::array<uint8_t, 4> History) {
    std::vector<uint8_t> missingPackets;

    //Check for missing packets based on the history
    for (int i = 0; i < 32; i++) {
        int byteIndex = i / 8;
        int bitIndex = 7 - (i % 8); //Ensure the bits are read left to right

        //If the bit is 0, packet wasn't received
        if (!(History[byteIndex] & (1 << bitIndex))) {
            int packetID = LatestID - i;

            //Check if the packet was previously sent
            if (previousData.find(packetID) != previousData.end()) {
                missingPackets.push_back(packetID);
            }
        }
    }

    //Resend the packets
    if (missingPackets.empty()) return;

    std::cout << "Resending missing packets " << std::endl;
    std::array<char, 65535> Buffer;

    size_t offset = 0;

    //Type of the message
    char chrType = MessageType::STREAM_DATA & 0x0F;
    memcpy(&Buffer[offset], &chrType, sizeof(chrType));
    offset += sizeof(chrType);

    //StreamID
    memcpy(&Buffer[offset], &id, sizeof(id));
    offset += sizeof(id);

    //Number of packet sent
    int numOfPacket = missingPackets.size();
    memcpy(&Buffer[offset], &numOfPacket, sizeof(numOfPacket));
    offset += sizeof(numOfPacket);

    //Calculate the total size of the missing packets' data
    for (uint8_t packetID: missingPackets) {
        if (previousData.find(packetID) != previousData.end()) {
            int packetData = previousData[packetID];

            //Add the size of the packet (size of ID + PacketData)
            size_t packetSize = sizeof(uint8_t) + sizeof(int);
            memcpy(&Buffer[offset], &packetSize, sizeof(packetSize));
            offset += sizeof(packetSize);

            //Add the ID
            memcpy(&Buffer[offset], &packetID, sizeof(packetID));
            offset += sizeof(packetID);

            //Add the data
            memcpy(&Buffer[offset], &packetData, sizeof(packetData));
            offset += sizeof(packetData);
        }
    }

    //Send the packet
    socket.SendTo(endpointIp, endpointPort, std::span{Buffer.data(), offset});
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
