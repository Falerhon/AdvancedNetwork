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
//Client
uint32_t Falcon::CreateStream(bool reliable) {
    std::unique_ptr<Stream> stream = Stream::CreateStream(reliable, this);
    uint32_t idCreated = stream->id;
    existingStream.insert(std::pair{stream->id, std::move(stream)});
    return idCreated;
}
//Server
uint32_t Falcon::CreateStream(bool reliable, uint64_t clientId, std::string endpIp, int endpPort) {
    std::unique_ptr<Stream> stream = Stream::CreateStream(clientId, endpIp, endpPort , reliable, this);
    uint32_t idCreated = stream->id;
    existingStream.insert(std::pair{stream->id, std::move(stream)});
    return idCreated;
}
//Client from external
uint32_t Falcon::CreateStreamFromExternal(uint32_t id, bool reliable) {
    std::unique_ptr<Stream> stream = Stream::CreateStreamExternal(id, reliable, this);
    uint32_t idCreated = stream->id;
    existingStream.insert(std::pair{stream->id, std::move(stream)});
    return idCreated;
}
//Server from external
uint32_t Falcon::CreateStreamFromExternal(uint32_t id, uint64_t clientId, std::string endpIp, int endpPort, bool reliable) {
    std::unique_ptr<Stream> stream = Stream::CreateStreamExternal(id, clientId, endpIp, endpPort , reliable, this);
    uint32_t idCreated = stream->id;
    existingStream.insert(std::pair{stream->id, std::move(stream)});
    return idCreated;
}
