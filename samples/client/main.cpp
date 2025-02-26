#include <iostream>
#include "Client.h"
#include <falcon.h>


int main() {
    Client client = Client();

    while (true) {
        client.Update();
    }

    return EXIT_SUCCESS;
}
