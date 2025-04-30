#include <cstdint>
#include <cstring>
#include <iostream>
#include "enet6/enet.h"
#include "../../src/CubeGame.h"

constexpr uint16_t SERVER_PORT = 1234;

int main() {
    ENetAddress address;
    ENetHost *client;
    ENetPeer *server;
    char addressBuffer[ENET_ADDRESS_MAX_LENGTH];

    if (enet_initialize() != 0) {
        std::cerr << "ENet failed to An error occurred while initializing ENet! - Client" << std::endl;
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize);

    enet_address_set_host(&address, ENET_ADDRESS_TYPE_ANY, "localhost");
    address.port = SERVER_PORT;

    enet_address_get_host_ip(&address, addressBuffer, ENET_ADDRESS_MAX_LENGTH);
    std::cout << "Connecting to server at " << addressBuffer << std::endl;

    client = enet_host_create(address.type,
                              NULL /* No address since not a listen host */,
                              1 /* allow up to 1 outgoing connections */,
                              2 /* allow up to 2 channels to be used, 0 and 1 */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);
    if (client == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    server = enet_host_connect(client, &address, 2, 0);
    if (server == NULL) {
        fprintf(stderr, "No server found for Enet connection on client.\n");
        exit(EXIT_FAILURE);
    }

    int argc = 1;
    char arg0[] = "CubeGameApp";
    char* argv[] = { arg0 };

    CubeGame game(Platform::GlfwApplication::Arguments{argc, argv});
    game.Init(client);
    return game.exec();
}
