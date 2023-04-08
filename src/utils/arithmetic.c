#include "arithmetic.h"

struct Room calculate_room(uint64 bytes)
{
    uint32 space[4] = {0, 0, 0, 0};
    int i = 3;

    while (bytes > 0)
    {
        space[i] = bytes % 1024;
        bytes = bytes / 1024;
        i--;
    }

    struct Room room;
    room.gb = space[0];
    room.mb = space[1];
    room.kb = space[2];
    room.bytes = space[3];

    return room;
}