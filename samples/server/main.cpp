#include "Server.h"
#include "spdlog/spdlog.h"


int main() {
    Server server = Server();
    while (true) {
        server.Update();
    }

    return EXIT_SUCCESS;
}
