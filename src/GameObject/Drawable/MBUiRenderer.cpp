//
// Created by alex- on 2025-04-26.
//

#include "MBUiRenderer.h"
#include <imgui.h>
#include <iostream>
#include <string>

UiRenderer::UiRenderer(APIHandler* api) {
    TextOne[0] = '\0';
    TextTwo[0] = '\0';
    API = api;
}

std::optional<GameState> UiRenderer::draw(GameState& state) {

    switch(state) {
        case GameState::Login:
            return DrawLogin();
        break;
        case GameState::LoginFailed:
            //The same, but we will add something to the UI
            return DrawLogin();
        break;
        case GameState::SignIn:
            return DrawSignIn();
        break;
        case GameState::MainMenu:
            ImGui::Begin("Main Menu");
        ImGui::Text("Welcome!");
        if(ImGui::Button("Start Game")) {
            state = GameState::InGame;
        }
        ImGui::End();
        break;

        case GameState::InGame:
            // Draw in-game UI if needed
                break;
    }
    return std::nullopt;
}

std::optional<GameState> UiRenderer::DrawLogin() {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::InputText("Username", TextOne, IM_ARRAYSIZE(TextOne));
    ImGui::InputText("Password", TextTwo, IM_ARRAYSIZE(TextTwo), ImGuiInputTextFlags_Password);

    if(ImGui::Button("Login")) {

        if (API->login(TextOne, TextTwo)) {
            ImGui::End();
            return GameState::MainMenu;
        }
    }

    if(ImGui::Button("Signin page")) {
        ImGui::End();
        return GameState::SignIn;
    }

    ImGui::End();
    return std::nullopt;
}

std::optional<GameState> UiRenderer::DrawSignIn() {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Sign in", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::InputText("Username", TextOne, IM_ARRAYSIZE(TextOne));
    ImGui::InputText("Password", TextTwo, IM_ARRAYSIZE(TextTwo), ImGuiInputTextFlags_Password);

    if(ImGui::Button("Sign in")) {

        nlohmann::json payload = {
            {"username", TextOne},
            {"passwordHash", TextTwo}
        };

        auto response = API->post("/useritems", payload);

        if (response.status_code != 201) {
            std::cout << "Sign in failed" << response.status_code << std::endl;
            ImGui::End();
            return GameState::SignIn;
        }

        if (API->login(TextOne, TextTwo)) {
            ImGui::End();
            return GameState::MainMenu;
        }

    }

    if(ImGui::Button("Login page")) {
        ImGui::End();
        return GameState::Login;
    }

    ImGui::End();
    return std::nullopt;
}
