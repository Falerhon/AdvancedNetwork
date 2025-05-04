//
// Created by alex- on 2025-04-28.
//

#include "GameLogic.h"

#include <iostream>
#include <ostream>
#include <../../src/Network/APIHandler.h>
#include "../../src/Network/NetworkEvent.h"


enum class NetworkEventType : uint8_t;

void GameLogic::AddPlayer(int UUID) {
    userIds.push_back(UUID);
    cubesLeft.push_back(numbOfBoxesPerPlayers);
}

void GameLogic::CubeDestroyed(int index) {
    //Stop here if the cube is not owned by a player
    if (cubesLeft.size() - 1 < index || index == -1)
        return;

    cubesLeft[index]--;

    if (cubesLeft[index] == 0) {
        PlayerVictory(index);
    }
}

void GameLogic::PlayerVictory(int index) {
    std::cout << "Player : " << index << " Won!" << std::endl;

#ifdef  IS_SERVER

    for (int i = 0; i < userIds.size(); i++) {

        int id = userIds[i];

        nlohmann::json payload = id;
        cpr::Response response;
        if (i == index) {
            response = API->post("/api/Stats/win", payload);
        } else {
            response = API->post("/api/Stats/loss", payload);
        }

        if (response.status_code != 201 && response.status_code != 200) {
            std::cout << "Could not send data to the online server : " << response.status_code << std::endl;
        }

        nlohmann::json payloadCube = {
            {"PlayerId", id},
            {"Cubes",  numbOfBoxesPerPlayers - cubesLeft[i]},
        };

        auto responseCube = API->post("/api/Stats/destroy", payloadCube);

        if (responseCube.status_code != 201 && responseCube.status_code != 200) {
            std::cout << "Could not send data to the online server : " << responseCube.status_code << std::endl;
        }
    }

    nlohmann::json payloadServ = {};

    auto responseServ = API->post("/api/GameServer/free", payloadServ);

    if (responseServ.status_code != 201 && responseServ.status_code != 200) {
        std::cout << "Could not free the server : " << responseServ.status_code << std::endl;
    }

    //Send game over message
    char buffer[2048];
    size_t offset = 0;

    NetworkEventType packetType = NetworkEventType::ENDGAME;
    memcpy(buffer + offset, &packetType, sizeof(NetworkEventType));
    offset += sizeof(NetworkEventType);

    int winner = userIds[index];
    memcpy(buffer + offset, &winner, sizeof(int));
    offset += sizeof(winner);

    ENetPacket *packet = enet_packet_create(buffer, offset, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(host, 0, packet);
    enet_host_flush(host);

    exit(0);
#endif

    SetGameState(GameState::PostGame);
}
