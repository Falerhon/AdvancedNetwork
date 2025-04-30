//
// Created by alex- on 2025-04-28.
//

#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include <string>
#include <vector>
#include <utility>

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

    void SetNewAchievements(const std::vector<std::pair<std::string, std::string>> &achievements){NewAchievements = achievements;}
    std::vector<std::pair<std::string, std::string>> GetNewAchievements(){return NewAchievements;}

private:
    std::vector<int> cubesLeft;
    GameState gameState;
    std::vector<std::pair<std::string, std::string>> NewAchievements;
};



#endif //GAMELOGIC_H
