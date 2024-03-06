#pragma once

#include "utils.h"

typedef struct Player {
    v2 position;
    v2 direction;
    v2 camera_plane;
} Player;

void move_player(Player* player, const u8* keys);