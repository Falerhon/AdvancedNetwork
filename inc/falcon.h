#pragma once

#include <functional>
#include <memory>
#include <string>
#include <span>
#include <iostream>

#include "stream.h"

void hello();

#ifdef WIN32
    using SocketType = unsigned int;
#else
    using SocketType = int;
#endif

enum MessageType {
    CONNECT,
    CONNECT_ACK,
    DISCONNECT,
    DISCONNECT_ACK,
    PING,
    PING_ACK,
    STREAM_CREATE,
    STREAM_CREATE_ACK,
    STREAM_DATA,
    STREAM_DATA_ACK,
};

//Use this struct to ease the creation and reading of messages
struct Message {
    int MessType;
    std::array<char, 65535> Data;
    virtual void readBuffer (std::array<char, 65535>& buff) = 0;
    virtual void WriteBuffer (std::array<char, 65535>& buff) = 0;

    Message () {};
    virtual ~Message() = default;
};

#define TIMEOUTTIME 3
#define PINGTIME 1

class Falcon {
public:
    static std::unique_ptr<Falcon> Listen(const std::string& endpoint, uint16_t port);
    void OnClientConnected(std::function<void(uint64_t)> handler);
    static std::unique_ptr<Falcon> Connect(const std::string& serverIp, uint16_t port);
    void OnConnectionEvent(std::function<void(bool, uint64_t)> handler);

    void OnClientDisconnected(std::function<void(uint64_t)> handler); //Server API
    void OnDisconnect(std::function<void()> handler); //Client API

    void SetBlocking(bool block);

    Falcon();
    ~Falcon();
    Falcon(const Falcon&) = default;
    Falcon& operator=(const Falcon&) = default;
    Falcon(Falcon&&) = default;
    Falcon& operator=(Falcon&&) = default;

    int SendTo(const std::string& to, uint16_t port, std::span<const char> message);
    int ReceiveFrom(std::string& from, std::span<char, 65535> message);

    std::function<void(uint64_t)> serverConnectionHandler;
    std::function<void(bool, uint64_t)> clientConnectionHandler;
    std::function<void()> serverDisconnectionHandler;
    std::function<void(uint64_t)> clientDisconnectionHandler;

    //Client stream creation
    uint32_t CreateStream(bool reliable);
    //Server stream creation
    uint32_t CreateStream(bool reliable, uint64_t clientId, std::string endpIp, int endpPort);
    //Client stream creation from external request
    uint32_t CreateStreamFromExternal(uint32_t id, bool reliable);
    //Server stream creation from external request
    uint32_t CreateStreamFromExternal(uint32_t id, uint64_t clientId, std::string endpIp, int endpPort, bool reliable);

    void HandleStreamData(std::span<char, 65535> message);
    void SendStreamData(std::span<char, 65535> message, uint32_t streamId);

private:
    int SendToInternal(const std::string& to, uint16_t port, std::span<const char> message);
    int ReceiveFromInternal(std::string& from, std::span<char, 65535> message);

    //Streams existing on this socket
    std::map<uint32_t, std::unique_ptr<Stream>> existingStream;


    SocketType m_socket;
};
