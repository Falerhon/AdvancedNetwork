//
// Created by alex- on 2025-04-28.
//

#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include "enet6/enet.h"

#include "../GameObject/Object/GameState.h"
#include "../Network/LinkingContext.h"

class APIHandler;

class GameLogic {
private:
    GameLogic() {
        cubesLeft = {};
        gameState = GameState::Login;
        userIds = {};
    };

public:
    static GameLogic &GetInstance() {
        static GameLogic instance;
        return instance;
    }

    //No copy or new instance
    GameLogic(GameLogic const &) = delete;

    void operator=(GameLogic const &) = delete;

    void AddPlayer(int UUID);

    void CubeDestroyed(int index);

    int GetLocalPlayerID() const { return playerID; };
    void SetLocalPlayerID(uint8_t id) { playerID = id; };

    int GetLocalPlayerNetID() const { return playerNetID; }
    void SetLocalPlayerNetID(NetworkId netId) { playerNetID = netId; };

    void PlayerVictory(int index);

    GameState GetGameState() const { return gameState; }
    void SetGameState(GameState newState) { gameState = newState; }

    void SetNewAchievements(const std::vector<std::pair<std::string, std::string> > &achievements) {
        NewAchievements = achievements;
    }

    std::vector<std::pair<std::string, std::string> > GetNewAchievements() { return NewAchievements; }

    void SetHost(ENetHost *_host) { host = _host; }
    ENetHost *GetHost() { return host; }

    void SetAPI(APIHandler *_api) { API = _api; }

    int numbOfBoxesPerPlayers = 0;

private:
    std::vector<int> cubesLeft;
    std::vector<int> userIds;
    GameState gameState;
    std::vector<std::pair<std::string, std::string> > NewAchievements;
    ENetHost *host;
    uint8_t playerID;
    NetworkId playerNetID;
    APIHandler *API;
};


#endif //GAMELOGIC_H
