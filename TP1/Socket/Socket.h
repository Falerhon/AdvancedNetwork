//
// Created by alex- on 2025-01-23.
//
#ifndef SOCKET_H
#define SOCKET_H
#include <string>

#define DEFAULT_PORT "5555"
#define DEFAULT_BUFLEN 512

// Forward declare platform-specific classes
#ifdef WIN32
class Socket_windows;  // Forward declaration of Windows-specific class
#else
class Socket_posix;    // Forward declaration of POSIX-specific class
#endif

class Socket {
public:
    static Socket* CreateSocket();
    virtual  int initialize() = 0;

    virtual  int CreateClientSocket(const std::string &HostAddress) = 0;

    virtual  int CreateServerSocket() = 0;

    virtual  int CloseSocket() = 0;

    virtual  int MessageSend(const std::string &Message) = 0;
    virtual  int MessageReceived(char *buffer, int bufferSize) = 0;
};
#endif //SOCKET_H
