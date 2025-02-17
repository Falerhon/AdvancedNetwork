#include "falcon.h"

int Falcon::SendTo(const std::string &to, uint16_t port, const std::span<const char> message)
{
    return SendToInternal(to, port, message);
}

int Falcon::ReceiveFrom(std::string& from, const std::span<char, 65535> message)
{
    return ReceiveFromInternal(from, message);
}

void Falcon::OnClientConnected(std::function<void(uint64_t)> handler) {
    serverConnectionHandler = std::move(handler);
}

void Falcon::OnConnectionEvent(std::function<void(bool, uint64_t)> handler) {
    clientConnectionHandler = std::move(handler);
}

void Falcon::OnClientDisconnected(std::function<void(uint64_t)> handler) {
    clientDisconnectionHandler = std::move(handler);
}

void Falcon::OnDisconnect(std::function<void()> handler) {
    serverDisconnectionHandler = std::move(handler);
}