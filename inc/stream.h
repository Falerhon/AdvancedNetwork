//
// Created by alex- on 2025-02-18.
//
#pragma once

#ifndef STREAM_H
#define STREAM_H

#include <functional>
#include <map>
#include <string>
#include<memory>
#include<span>

//Foward declare the falcon class
class Falcon;

class Stream {
public:
    explicit Stream(Falcon &socket)
        : socket(socket) {
    }

    static std::unique_ptr<Stream> CreateStream(uint64_t client, std::string endpIp, int endpPort, bool reliable, Falcon& socket); //Server API
    static std::unique_ptr<Stream> CreateStreamExternal(uint32_t id, uint64_t client, std::string endpIp, int endpPort, bool reliable, Falcon& socket); //Server API

    static std::unique_ptr<Stream> CreateStream(bool reliable, Falcon& socket); //Client API
    static std::unique_ptr<Stream> CreateStreamExternal(uint32_t id, bool reliable, Falcon& socket); //Client API

    void CloseStream(const Stream& stream); //Server API

    void SendData(std::span<const char> Data);
    void OnDataReceived(std::span<const char> Data);
    void OnAcknowledgedReceived(std::span<const char> Data);

    void SendAcknowledgment();

    uint32_t id;

    static uint32_t GenerateId();
private:
    //Sender's current packet ID
    uint8_t currentPacketId = 0;
    //Only needed on the server
    uint64_t linkedClient = 0;
    std::string endpointIp  = "127.0.0.1";
    int endpointPort = 5555;
    bool isReliable = false;
    //Past data so we can resend them
    std::map<int, int> previousData;

    //History of the 32 last packets received
    std::vector<uint8_t> receivedPackets;

    //Socket this stream is linked to
    Falcon& socket;

};



#endif //STREAM_H
