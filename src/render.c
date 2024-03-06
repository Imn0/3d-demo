#include "render.h"

extern u8 map[MAPSIZE_R][MAPSIZE_C];
extern f32* z_buffer;
extern u32* pixels;

void calculate_line_draw_start_end(i32* draw_start, i32* draw_end, f32 perpendicular_wall_dist) {
    i32 line_height = (i32)(SCREEN_HEIGHT / perpendicular_wall_dist);
    *draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
    *draw_end = line_height / 2 + SCREEN_HEIGHT / 2;

}

u32 get_argb_from_position(f32 x, f32 y, SDL_Surface* map1) {
    //size is 256/256
    i32 x_index = MIN((i32)(x * 256), 255);
    i32 y_index = MIN((i32)(y * 256), 255);

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


void render(Player player, SDL_Surface* map1) {

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

        f32 total_distance = (f32)(draw_end - draw_start);

        i32 actual_start = CLIP(draw_start, 0, SCREEN_HEIGHT);
        i32 actual_end = CLIP(draw_end, 0, SCREEN_HEIGHT);

        for (i32 y = actual_start; y < actual_end; y++) {
            //! this
            pixels[(y * SCREEN_WIDTH) + x]
                = get_argb_from_position(cell_dist,
                                         (f32)(y - draw_start) / (f32)(draw_end - draw_start),
                                         map1);
        }

        render_line(x, draw_end, SCREEN_HEIGHT, FLOOR_COLOR);
        z_buffer[x] = perpendicular_wall_dist;

    }
    SDL_UnlockSurface(map1);
}


