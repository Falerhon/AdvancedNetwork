//
// Created by alex- on 2025-04-28.
//

#include "GameLogic.h"

#include <iostream>
#include <ostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <../../src/Network/APIHandler.h>

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

        nlohmann::json payload = {
            {"id", userIds[i]},
        };

        auto response = API->post("/api/Stats/win", payload);

        if (response.status_code != 201 && response.status_code != 200) {
            std::cout << "Could not send data to the online server : " << response.status_code << std::endl;
            continue;
        }

        nlohmann::json payloadCube = {
            {"id", userIds[i]},
            {"count", cubesLeft[i]},
        };

        auto responseCube = API->post("/api/Stats/destroy", payloadCube);

        if (responseCube.status_code != 201 && responseCube.status_code != 200) {
            std::cout << "Could not send data to the online server : " << responseCube.status_code << std::endl;
            continue;
        }
    }

    nlohmann::json payloadServ = {};

    auto responseServ = API->post("/api/server/free", payloadServ);

    if (responseServ.status_code != 201 && responseServ.status_code != 200) {
        std::cout << "Could not free the server : " << responseServ.status_code << std::endl;
    }

#endif


    SetGameState(GameState::PostGameVictory);
}
