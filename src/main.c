#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "utils.h"
#include "player.h"
#include "render.h"

u8 map[MAPSIZE_R][MAPSIZE_C] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

f32* z_buffer;
u32* pixels;

typedef struct Sprite {
    v2 position;
    SDL_Surface* texture;
} Sprite;

//TODO 
/*
void render_sprite(Player player, u32* pixels, Sprite sprite) {
    // sprite position relative to the player
    f32 sprite_x = sprite.position.x - player.position.x;
    f32 sprite_y = sprite.position.y - player.position.y;

    // transform sprite with the inverse camera matrix
    f32 inv_det = 1.0 / (player.camera_plane.x * player.direction.y - player.direction.x * player.camera_plane.y);
    f32 transform_x = inv_det * (player.direction.y * sprite_x - player.direction.x * sprite_y);
    f32 transform_y = inv_det * (-player.camera_plane.y * sprite_x + player.camera_plane.x * sprite_y);

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
                u32 color = get_argb_from_position(tex_x, tex_y, sprite.texture);

                if ((color & 0xFFFFFFFF) != 0) {
                    pixels[y * SCREEN_WIDTH + stripe] = color;
                }
            }
        }
    }
}
*/

i32 main(i32 argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("demo",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          720,
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCREEN_WIDTH,
                                             SCREEN_HEIGHT);

    pixels = (u32*)malloc(sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
    z_buffer = (f32*)malloc(sizeof(f32) * SCREEN_WIDTH);

    SDL_Surface* test = IMG_Load("../assets/test.png");

    // Sprite sprite = { {5.0f, 5.0f}, .texture = IMG_Load("../assets/test_sprite.png") }; // Example sprite position


    Player player = {
        .position = {2.0f, 2.0f},
        .direction = {1.0f, 0.0f},
        .camera_plane = {0.0f, 0.66f}
    };

    i32 fps = 0;
    i32 last_time_fps = 0;
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
        move_player(&player, keys);
        

        memset(pixels, 0, sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
        memset(z_buffer, 255, sizeof(u8) * SCREEN_WIDTH);

        render(player, test);

        // render_sprite(player, pixels, sprite);

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);


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