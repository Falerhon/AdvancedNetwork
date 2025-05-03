#ifndef NETWORKEVENT_H
#define NETWORKEVENT_H
#include <cstdint>

enum class NetworkEventType : uint8_t {
    KEYBOARD_INPUT,
    MOUSE_INPUT,
    SNAPSHOT,
    ENDGAME,
    CONNECTION
};
#endif //NETWORKEVENT_H
