// Common implementation of all platforms
#include "Socket.h"

#ifdef WIN32
    #include "Socket_windows.cpp"  // Include Windows-specific implementation
#else
    #include "Socket_posix.cpp"    // Include POSIX-specific implementation
#endif

Socket* Socket::CreateSocket() {
    Socket* socket = nullptr;

#ifdef WIN32
    socket = new Socket_windows();  // Return Windows-specific implementation
#else
    socket = new Socket_posix();    // Return POSIX-specific implementation
#endif

    socket->initialize();
    return socket;
}