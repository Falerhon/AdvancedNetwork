//
// Created by alex- on 2025-04-27.
//

#ifndef MATCHMAKINGMANAGER_H
#define MATCHMAKINGMANAGER_H
#include <chrono>

#include "APIHandler.h"


class MatchmakingManager {
public:
    MatchmakingManager(APIHandler* apiHandler)
    :  API(apiHandler), _lastPollTime(std::chrono::steady_clock::now()) {}

    void update();

private:
    APIHandler* API;
    std::chrono::steady_clock::time_point _lastPollTime;
    const float _pollInterval = 2.0f;

    void checkMatchmakingStatus();
};



#endif //MATCHMAKINGMANAGER_H
