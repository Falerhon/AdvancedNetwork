#include <chrono>
#include <cstdint>
#include <iostream>
#include "enet6/enet.h"

constexpr uint16_t SERVER_PORT = 1234;
constexpr size_t MAX_CLIENTS = 4;

int main() {
    ENetAddress address;
    ENetHost *server;
    ENetEvent event;
    int eventStatus;
    char addressBuffer[ENET_ADDRESS_MAX_LENGTH];


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

    while (true) {
        if (enet_host_service(server, &event, 100) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    enet_address_get_host_ip(&event.peer->address, addressBuffer, ENET_ADDRESS_MAX_LENGTH);
                    std::cout << "Client connected from " << addressBuffer << std::endl;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Received packet of size "
                            << event.packet->dataLength
                            << " on channel " << (int) event.channelID
                            << std::endl;

                    // Echo back the received packet
                    enet_host_broadcast(server, 0, event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    enet_address_get_host_ip(&event.peer->address, addressBuffer, ENET_ADDRESS_MAX_LENGTH);
                    std::cout << "Client" << addressBuffer << " disconnected." << std::endl;
                    break;
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    enet_address_get_host_ip(&event.peer->address, addressBuffer, ENET_ADDRESS_MAX_LENGTH);
                    std::cout << "Client" << addressBuffer << " timed out." << std::endl;
                default:
                    break;
            }
        }
    }
    return EXIT_SUCCESS;
}
