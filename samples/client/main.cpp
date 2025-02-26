#include <iostream>
#include "Client.h"
#include <falcon.h>


int main() {
    bool doOnce = true;
    Client client = Client();

    client.ConnectToServer();

    auto lastDataSend = std::chrono::high_resolution_clock::now();
    while (true) {
        client.Update();

        if (std::chrono::high_resolution_clock::now() - lastDataSend > std::chrono::seconds(PINGTIME)) {
            if (doOnce) {
                client.CreateStream();
                doOnce = false;
            } else {
                client.GenerateAndSendData();
                lastDataSend = std::chrono::high_resolution_clock::now();
            }

        }
    }

    return EXIT_SUCCESS;
}
