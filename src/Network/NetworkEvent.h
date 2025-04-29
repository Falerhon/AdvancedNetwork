#ifndef NETWORKEVENT_H
#define NETWORKEVENT_H
#include <cstdint>

enum class NetworkEventType : uint8_t {
    INPUT,
    SNAPSHOT,
    ENDGAME
};
#endif //NETWORKEVENT_H
