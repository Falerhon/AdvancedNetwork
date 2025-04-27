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
            return DrawLogin(true);
        break;
        case GameState::SignUp:
            return DrawSignUp();
        break;
        case GameState::MainMenu:
            return DrawMainMenu();
        break;
        case GameState::LookingForSession:
            return DrawLookingForSession();
        break;

        case GameState::InGame:
            // Draw in-game UI if needed
                break;
    }
    return std::nullopt;
}

std::optional<GameState> UiRenderer::DrawLogin(bool wasError) {
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
            return GameState::MainMenu;
        }
        ImGui::End();
        return GameState::LoginFailed;
    }

    if(ImGui::Button("Signin page")) {
        ImGui::End();
        return GameState::SignUp;
    }

    ImGui::End();
    return std::nullopt;
}

std::optional<GameState> UiRenderer::DrawSignUp() {
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
            return GameState::SignUp;
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

std::optional<GameState> UiRenderer::DrawMainMenu() {
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
            return GameState::MainMenu;
        }

        ImGui::End();
        return GameState::LookingForSession;
    }

    if(ImGui::Button("Quit game")) {
        ImGui::End();
        exit(0);
    }

    ImGui::End();
    return std::nullopt;
}

std::optional<GameState> UiRenderer::DrawLookingForSession() {
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
    return std::nullopt;
}
