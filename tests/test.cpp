#include <string>
#include <array>
#include <span>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "Server.h"
#include "Client.h"

#include "falcon.h"

TEST_CASE("Can Listen", "[falcon]") {
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    REQUIRE(receiver != nullptr);
}

TEST_CASE("Can Connect", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    REQUIRE(sender != nullptr);
}

TEST_CASE("Can Send To", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    std::string message = "Hello World!";
    std::span data(message.data(), message.size());
    int bytes_sent = sender->SendTo("127.0.0.1", 5555, data);
    REQUIRE(bytes_sent == message.size());
}

TEST_CASE("Can Receive From", "[falcon]") {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    std::string message = "Hello World!";
    std::span data(message.data(), message.size());
    int bytes_sent = sender->SendTo("127.0.0.1", 5555, data);
    REQUIRE(bytes_sent == message.size());
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    int byte_received = receiver->ReceiveFrom(from_ip, buffer);

    REQUIRE(byte_received == message.size());
    REQUIRE(std::equal(buffer.begin(),
        buffer.begin() + byte_received,
        message.begin(),
        message.end()));
    REQUIRE(from_ip == "127.0.0.1:5556");
}

TEST_CASE("Connection", "[falcon]") {
    Server server = Server();

    REQUIRE(server.knownUsers.size() == 0); //No user initially connected

    Client client = Client();
    client.ConnectToServer();
    server.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();
        client.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();

    // Check if the user was successfully registered
    REQUIRE(userRegistered == true);
    // Extract the UUID for validation
    REQUIRE(client.CurrentUUID != -1);


    std::this_thread::sleep_for(std::chrono::seconds(1)); //Let the ping time pass
    client.PingServer();

    auto ping = client.lastPing_ack;
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client.Update();
        server.Update();
        if (client.lastPing_ack != ping) break;
    }

    auto newPing = client.lastPing_ack;

    REQUIRE(ping != newPing);
}

TEST_CASE("Server Times Out", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();
        client.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    // Check if the user was successfully registered
    REQUIRE(client.IsConnected == true);

    // Measure elapsed time to ensure at least 11 seconds of inactivity
    //Have to do this that way so MacOS can synch correctly
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(12)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small sleep to avoid busy-waiting
    }

    client.Update();
    client.Update();

    // Check if the user was timed-out
    REQUIRE(client.IsConnected == false);
}

TEST_CASE("Client Times Out", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    client.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    auto clientUUID = client.CurrentUUID;

    bool userFound = false;
    for (const auto &user: server.knownUsers) {
        if (user.UUID == clientUUID) {
            userFound = true;
            break;
        }
    }
    // Check if the user was successfully registered
    REQUIRE(userFound == true);
    server.Update();
    // Measure elapsed time to ensure at least 11 seconds of inactivity
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(11)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small sleep to avoid busy-waiting
    }
    server.Update();
    server.Update();

    userFound = false;
    for (const auto &user: server.knownUsers) {
        if (user.UUID == client.CurrentUUID) {
            userFound = true;
            break;
        }
    }

    // Check if the user was timed-out
    REQUIRE(userFound == false);
}


TEST_CASE("Can Create Stream - Client", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();
        client.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    client.CreateStream();
    server.Update();
    client.Update();
    server.Update();

    // Retry loop to ensure streams are populated
    for (int i = 0; i < 20 && (client.falcon->existingStream.empty() || server.falcon->existingStream.empty()); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client.Update();
        server.Update();
    }

    REQUIRE(client.falcon->existingStream.size() > 0);
    REQUIRE(server.falcon->existingStream.size() > 0);

    if (client.falcon->existingStream.empty()) {
        FAIL("client.falcon->existingStream is unexpectedly empty");
        return;
    }
    if (server.falcon->existingStream.empty()) {
        FAIL("server.falcon->existingStream is unexpectedly empty");
        return;
    }

    auto clientIt = client.falcon->existingStream.begin();
    auto serverIt = std::prev(server.falcon->existingStream.end());

    if (!clientIt->second || !serverIt->second) {
        FAIL("clientIt or serverIt is nullptr");
        return;
    }

    REQUIRE(clientIt->second->id == serverIt->second->id);
}

TEST_CASE("Can Create Stream - Server", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    client.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    server.CreateStream(client.CurrentUUID, false);

    client.Update();
    server.Update();
    client.Update();

    // Retry loop to ensure streams are populated
    for (int i = 0; i < 20 && (client.falcon->existingStream.empty() || server.falcon->existingStream.empty()); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.Update();
        client.Update();
    }

    REQUIRE(client.falcon->existingStream.size() > 0);
    REQUIRE(server.falcon->existingStream.size() > 0);

    if (client.falcon->existingStream.empty()) {
        FAIL("client.falcon->existingStream is unexpectedly empty");
        return;
    }
    if (server.falcon->existingStream.empty()) {
        FAIL("server.falcon->existingStream is unexpectedly empty");
        return;
    }

    auto clientIt = client.falcon->existingStream.begin();
    auto serverIt = std::prev(server.falcon->existingStream.end());

    if (!clientIt->second || !serverIt->second) {
        FAIL("clientIt or serverIt is nullptr");
        return;
    }

    REQUIRE(clientIt->second->id == serverIt->second->id);
}


