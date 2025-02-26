#include <string>
#include <array>
#include <span>

#include <catch2/catch_test_macros.hpp>

#include "falcon.h"

TEST_CASE( "Can Listen", "[falcon]" ) {
    auto receiver = Falcon::Listen("127.0.0.1", 5555);
    REQUIRE(receiver != nullptr);
}

TEST_CASE( "Can Connect", "[falcon]" ) {
    auto sender = Falcon::Connect("127.0.0.1", 5556);
    REQUIRE(sender != nullptr);
}

TEST_CASE( "Can Send To", "[falcon]" ) {
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

TEST_CASE("On Connection - Server", "[falcon]") {

}

TEST_CASE("On Connection - Client", "[falcon]") {

}

TEST_CASE("On Client Disconnect", "[falcon]") {

}

TEST_CASE("Can Create Stream - Server", "[falcon]") {

}

TEST_CASE("Can Create Stream - Client", "[falcon]") {

}

TEST_CASE("Can Create Stream From External - Server", "[falcon]") {

}

TEST_CASE("Can Create Stream From External - Client", "[falcon]") {

}

TEST_CASE("Can Send Data Through Stream", "[falcon]") {

}

TEST_CASE("Can Receive Data From Stream", "[falcon]") {

}

TEST_CASE("Stream Reliability", "[falcon]") {

}