#include <cstdint>
#include <iostream>

#include "../../src/CubeGame.h"
#include "enet6/enet.h"

constexpr uint16_t SERVER_PORT = 1234;
constexpr size_t MAX_CLIENTS = 4;

int main() {
    ENetAddress address;
    ENetHost *server;


    //Initialize Enet
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet. - Server\n");
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize); //Call deinitialize when exiting

    //Listen address
    enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
    address.port = SERVER_PORT;

    server = enet_host_create(ENET_ADDRESS_TYPE_ANY,
                              &address /* the address to bind the server host to */,
                              MAX_CLIENTS /* allow up to 4 clients and/or outgoing connections */,
                              2 /* allow up to 2 channels to be used, 0 and 1 */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);

    if (server == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }


    std::cout << "Server started on port " << SERVER_PORT << std::endl;

    int argc = 1;
    char arg0[] = "CubeGameApp";
    char *argv[] = {arg0};

    CubeGame game(Platform::GlfwApplication::Arguments{argc, argv});
    game.Init(server);
    return game.exec();
}
