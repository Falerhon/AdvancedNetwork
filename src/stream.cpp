//
// Created by alex- on 2025-02-18.
//

#include "stream.h"

#include <iostream>
#include <random>
//Server
std::unique_ptr<Stream> Stream::CreateStream(const uint64_t client, const bool reliable) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = GenerateId();
    stream->linkedClient = client;

    return stream;
}
//Client
std::unique_ptr<Stream> Stream::CreateStream(const bool reliable, const uint32_t id) {
    auto stream = std::make_unique<Stream>();
    stream->isReliable = reliable;
    stream->id = id;


    return stream;
}

void Stream::CloseStream(const Stream &stream) {
    //TODO : TELL THE CLIENT THIS STREAM IS CLOSING
}

void Stream::SendData(std::span<const char> Data) {
    std::cout << "Sending message to linked client" << std::endl;
    if (isReliable) {
        std::cout << "Stream is reliable" << std::endl;
    }
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
