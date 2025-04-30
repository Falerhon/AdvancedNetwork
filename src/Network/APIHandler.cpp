//
// Created by alex- on 2025-04-26.
//

#include "APIHandler.h"

#include <iostream>

APIHandler::APIHandler(const std::string& baseUrl)
    : baseUrl(baseUrl) {}

//Login to the Online server an recieve a token
bool APIHandler::login(const std::string& username, const std::string& password) {
    nlohmann::json payload = {
        {"username", username},
        {"passwordHash", password}
    };

    auto response = cpr::Post(
        cpr::Url{baseUrl + "/api/Auth/login"},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    if (response.status_code == 200) {
        try {
            auto json = nlohmann::json::parse(response.text);
            token = json.at("token");
            userId = json.at("id");
            std::cout << "Login successful. Token acquired." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse login response: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cerr << "Login failed: " << response.status_code << std::endl;
        return false;
    }
}

bool APIHandler::loginServer(const std::string &username, const std::string &password) {
    nlohmann::json payload = {
        {"serverName", username},
        {"password", password}
    };

    auto response = cpr::Post(
        cpr::Url{baseUrl + "/api/server/auth/login"},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    if (response.status_code == 200) {
        try {
            auto json = nlohmann::json::parse(response.text);
            token = json.at("token");
            std::cout << "Login successful. Token acquired." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse login response: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cerr << "Login failed: " << response.status_code << std::endl;
        return false;
    }
}

//Get request to the online server
cpr::Response APIHandler::get(const std::string& endpoint) {
    return cpr::Get(
        cpr::Url{baseUrl + endpoint},
        getAuthHeader()
    );
}

//Post request to the online server
cpr::Response APIHandler::post(const std::string& endpoint, const nlohmann::json& body) {
    return cpr::Post(
        cpr::Url{baseUrl + endpoint},
        getAuthHeader(),
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{body.dump()}
    );
}

//Returns a built authentication header
cpr::Header APIHandler::getAuthHeader() {
    if (token.empty()) {
        return {};
    }
    return { {"Authorization", "Bearer " + token} };
}