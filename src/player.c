#include "player.h"
#include <SDL2/SDL.h>
#include <math.h>

extern u8 map[MAPSIZE_R][MAPSIZE_C];


void rotate(Player* player, f32 angle) {
    v2 old_direction = { player->direction.x, player->direction.y };
    v2 old_camera_plane = { player->camera_plane.x, player->camera_plane.y };

    player->direction.x = old_direction.x * cosf(angle) - old_direction.y * sinf(angle);
    player->direction.y = old_direction.x * sinf(angle) + old_direction.y * cosf(angle);

    player->camera_plane.x = old_camera_plane.x * cosf(angle) - old_camera_plane.y * sinf(angle);
    player->camera_plane.y = old_camera_plane.x * sinf(angle) + old_camera_plane.y * cosf(angle);
}

void move_player(Player* player, const u8* keys) {

    v2 pos_delta = { 0.0f, 0.0f };

    if (keys[SDL_SCANCODE_LEFT]) {
        rotate(player, -0.01f);
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        rotate(player, 0.01f);
    }
    if (keys[SDL_SCANCODE_UP]) {
        pos_delta.x = player->direction.x * 0.01f;
        pos_delta.y = player->direction.y * 0.01f;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        pos_delta.x = -player->direction.x * 0.01f;
        pos_delta.y = -player->direction.y * 0.01f;
    }

    // check collisions
    if (map[(i32)(player->position.y)][(i32)(player->position.x + pos_delta.x)] != 0) {
        pos_delta.x = 0.0f;
    }
    if (map[(i32)(player->position.y + pos_delta.y)][(i32)(player->position.x)] != 0) {
        pos_delta.y = 0.0f;
    }
    player->position.x += pos_delta.x;
    player->position.y += pos_delta.y;
}