//
// Created by alex- on 2025-04-26.
//

#pragma once

#include "../../Network/APIHandler.h"
#include "../Object/GameState.h"

class UiRenderer {
public:
    UiRenderer(APIHandler* API);

    void draw(GameState state);

private:
    APIHandler* API;
    // Holding input text
    char TextOne[128];
    char TextTwo[128];

    void DrawLogin(bool wasError = false);
    void DrawSignUp();
    void DrawMainMenu();
    void DrawLookingForSession();
    void DrawNewAchivements();
    void DrawAchivement();
    void DrawGameOver(bool victory);
};