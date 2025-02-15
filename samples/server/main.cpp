#include <iostream>

#include <falcon.h>
#include <random>

#include "spdlog/spdlog.h"

//Struct used to hold the user info
struct User { // This structure is named "myDataType"
    uint64_t UUID;
    std::string address;
    User(const uint64_t id, const std::string& add):UUID(id),address(add) {};
    bool operator==(uint64_t id) const {return UUID == id;}
};
//List of known users
std::vector<User> knownUsers;

uint64_t RegisterUser(std::string& address);
void ConnectionConfirmation(uint64_t UUID);

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Hello World!");

    auto falcon = Falcon::Listen("127.0.0.1", 5555);
    std::string from_ip;
    from_ip.resize(255);
    std::array<char, 65535> buffer;
    int recv_size = falcon->ReceiveFrom(from_ip, buffer);
    std::string ip = from_ip;
    uint16_t port = 0;
    auto pos = from_ip.find_last_of (':');
    if (pos != std::string::npos) {
        ip = from_ip.substr (0,pos);
        std::string port_str = from_ip.substr (++pos);
        port = atoi(port_str.c_str());
    }

    //Convert char to uint64_t
    uint64_t SearchedUUID = 0;
    for( int i = 7; i >= 0; --i )
    {
        SearchedUUID <<= 8;
        SearchedUUID |= (uint64_t)buffer[i];
    }

    //Check if user is known
    if (std::find(knownUsers.begin(), knownUsers.end(), SearchedUUID) == knownUsers.end()) {
        SearchedUUID = RegisterUser(ip);

        std::cout << "New user id : " << SearchedUUID << std::endl;
        //TODO : Send message containing new UUID
    }


    std::string message = std::to_string(SearchedUUID) + " is your new ID";
    falcon->SendTo(ip, port, std::span {message.data(), static_cast<unsigned long>(message.length())});
    return EXIT_SUCCESS;
}

uint64_t RegisterUser(std::string& address) {
    //Generate a random identifier
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<unsigned long long> dis(
        std::numeric_limits<std::uint64_t>::min(),
        std::numeric_limits<std::uint64_t>::max()
    );

    uint64_t UUID = dis(gen);
    //Add user to known user list
    knownUsers.emplace_back(UUID, address);
    return UUID;
}

void ConnectionConfirmation(uint64_t UUID) {

}