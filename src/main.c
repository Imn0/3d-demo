#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "utils.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TARGET_DELAY_MS 16
#define MAPSIZE_R 8
#define MAPSIZE_C 8

#define CELLING_COLOR 0xFF202020
#define FLOOR_COLOR 0xFF303030

typedef struct Player {
    v2 position;
    v2 direction;
    v2 camera_plane;
} Player;

u8 map[MAPSIZE_R][MAPSIZE_C] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

f32* z_buffer;

typedef struct Sprite {
    v2 position;
    SDL_Surface* texture;
} Sprite;

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


void render_sprites(Player player, u32* pixels, Sprite sprite) {
    // sprite position relative to the player
    f32 sprite_x = sprite.position.x - player.position.x;
    f32 sprite_y = sprite.position.y - player.position.y;

    // transform sprite with the inverse camera matrix
    f32 inv_det = 1.0 / (player.camera_plane.x * player.direction.y - player.direction.x * player.camera_plane.y);
    f32 transform_x = inv_det * (player.direction.y * sprite_x - player.direction.x * sprite_y);
    f32 transform_y = inv_det * (-player.camera_plane.y * sprite_x + player.camera_plane.x * sprite_y);

    // calculate screen position of the sprite
    int sprite_screen_x = (int)((SCREEN_WIDTH / 2) * (1 + transform_x / transform_y));

    // calculate height and width of the sprite
    int sprite_height = abs((int)(SCREEN_HEIGHT / transform_y));
    int sprite_width = abs((int)(SCREEN_HEIGHT / transform_y));

    // calculate start and end points for drawing the sprite
    int draw_start_y = -sprite_height / 2 + SCREEN_HEIGHT / 2;
    // if (draw_start_y < 0) draw_start_y = 0;
    int draw_end_y = sprite_height / 2 + SCREEN_HEIGHT / 2;
    // if (draw_end_y >= SCREEN_HEIGHT) draw_end_y = SCREEN_HEIGHT - 1;

    int draw_start_x = -sprite_width / 2 + sprite_screen_x;
    // if (draw_start_x < 0) draw_start_x = 0;
    int draw_end_x = sprite_width / 2 + sprite_screen_x;
    // if (draw_end_x >= SCREEN_WIDTH) draw_end_x = SCREEN_WIDTH - 1;

    for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
        if (stripe < 0 || stripe >= SCREEN_WIDTH) continue;
        f32 tex_x = (f32)(stripe - draw_start_x) / (f32)(draw_end_x - draw_start_x);

        if (transform_y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform_y < z_buffer[stripe]) {
            for (int y = draw_start_y; y < draw_end_y; y++) {
                if (y < 0 || y >= SCREEN_HEIGHT) continue;
                f32 tex_y = (f32)(y - draw_start_y) / (f32)(draw_end_y - draw_start_y);
                u32 color = get_argb_from_position(tex_x, tex_y, sprite.texture);

                if ((color & 0xFFFFFFFF) != 0) {
                    pixels[y * SCREEN_WIDTH + stripe] = color;
                }
            }
        }
    }
}

void render_line(int x, int y0, int y1, u32 color, u32* pixels) {
    if (y0 < 0) y0 = 0;
    if (y0 > SCREEN_HEIGHT) y0 = SCREEN_HEIGHT;
    if (y1 < 0) y1 = 0;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;


    for (int y = y0; y < y1; y++) {
        pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

void render(Player player, u32* pixels, SDL_Surface* map1) {

    SDL_LockSurface(map1);

    for (int x = 0; x < SCREEN_WIDTH; x++) {
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
        int side;
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


        u32 wall_color = (side == 1) ? 0xFFFF0000 : 0xFF771010;


        render_line(x, 0, draw_start, CELLING_COLOR, pixels);

        for (int y = draw_start; y < draw_end; y++) {
            if (y < 0 || y >= SCREEN_HEIGHT) continue;
            pixels[(y * SCREEN_WIDTH) + x] = get_argb_from_position(1 - cell_dist, (f32)(y - draw_start) / (f32)(draw_end - draw_start), map1);
        }

        render_line(x, draw_end, SCREEN_HEIGHT, FLOOR_COLOR, pixels);
        z_buffer[x] = perpendicular_wall_dist;

    }
    SDL_UnlockSurface(map1);
}

void rotate(Player* player, f32 angle) {
    v2 old_direction = { player->direction.x, player->direction.y };
    v2 old_camera_plane = { player->camera_plane.x, player->camera_plane.y };

    player->direction.x = old_direction.x * cosf(angle) - old_direction.y * sinf(angle);
    player->direction.y = old_direction.x * sinf(angle) + old_direction.y * cosf(angle);

    player->camera_plane.x = old_camera_plane.x * cosf(angle) - old_camera_plane.y * sinf(angle);
    player->camera_plane.y = old_camera_plane.x * sinf(angle) + old_camera_plane.y * cosf(angle);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("demo",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          720,
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCREEN_WIDTH,
                                             SCREEN_HEIGHT);

    u32* pixels = (u32*)malloc(sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
    z_buffer = (f32*)malloc(sizeof(f32) * SCREEN_WIDTH);

    SDL_Surface* test = IMG_Load("../assets/test.png");

    Sprite sprite = { {5.0f, 5.0f}, .texture = IMG_Load("../assets/test_sprite.png") }; // Example sprite position


    Player player = {
        .position = {2.0f, 2.0f},
        .direction = {1.0f, 0.0f},
        .camera_plane = {0.0f, 0.66f}
    };

    int fps = 0;
    int last_time_fps = 0;
    char buffer[32];

    bool running = true;
    SDL_Event event;
    while (running) {
        fps++;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        // FPS
        if (SDL_GetTicks() - last_time_fps > 1000) {
            snprintf(buffer, sizeof(buffer), "FPS: %d", fps);
            SDL_SetWindowTitle(window, buffer);
            fps = 0;
            last_time_fps = SDL_GetTicks();
        }

        const u8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT]) {
            rotate(&player, -0.01f);
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            rotate(&player, 0.01f);
        }
        if (keys[SDL_SCANCODE_UP]) {
            player.position.x += player.direction.x * 0.01f;
            player.position.y += player.direction.y * 0.01f;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            player.position.x -= player.direction.x * 0.01f;
            player.position.y -= player.direction.y * 0.01f;
        }
        if (player.position.x < 1) player.position.x = 1;
        if (player.position.y < 1) player.position.y = 1;
        if (player.position.x > MAPSIZE_C - 1) player.position.x = MAPSIZE_C - 1;
        if (player.position.y > MAPSIZE_R - 1) player.position.y = MAPSIZE_R - 1;

        memset(pixels, 0, sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
        memset(z_buffer, 255, sizeof(u8) * SCREEN_WIDTH);

        render(player, pixels, test);

        render_sprites(player, pixels, sprite);

        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);


        SDL_RenderCopyEx(renderer,
                         texture,
                         NULL,
                         NULL,
                         0.0,
                         NULL,
                         0);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(test);
    free(z_buffer);
    free(pixels);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}