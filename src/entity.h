#pragma once

#include "utils.h"
#include "player.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>


typedef struct{
    v2 velocity;
    v2 position;
    SDL_Surface* texture;
}Entity;

void render_entity(Entity* entity, Player* player);
void update_entiy(Entity* entity);