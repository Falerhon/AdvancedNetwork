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

    // Wait until the server registers the user
    for (int i = 0; i < 10 && server.knownUsers.size() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.Update();
    }

    client.Update();
    server.Update();
    server.Update();

    // Extract the UUID for validation
    REQUIRE(client.CurrentUUID != -1);

    // Verify that the server registered the user by checking known users
    bool userFound = false;
    for (const auto &user: server.knownUsers) {
        if (user.UUID == client.CurrentUUID) {
            userFound = true;
            break;
        }
    }

    // Check if the user was successfully registered
    REQUIRE(userFound == true);


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
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();

    // Check if the user was successfully registered
    REQUIRE(client.IsConnected == true);

    // Measure elapsed time to ensure at least 11 seconds of inactivity
    //Have to do this that way so MacOS can synch correctly
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(11)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small sleep to avoid busy-waiting
    }

    client.Update();

    // Check if the user was timed-out
    REQUIRE(client.IsConnected == false);
}

TEST_CASE("Client Times Out", "[falcon]") {
    Server server = Server();
    Client client = Client();

    client.ConnectToServer();
    server.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();
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

    // Measure elapsed time to ensure at least 11 seconds of inactivity
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(11)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small sleep to avoid busy-waiting
    }
    server.Update();

    userFound = false;
    for (const auto &user: server.knownUsers) {
        if (user.UUID == clientUUID) {
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
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();
    server.Update();

    client.CreateStream();
    server.Update();
    client.Update();

    REQUIRE(client.falcon->existingStream[0] == server.falcon->existingStream[1]);
}

TEST_CASE("Can Create Stream - Server", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();
    server.Update();

    server.CreateStream(false);

    server.Update();
    client.Update();

    if (server.falcon->existingStream.size() > 0)
        REQUIRE(client.falcon->existingStream[0] == server.falcon->existingStream[1]);
    else
        REQUIRE(client.falcon->existingStream[0] == server.falcon->existingStream[0]);
}

TEST_CASE("Can Send Data Through Stream", "[falcon]") {
    Server server = Server();
    Client client = Client();
    client.ConnectToServer();
    server.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();
    server.Update();

    client.CreateStream();
    server.Update();
    client.Update();

    client.GenerateAndSendData();
    server.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
/*
    if (server.falcon->existingStream[0]->receivedPackets.size() > 0)
        server.falcon->existingStream[0]->receivedPackets.end();
    if (client.falcon->existingStream[0]->receivedPackets.size() > 0)
        client.falcon->existingStream[0]->receivedPackets.end();
*/
    //  REQUIRE();
}

TEST_CASE("Can Receive Data From Stream", "[falcon]") {
}

TEST_CASE("Stream Reliability", "[falcon]") {
}
