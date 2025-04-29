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
    ENetEvent event;
    char buffer[1024];
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

    // while (true) {
    //     if (enet_host_service(client, &event, 1000) > 0) {
    //         switch (event.type) {
    //             case ENET_EVENT_TYPE_CONNECT:
    //                 enet_address_get_host_ip(&event.peer->address, addressBuffer, ENET_ADDRESS_MAX_LENGTH);
    //                 std::cout << "Client connected to server at address " << addressBuffer << std::endl;
    //                 break;
    //
    //             case ENET_EVENT_TYPE_RECEIVE:
    //                 std::cout << "Client received data from server : " << event.packet->data << std::endl;
    //                 enet_packet_destroy(event.packet);
    //                 break;
    //
    //             case ENET_EVENT_TYPE_DISCONNECT:
    //                 std::cout << "Client disconnected from server" << std::endl;
    //                 break;
    //
    //             case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
    //                 std::cout << "Client timed out" << std::endl;
    //                 break;
    //             default:
    //                 break;
    //         }
    //     }
    //     else if (server->state == ENET_PEER_STATE_CONNECTED) //Quick ping to avoid timeout
    //     {
    //
    //         std::string packetMsg = "packet";
    //         if (strlen(buffer) > 0 && strcmp(buffer, "\n") != 0)
    //         {
    //             /* Build a packet passing our bytes, length and flags (reliable means this packet will be resent if lost) */
    //             ENetPacket* packet = enet_packet_create(packetMsg.data(), strlen(packetMsg.data()) + 1, ENET_PACKET_FLAG_RELIABLE);
    //             /* Send the packet to the server on channel 0 */
    //             enet_peer_send(server, 0, packet);
    //         }
    //         else
    //         {
    //             enet_peer_disconnect_now(server, 0);
    //         }
    //     }
    // }
    //
    // game.Shutdown();
    // return EXIT_SUCCESS;
}
