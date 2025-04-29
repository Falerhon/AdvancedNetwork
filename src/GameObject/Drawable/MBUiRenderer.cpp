//
// Created by alex- on 2025-04-26.
//

#include "MBUiRenderer.h"
#include <imgui.h>
#include <iostream>
#include <string>

#include "../../GameLogic/GameLogic.h"

UiRenderer::UiRenderer(APIHandler* api) {
    TextOne[0] = '\0';
    TextTwo[0] = '\0';
    API = api;
}

void UiRenderer::draw(GameState state) {

    switch(state) {
        case GameState::Login:
            DrawLogin();
        break;
        case GameState::LoginFailed:
            //The same, but we will add something to the UI
            DrawLogin(true);
        break;
        case GameState::SignUp:
            DrawSignUp();
        break;
        case GameState::MainMenu:
            DrawMainMenu();
        break;
        case GameState::LookingForSession:
            DrawLookingForSession();
        break;

        case GameState::InGame:
            // Draw in-game UI if needed
                break;
    }
}

void UiRenderer::DrawLogin(bool wasError) {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (wasError) {
        ImGui::Text("Error during login");
    }

    ImGui::InputText("Username", TextOne, IM_ARRAYSIZE(TextOne));
    ImGui::InputText("Password", TextTwo, IM_ARRAYSIZE(TextTwo), ImGuiInputTextFlags_Password);

    if(ImGui::Button("Login")) {

        if (API->login(TextOne, TextTwo)) {
            ImGui::End();
            GameLogic::GetInstance().SetGameState(GameState::MainMenu);
            return;
        }
        ImGui::End();
        GameLogic::GetInstance().SetGameState(GameState::LoginFailed);
        return;
    }

    if(ImGui::Button("Signin page")) {
        ImGui::End();
        GameLogic::GetInstance().SetGameState(GameState::SignUp);
        return;
    }

    ImGui::End();;
}

void UiRenderer::DrawSignUp() {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Sign up", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::InputText("Username", TextOne, IM_ARRAYSIZE(TextOne));
    ImGui::InputText("Password", TextTwo, IM_ARRAYSIZE(TextTwo), ImGuiInputTextFlags_Password);

    if(ImGui::Button("Sign up")) {

        nlohmann::json payload = {
            {"username", TextOne},
            {"passwordHash", TextTwo}
        };

        auto response = API->post("/useritems", payload);

        if (response.status_code != 201) {
            std::cout << "Sign up failed" << response.status_code << std::endl;
            ImGui::End();
            GameLogic::GetInstance().SetGameState(GameState::SignUp);
            return;
        }

        if (API->login(TextOne, TextTwo)) {
            ImGui::End();
            GameLogic::GetInstance().SetGameState(GameState::MainMenu);
            return;
        }

    }

    if(ImGui::Button("Login page")) {
        ImGui::End();
        GameLogic::GetInstance().SetGameState(GameState::Login);
        return;
    }

    ImGui::End();
}

void UiRenderer::DrawMainMenu() {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if(ImGui::Button("Start game")) {

        nlohmann::json payload = {};
        auto response = API->post("/api/matchmaking/enqueue", payload);

        if (response.status_code != 200) {
            std::cout << "Could not start looking for session" << response.status_code << std::endl;
            ImGui::End();
            GameLogic::GetInstance().SetGameState(GameState::MainMenu);
            return;
        }

        ImGui::End();
        GameLogic::GetInstance().SetGameState(GameState::LookingForSession);
        return;
    }

    if(ImGui::Button("Quit game")) {
        ImGui::End();
        exit(0);
    }

    ImGui::End();
}

void UiRenderer::DrawLookingForSession() {
    ImGuiIO& io = ImGui::GetIO(); // Get window size
    ImVec2 windowSize = ImVec2(500, 250); // Set your login window size

    ImVec2 windowPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("Matchmaking", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Looking for session...");

    ImGui::End();
}
