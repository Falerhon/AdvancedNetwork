#include <Socket.h>

int main() {
    Socket* socket = Socket::CreateSocket();
    socket->CreateServerSocket();
    return 0;
}