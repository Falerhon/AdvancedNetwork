//
// Created by alex- on 2025-01-23.
//

#ifndef SOCKET_H
#define SOCKET_H
#include <string>

#endif //SOCKET_H
#define DEFAULT_PORT "5555"
#define DEFAULT_BUFLEN 512

struct UNIVERSALSOCKET {
    int af;
    int type;
    int protocol;

    UNIVERSALSOCKET(int af, int type, int protocol) :af(af),type(type),protocol(protocol){};
};

class Socket {

//UNIVERSALSOCKET soc = {0,0,0};

protected:

public:
    static int initialize();

    static int CreateClientSocket(const std::string &HostAddress);

    static int CreateServerSocket();
    static int ServerWaitConnection();

    static int CloseSocket();

    static int MessageSend(const char &Message);
    static int MessageRecieve(const char &Message);

    //static int CreateServerSocket(const std::string &HostAddress);
};
