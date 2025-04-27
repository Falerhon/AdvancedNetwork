//
// Created by alex- on 2025-04-26.
//

#pragma once

#include "../../Network/APIHandler.h"
#include "../Object/GameState.h"

class UiRenderer {
public:
    UiRenderer(APIHandler* API);

    std::optional<GameState> draw(GameState& state);

private:
    APIHandler* API;
    // Holding input text
    char TextOne[128];
    char TextTwo[128];

    std::optional<GameState> DrawLogin(bool wasError = false);
    std::optional<GameState> DrawSignUp();
    std::optional<GameState> DrawMainMenu();
    std::optional<GameState> DrawLookingForSession();
};