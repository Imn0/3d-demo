#include "render.h"
#include "entity.h"

extern u8 map[MAPSIZE_R][MAPSIZE_C];
extern f32* z_buffer;
extern u32* pixels;

void calculate_line_draw_start_end(i32* draw_start, i32* draw_end, f32 perpendicular_wall_dist) {
    i32 line_height = (i32)(SCREEN_HEIGHT / perpendicular_wall_dist);
    *draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
    *draw_end = line_height / 2 + SCREEN_HEIGHT / 2;

}

u32 get_argb_from_position(f32 x, f32 y, SDL_Surface* map1) {
    i32 x_index = MIN((i32)(x * map1->w), map1->w);
    i32 y_index = MIN((i32)(y * map1->h), map1->h);

    SDL_PixelFormat* pixelFormat = map1->format;

    i32 pixelIndex = (y_index * map1->pitch) + (x_index * pixelFormat->BytesPerPixel);

    u8* _pixels = (u8*)map1->pixels;

    u8 red, green, blue, alpha;
    SDL_GetRGBA(*(u32*)(_pixels + pixelIndex), pixelFormat, &red, &green, &blue, &alpha);
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void render_line(i32 x, i32 y0, i32 y1, u32 color) {
    if (y0 < 0) y0 = 0;
    if (y0 > SCREEN_HEIGHT) y0 = SCREEN_HEIGHT;
    if (y1 < 0) y1 = 0;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;


    for (i32 y = y0; y < y1; y++) {
        pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}


void render(Player player, SDL_Surface* map1, LinkedList* entities) {

    SDL_LockSurface(map1);

    for (i32 x = 0; x < SCREEN_WIDTH; x++) {
        f32 cameara_x = 2 * (x / (f32)SCREEN_WIDTH) - 1;

        const v2 ray_dir = {
            player.direction.x + player.camera_plane.x * cameara_x,
            player.direction.y + player.camera_plane.y * cameara_x
        };

        v2i map_pos = { (i32)player.position.x, (i32)player.position.y };

        // DDA setup 
        v2 side_dist;
        v2 delta_dist = { fabsf(1 / ray_dir.x), fabsf(1 / ray_dir.y) };
        v2 step;

        if (ray_dir.x < 0) {
            step.x = -1;
            side_dist.x = (player.position.x - map_pos.x) * delta_dist.x;
        }
        else {
            step.x = 1;
            side_dist.x = (map_pos.x + 1.0f - player.position.x) * delta_dist.x;
        }
        if (ray_dir.y < 0) {
            step.y = -1;
            side_dist.y = (player.position.y - map_pos.y) * delta_dist.y;
        }
        else {
            step.y = 1;
            side_dist.y = (map_pos.y + 1.0f - player.position.y) * delta_dist.y;
        }

        // DDA
        bool hit = false;
        i32 side;
        while (!hit) {
            if (side_dist.x < side_dist.y) {
                side_dist.x += delta_dist.x;
                map_pos.x += step.x;
                side = 0;
            }
            else {
                side_dist.y += delta_dist.y;
                map_pos.y += step.y;
                side = 1;
            }

            if (map[map_pos.y][map_pos.x] > 0) {
                hit = true;
            }
        }

        // calculate distance to wall
        f32 perpendicular_wall_dist;
        if (side == 0) {
            perpendicular_wall_dist = (map_pos.x - player.position.x + (1 - step.x) / 2) / ray_dir.x;
        }
        else {
            perpendicular_wall_dist = (map_pos.y - player.position.y + (1 - step.y) / 2) / ray_dir.y;
        }

        // calculate how far along a cell ray hit, 0 is the beggining of the cell, 1 is the end
        f32 cell_dist = 0;
        if (side != 0) {
            cell_dist = player.position.x + perpendicular_wall_dist * ray_dir.x;
        }
        else {
            cell_dist = player.position.y + perpendicular_wall_dist * ray_dir.y;
        }
        cell_dist = cell_dist - (i32)cell_dist;

        // calculate height of wall slice on screen
        i32 draw_start, draw_end;
        calculate_line_draw_start_end(&draw_start, &draw_end, perpendicular_wall_dist);

        render_line(x, 0, draw_start, CELLING_COLOR);

        i32 actual_start = CLIP(draw_start, 0, SCREEN_HEIGHT);
        i32 actual_end = CLIP(draw_end, 0, SCREEN_HEIGHT);

        for (i32 y = actual_start; y < actual_end; y++) {

            u32 color = get_argb_from_position(cell_dist,
                                               (f32)(y - draw_start) / (f32)(draw_end - draw_start),
                                               map1);

            if (side == 1) {
                u32
                    r = ((color & 0x00FF0000) * 0x90) >> 8,
                    g = ((color & 0x0000FF00) * 0x90) >> 8,
                    b = ((color & 0x000000FF) * 0x90) >> 8;

                color = 0xFF000000 | (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);
            }


            //! this
            pixels[(y * SCREEN_WIDTH) + x] = color;

        }

        render_line(x, draw_end, SCREEN_HEIGHT, FLOOR_COLOR);
        z_buffer[x] = perpendicular_wall_dist;

    }
    SDL_UnlockSurface(map1);
    if (entities == NULL) { return; }

    ListItr* itr = ll_itr_assign(entities);

    while (ll_itr_has_next(itr)) {
        render_entity((Entity*)ll_itr_get(itr), &player);
        ll_itr_next(itr);
    }
    free(itr);

}



void render_entity(Entity* entity, Player* player) {

    // sprite position relative to the player
    f32 sprite_x = entity->position.x - player->position.x;
    f32 sprite_y = entity->position.y - player->position.y;

    // transform sprite with the inverse camera matrix
    f32 inv_det = 1.0 / (player->camera_plane.x * player->direction.y - player->direction.x * player->camera_plane.y);
    f32 transform_x = inv_det * (player->direction.y * sprite_x - player->direction.x * sprite_y);
    f32 transform_y = inv_det * (-player->camera_plane.y * sprite_x + player->camera_plane.x * sprite_y);

    // calculate screen position of the sprite
    i32 sprite_screen_x = (i32)((SCREEN_WIDTH / 2) * (1 + transform_x / transform_y));

    // calculate height and width of the sprite
    i32 sprite_height = abs((i32)(SCREEN_HEIGHT / transform_y));
    i32 sprite_width = abs((i32)(SCREEN_HEIGHT / transform_y));

    // calculate start and end points for drawing the sprite
    i32 draw_start_y = -sprite_height / 2 + SCREEN_HEIGHT / 2;
    i32 draw_end_y = sprite_height / 2 + SCREEN_HEIGHT / 2;

    i32 draw_start_x = -sprite_width / 2 + sprite_screen_x;
    i32 draw_end_x = sprite_width / 2 + sprite_screen_x;


    i32 actual_draw_start_x = CLIP(draw_start_x, 0, SCREEN_WIDTH);
    i32 actual_draw_end_x = CLIP(draw_end_x, 0, SCREEN_WIDTH);

    for (i32 stripe = actual_draw_start_x; stripe < actual_draw_end_x; stripe++) {


        //!this
        f32 tex_x = (f32)(stripe - draw_start_x) / (f32)(draw_end_x - draw_start_x);

        if (transform_y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform_y < z_buffer[stripe]) {
            i32 actual_draw_start_y = CLIP(draw_start_y, 0, SCREEN_HEIGHT);
            i32 actual_draw_end_y = CLIP(draw_end_y, 0, SCREEN_HEIGHT);

            for (i32 y = actual_draw_start_y; y < actual_draw_end_y; y++) {

                //!this
                f32 tex_y = (f32)(y - draw_start_y) / (f32)(draw_end_y - draw_start_y);
                u32 color = get_argb_from_position(tex_x, tex_y, entity->texture);

                if ((color & 0xFFFFFFFF) != 0) {
                    z_buffer[stripe] = transform_y;
                    pixels[y * SCREEN_WIDTH + stripe] = color;
                }
            }
        }
    }
}