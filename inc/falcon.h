#pragma once

#include <functional>
#include <memory>
#include <string>
#include <span>

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


private:
    int SendToInternal(const std::string& to, uint16_t port, std::span<const char> message);
    int ReceiveFromInternal(std::string& from, std::span<char, 65535> message);



    SocketType m_socket;
};
