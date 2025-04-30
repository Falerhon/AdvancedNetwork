//
// Created by alex- on 2025-04-28.
//

#include "GameLogic.h"

#include <iostream>
#include <ostream>

void GameLogic::AddPlayer(int cubes) {
    cubesLeft.push_back(cubes);
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
    SetGameState(GameState::PostGameVictory);
    //TODO : Implement player victory logic
}
