﻿// Implementation for Linux and Mac

#include "Socket.h"

#if defined(__linux__) || defined(__APPLE__)
//Posix needed variables and includes
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <cstring> //for memset
#include <cerrno> //for errno and strerror
#include <cstdio> //for printf

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

class Socket_posix : public Socket {
  private:
  SOCKET Socket;
  struct sockaddr_in6 serverAddr;
  socklen_t serverAddrLen;
public:
int initialize() override {
      //Posix doesnt need initialisation
      return 0;
}

int CreateClientSocket(const std::string &HostAddress) override {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket

    // Get address info for the given host
    int status = getaddrinfo(HostAddress.c_str(), DEFAULT_PORT, &hints, &res);
    if (status != 0) {
      printf("getaddrinfo error: %s\n", gai_strerror(status));
      return 1;
    }

      Socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if(Socket == INVALID_SOCKET){
        printf("Failed to create client socket\n");
        freeaddrinfo(res);
        return 1;
      }

      printf("Successfully created client socket. Ready to send to %s\n", HostAddress.c_str());

      // Store the server address and length
      memcpy(&serverAddr, res->ai_addr, res->ai_addrlen);
      serverAddrLen = res->ai_addrlen;

      freeaddrinfo(res);
      return 0;
}

int CreateServerSocket() override {
    Socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if(Socket == INVALID_SOCKET){
      printf("Failed to create server socket\n");
      return 1;
    }

    //Set up the support for both IPv4 and IPv6
    char optval = 0;
    if (setsockopt(Socket, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0) {
      printf("Failed to set IPV6_V6ONLY option.\n");
      close(Socket);
      return 1;
    }

    //Bind the socket
    sockaddr_in6  serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_addr = in6addr_any;
    serverAddress.sin6_port = htons(5555);

    if(bind(Socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){
      printf("Failed to bind\n");
      close(Socket);
      return 1;
    }

  char buffer[1024];
  sockaddr_in6 clientAddress;
  socklen_t clientAddrSize = sizeof(clientAddress);

  // Receive data from the client
  int bytesRead = recvfrom(Socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddress, &clientAddrSize);
  if (bytesRead > 0) {
    // Process the data
    buffer[bytesRead] = '\0'; // Ensure the received data is a valid C-string

    printf("Received data: %s\n", buffer);

    //Send back the data to the client
    int bytesSent = sendto(Socket, buffer, bytesRead, 0, (struct sockaddr *) &clientAddress, clientAddrSize);
    if (bytesSent == SOCKET_ERROR) {
      printf("Failed to send data to the client \n");
      close(Socket);
      return 1;
    }

    printf("Sent data back to client: %s\n", buffer);
  }

  return 0;
}

int CloseSocket() override {
  close(Socket);
  return 0;
}

int MessageSend(const std::string &Message) override {
  int bytesSent = sendto(Socket, Message.c_str(), Message.size(), 0, (struct sockaddr *) &serverAddr, serverAddrLen);

  if (bytesSent == SOCKET_ERROR) {
    printf("Failed to send message. Error: %d\n", strerror(errno));
    return 1;
  }
  printf("Message sent: %s\n", Message.c_str());
  return 0;
}

int MessageReceived(char *buffer, int bufferSize) override {
  struct sockaddr_in6 recvAddr;
  socklen_t recvAddrLen = sizeof(recvAddr);

  int bytesReceived = recvfrom(Socket, buffer, bufferSize, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
  if (bytesReceived == SOCKET_ERROR) {
    printf("Failed to receive message. Error: %d\n", strerror(errno));
    return 1;
  }

  buffer[bytesReceived] = '\0'; // Null-terminate the received data
  printf("Received message: %s\n", buffer);
  return 0;
}
};
#endif

