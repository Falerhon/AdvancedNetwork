//
// Implementation for Windows
//
#include "Socket.h"
//#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

SOCKET Soc;

int Socket::initialize() {
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}

int Socket::CreateClientSocket(const std::string &HostAddress) {

    struct addrinfo *result = nullptr,
                *ptr = nullptr,
                hints{};

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Resolve the server address and port
    int iResult = getaddrinfo(HostAddress.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    Soc = INVALID_SOCKET;

    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;

    // Create a SOCKET for connecting to server
    Soc = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);
    //Allow IPV4 and IPV6 communication
    Soc = setsockopt(Soc, SOL_SOCKET, IPV6_V6ONLY, 0, sizeof(int));

    if (Soc == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect( Soc, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(Soc);
        Soc = INVALID_SOCKET;
    }
    //Free the rescources
    freeaddrinfo(result);

    if (Soc == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;
}

int Socket::CreateServerSocket() {

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    Soc = INVALID_SOCKET;

    Soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //Allow IPV4 and IPV6 communication
    Soc = setsockopt(Soc, SOL_SOCKET, IPV6_V6ONLY, 0, sizeof(int));

    if (Soc == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( Soc, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    if ( listen( Soc, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket;

    // Setup the listening socket
    iResult = bind( Soc, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(Soc, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(Soc, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(Soc);

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
            iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

//TODO : UNUSED UNTIL WE UNDERSTAND HOW IT WORKS
int Socket::ServerWaitConnection() {
    SOCKET ClientSocket;

    ClientSocket = INVALID_SOCKET;

    // Accept a client socket
    ClientSocket = accept(Soc, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(Soc);
}

int Socket::MessageSend(const char &Message) {
    // Send an initial buffer
    int iResult = send(Soc, &Message, (int) strlen(&Message), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection for sending since no more data will be sent
    // the client can still use the ConnectSocket for receiving data
    iResult = shutdown(Soc, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(Soc);
        WSACleanup();
        return 1;
    }

    return 0;
}

int Socket::MessageRecieve(const char &Message) {

    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

    int iResult;
    // Receive data until the server closes the connection
    do {
        iResult = recv(Soc, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    } while (iResult > 0);

    return 0;
}

int Socket::CloseSocket() {
    // cleanup
    closesocket(Soc);
    WSACleanup();
}