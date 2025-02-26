#include <iostream>
#include "Client.h"
#include <falcon.h>


int main() {
    Client client = Client();

    client.ConnectToServer();

    while (true) {
        client.Update();
    }

    return EXIT_SUCCESS;
}
