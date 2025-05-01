//
// Created by alex- on 2025-04-27.
//

#include "MatchmakingManager.h"

#include <iostream>

#include "../GameLogic/GameLogic.h"
#include "enet6/enet.h"
#include "Corrade/Utility/Debug.h"

void MatchmakingManager::update() {
    auto now = std::chrono::steady_clock::now();
    float secondsSinceLastPoll = std::chrono::duration<float>(now - _lastPollTime).count();

    if (secondsSinceLastPoll >= _pollInterval) {
        _lastPollTime = now;
        checkMatchmakingStatus();
    }
}

void MatchmakingManager::checkMatchmakingStatus() {
    nlohmann::json payload = {};

    auto response = API->post("/api/matchmaking/Status", payload);
    if (response.status_code == 200) {
        Corrade::Utility::Debug{} << "Matchmaking request successful";

        std::string serverAddress;

        try {
            auto json = nlohmann::json::parse(response.text);
            serverAddress = std::string(json.at("ip")) + ":" + std::to_string(int(json.at("port")));
            std::cout << "Server found : " << serverAddress;

            if (serverAddress.size() > 1) {
                ENetAddress address;
                enet_address_set_host(&address, ENET_ADDRESS_TYPE_ANY, std::string(json.at("ip")).c_str());
                address.port = int(json.at("port"));

                ENetPeer* server = enet_host_connect(GameLogic::GetInstance().GetHost(), &address, 2, 0);
                if (server == NULL) {
                    fprintf(stderr, "No server found for Enet connection on client.\n");
                    exit(EXIT_FAILURE);
                }

                GameLogic::GetInstance().SetGameState(GameState::InGame);
            }

        } catch (const std::exception& e) {
            std::cerr << "Still looking : " << e.what() << std::endl;
        }

    } else {
        Corrade::Utility::Error{} << "Failed to check matchmaking : " << response.status_code;
    }
}
