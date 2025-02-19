#include <iostream>

#include <falcon.h>

#include "stream.h"
#include "spdlog/spdlog.h"

uint64_t CurrentUUID = -1;

void ComposeMessage(MessageType type, std::string& message);
void ConnectionEvent(bool success, uint64_t uuid);
void Disconnection();

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> lastPing_ack = std::chrono::high_resolution_clock::now();

    //Streams existing on this instance
    std::map<uint32_t, std::unique_ptr<Stream>> existingStream;

    bool doOnce = false;

    auto falcon = Falcon::Connect("127.0.0.1", 5556);
    falcon->SetBlocking(false);
    falcon->OnConnectionEvent(ConnectionEvent);
    falcon->OnDisconnect(Disconnection);
    //Sending a connection request
    std::string message;
    ComposeMessage(CONNECT, message);
    std::span data(message.data(), message.size());
    falcon->SendTo("127.0.0.1", 5555, data);

    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;

    while (true) {
        //Clearing the buffer
        buffer = {{}};

        falcon->ReceiveFrom(from_ip, buffer);

        if (!buffer[0] == '\0') {
            std::string strid = buffer.data();

            //Get the message type
            int messageType = -1;
            //ASCII to numeral
            messageType = strid.front() - 48;
            strid.erase(strid.begin());

            switch (messageType) {
                //Connection
                case 0:

                    break;
                //Connectio_ACK
                case 1: {
                    uint64_t FoundUUID = -1;
                    bool success = false;
                    try {
                        success = (int)strid.front() - 48;
                    }catch(const std::invalid_argument& e){}
                    strid.erase(strid.begin());

                    if (success) {
                        FoundUUID = std::stoull(strid);
                    } else {
                        FoundUUID = -1;
                    }

                    falcon->clientConnectionHandler(success, FoundUUID);

                    message.clear();
                    message = "Connected!";
                    ComposeMessage(CONNECT_ACK, message);
                    falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});

                }
                break;
                //Disconnection
                case 2:
                    message.clear();
                    ComposeMessage(DISCONNECT_ACK, message);
                    falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});

                std::cout << "Server sent disconnection" << std::endl;
                    falcon->serverDisconnectionHandler();
                break;
                //Disconnection_ACK
                case 3:
                    std::cout << "Server acknowledged the disconnection" << std::endl;
                    falcon->serverDisconnectionHandler();
                break;
                //Ping
                case 4:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //Ping_ACK
                case 5:
                    std::cout << "Ping acknowledged" << std::endl;
                    lastPing_ack = std::chrono::high_resolution_clock::now();
                break;
                //StreamCreate
                case 6: {
                    std::cout << "Server created a stream" << std::endl;

                    std::string dataStr = buffer.data();

                    uint32_t FoundStreamId = -1;
                    bool success = false;
                    try {
                        success = (int)dataStr.front() - 48;
                    }catch(const std::invalid_argument& e){}
                    dataStr.erase(dataStr.begin());

                    if (success) {
                        FoundStreamId = std::stoull(dataStr);
                    } else {
                        std::cout << "Could not find stream ID" << std::endl;
                        break;
                    }

                    auto stream = Stream::CreateStream(false, FoundStreamId);
                    auto streamId = stream->id;
                    //Adding the stream
                    existingStream.insert(std::pair(stream->id, std::move(stream)));
                    //Preparing the stream created message
                    message.clear();
                    message = std::to_string(streamId);
                    ComposeMessage(STREAM_CREATE_ACK, message);
                    falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});
                    break;
                }
                //StreamCreate_ACK
                case 7:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //StreamData
                case 8:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                //StreamData_ACK
                case 9:
                    std::cout << "Not implemented yet" << std::endl;
                break;
                default:
                    std::cout << "Unknown message type " << messageType << std::endl;
                break;
            }
        }

        if (std::chrono::high_resolution_clock::now() - lastPing > std::chrono::seconds(PINGTIME)) {
            std::cout << "Sending ping to server" << std::endl;
            std::string ping;
            ComposeMessage(PING, ping);
            falcon->SendTo("127.0.0.1", 5555, std::span {ping.data(), static_cast<unsigned long>(ping.length())});
            lastPing = std::chrono::high_resolution_clock::now();

            if (doOnce) {
                doOnce = false;
                message.clear();
                //Ask the server to create the stream
                ComposeMessage(STREAM_CREATE, message);
                falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});
            }
        }

        if (std::chrono::high_resolution_clock::now() - lastPing_ack > std::chrono::seconds(TIMEOUTTIME)) {
            std::cout << "Server has timed out" << std::endl;

            //Telling the server we are disconnecting
            //Most likely won't reach the server
            message.clear();
            ComposeMessage(DISCONNECT, message);
            falcon->SendTo("127.0.0.1", 5555, std::span {message.data(), static_cast<unsigned long>(message.length())});

            break;
        }
    }


    return EXIT_SUCCESS;
}

void ComposeMessage(MessageType type, std::string& message) {
    message = std::to_string(type) + std::to_string(CurrentUUID) + message;
}

void ConnectionEvent(bool success, uint64_t uuid) {
    if (success) {
        CurrentUUID = uuid;
        std::cout << "Connection successful with UUID : " << CurrentUUID << std::endl;
    } else {
        std::cout << "Connection failed" << std::endl;
    }
}

void Disconnection() {
    std::cout << "Lost connection with the server" << std::endl;
    exit(0);
}