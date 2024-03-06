#include "entity.h"
#include "utils.h"
#include "render.h"


void update_entiy(Entity* entity){
    entity->position.x += entity->velocity.x;
    entity->position.y += entity->velocity.y;
}