//
// Created by alex- on 2025-04-28.
//

#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include <string>
#include <vector>

#include "../GameObject/Object/GameState.h"

class GameLogic {
private:
    GameLogic(){cubesLeft = {}; gameState = GameState::Login;};

public:
    static GameLogic &GetInstance() {
        static GameLogic instance;
        return instance;
    }

    //No copy or new instance
    GameLogic(GameLogic const &) = delete;
    void operator=(GameLogic const &) = delete;

    void AddPlayer(int cubes);

    void CubeDestroyed(int index);

    void PlayerVictory(int index);

    GameState GetGameState() const {return gameState;}
    void SetGameState(GameState newState){gameState = newState;}

private:
    std::vector<int> cubesLeft;
    GameState gameState;
};



#endif //GAMELOGIC_H
