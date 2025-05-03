//
// Created by alex- on 2025-04-26.
//

#ifndef APIHANDLER_H
#define APIHANDLER_H
#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>


class APIHandler {
public:
    APIHandler(const std::string &baseUrl);

    bool login(const std::string &username, const std::string &password);

    bool loginServer(const std::string &username, const std::string &password);

    cpr::Response get(const std::string &endpoint);

    cpr::Response post(const std::string &endpoint, const nlohmann::json &body);

    int GetUserID() const { return userId; }

private:
    std::string baseUrl;
    std::string token;
    int userId;

    cpr::Header getAuthHeader();
};


#endif //APIHANDLER_H
