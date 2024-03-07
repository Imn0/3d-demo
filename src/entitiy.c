#include "entity.h"
#include "utils.h"
#include "render.h"

extern u8 map[MAPSIZE_R][MAPSIZE_C];

void update_entities(LinkedList* entities) {

    ll_init_iterator(entities);

    while (ll_itr_has_current(entities)) {
        bool remove = update_entiy((Entity*)ll_itr_get_current(entities));

        if (remove) {
            Entity* e = ll_itr_pop_current(entities);
            free(e);
        }
        else {
            ll_itr_next(entities);
        }

    }

}

bool update_entiy(Entity* entity) {
    if (entity == NULL) {
        return true;
    }

    entity->position.x += entity->velocity.x;
    entity->position.y += entity->velocity.y;

    if (map[(i32)entity->position.y][(i32)entity->position.x] != 0) {
        return true;
    }
    return false;
}