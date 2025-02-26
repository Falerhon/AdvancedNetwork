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

    // Allow some time for the connection to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

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
    server.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.Update();
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

    std::this_thread::sleep_for(std::chrono::seconds(11));
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

    std::this_thread::sleep_for(std::chrono::seconds(11));
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
}

TEST_CASE("Can Send Data Through Stream", "[falcon]") {
}

TEST_CASE("Can Receive Data From Stream", "[falcon]") {
}

TEST_CASE("Stream Reliability", "[falcon]") {
}
