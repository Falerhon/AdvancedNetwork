#include <iostream>
#include "Client.h"
#include <falcon.h>


int main() {
    Client client = Client();

    client.ConnectToServer();
    client.CreateStream();

    auto lastDataSend = std::chrono::high_resolution_clock::now();
    while (true) {
        client.Update();

        if (std::chrono::high_resolution_clock::now() - lastDataSend > std::chrono::seconds(PINGTIME)) {
            client.GenerateAndSendData();
            lastDataSend = std::chrono::high_resolution_clock::now();
        }
    }

    return EXIT_SUCCESS;
}
