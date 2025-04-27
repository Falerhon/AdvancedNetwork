//
// Created by alex- on 2025-04-27.
//

#include "MatchmakingManager.h"

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
    } else {
        Corrade::Utility::Error{} << "Failed to check matchmaking : " << response.status_code;
    }
}
