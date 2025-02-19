//
// Created by alex- on 2025-02-18.
//
#pragma once

#ifndef STREAM_H
#define STREAM_H

#include<string>
#include<memory>
#include<span>
#include <vector>

class Stream {
public:
    static std::unique_ptr<Stream> CreateStream(uint64_t client, bool reliable); //Server API

    static std::unique_ptr<Stream> CreateStream(bool reliable, uint32_t id); //Client API

    void CloseStream(const Stream& stream); //Server API

    void SendData(std::span<const char> Data);
    void OnDataReceived(std::span<const char> Data);

    uint32_t id;

    static uint32_t GenerateId();
    private:
    //Server's current packet ID
    //Client's last packet received
    uint32_t currentPacketId = 0;
    //Only needed on the server
    uint64_t linkedClient = 0;
    bool isReliable = false;
    std::vector<std::span<const char>> previousData;



};



#endif //STREAM_H