TEST_CASE("Can Send Data Through Stream", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    client.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    client.CreateStream();

    server.Update();
    client.Update();
    server.Update();

    // Retry loop to ensure streams are populated
    for (int i = 0; i < 15 && (client.falcon->existingStream.empty() || server.falcon->existingStream.empty()); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.Update();
        client.Update();
    }


    client.GenerateAndSendData();
    server.Update();
    client.Update();

    REQUIRE(client.falcon->existingStream.size() > 0);
    REQUIRE(server.falcon->existingStream.size() > 0);

    if (client.falcon->existingStream.empty()) {
        FAIL("client.falcon->existingStream is unexpectedly empty");
        return;
    }
    if (server.falcon->existingStream.empty()) {
        FAIL("server.falcon->existingStream is unexpectedly empty");
        return;
    }

    auto clientIt = client.falcon->existingStream.begin();
    auto serverIt = std::prev(server.falcon->existingStream.end());

    if (!clientIt->second || !serverIt->second) {
        FAIL("clientIt or serverIt is nullptr");
        return;
    }

    // Retry loop to ensure the message is sent
    for (int i = 0; i < 15 && (clientIt->second->previousData.empty() || serverIt->second->receivedPackets.empty()); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.Update();
        client.Update();
    }

    auto clientLastDataSent = std::prev(clientIt->second->previousData.end());
    auto serverLastDataReceived = serverIt->second->receivedPackets;

    if (serverLastDataReceived.size() == 0) {
        FAIL("serverLastDataReceived is invalid");
        return;
    }

    REQUIRE(clientLastDataSent->first == serverLastDataReceived[0]);
}

TEST_CASE("Stream Reliability", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    client.Update();

    // Retry loop to wait until the server registers the user
    bool userRegistered = false;
    for (int i = 0; i < 20 && !userRegistered; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Increase sleep time if needed
        server.Update();

        // Check if the user has been registered by the server
        for (const auto &user: server.knownUsers) {
            if (user.UUID == client.CurrentUUID) {
                userRegistered = true;
                break;
            }
        }
    }
    client.Update();
    server.Update();
    server.Update();

    client.CreateStream();

    server.Update();
    client.Update();
    server.Update();

    // Retry loop to ensure streams are populated
    for (int i = 0; i < 15 && (client.falcon->existingStream.empty() || server.falcon->existingStream.empty()); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.Update();
        client.Update();
    }

    auto clientIt = client.falcon->existingStream.begin();
    auto serverIt = std::prev(server.falcon->existingStream.end());
    if (!clientIt->second || !serverIt->second) {
        FAIL("clientIt or serverIt is nullptr");
        return;
    }


    for (int i = 0; i < 5; i++) {
        client.GenerateAndSendData();
        client.Update();
    }

    std::array<char, 32> SendBuffer;
    char chrType = MessageType::STREAM_DATA_ACK & 0x0F;

    //Type of the message
    memcpy(&SendBuffer[0], &chrType, sizeof(chrType));
    //Stream ID
    memcpy(&SendBuffer[1], &serverIt->second->id, sizeof(serverIt->second->id));

    //Package ID
    uint8_t packetID = 5;
    memcpy(&SendBuffer[6], &packetID, sizeof(packetID));
    //History
    std::array<char, 4> history{};
    history[0] = 0x98;  // 1001 1000 in binary
    history[1] = 0x00;  // 0000 0000 in binary
    history[2] = 0x00;  // 0000 0000 in binary
    history[3] = 0x00;  // 0000 0000 in binary
    memcpy(&SendBuffer[7], &history, sizeof(history));

    std::string ip = "127.0.0.1";
    auto port = 5555;
    for (auto user : server.knownUsers) {
        if (user.UUID == client.CurrentUUID) {
            ip = user.address;
            port = user.port;
        }
    }
    server.falcon->SendTo(ip, port, std::span{SendBuffer.data(), SendBuffer.size()});

    client.Update();
    client.Update();client.Update();client.Update();client.Update();
    server.Update();server.Update(); server.Update();
    server.Update();server.Update(); server.Update();

    auto latestPackage = std::prev(serverIt->second->amountOfPacketsInData.end());

    REQUIRE(latestPackage->second == 3);
}
