#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "utils.h"
#include "player.h"
#include "render.h"
#include "entity.h"

u8 map[MAPSIZE_R][MAPSIZE_C] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

f32* z_buffer;
u32* pixels;


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
    SDL_Surface* test_sprite = IMG_Load("../assets/test_sprite.png");

    Player player = {
        .position = {2.0f, 2.0f},
        .direction = {1.0f, 0.0f},
        .camera_plane = {0.0f, 0.66f}
    };

    LinkedList* entities = ll_init();

    u32 last_shoot = -1;
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


        //TODO
        if (SDL_GetTicks() - last_shoot > 400 && keys[SDL_SCANCODE_SPACE]) {
            Entity* bullet = (Entity*)malloc(sizeof(Entity)); 
            bullet->position = (v2){player.position.x, player.position.y};
            bullet->texture = test_sprite;
            ll_push_back(entities, bullet);
            last_shoot = SDL_GetTicks();
        }

        memset(pixels, 0, sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
        memset(z_buffer, 255, sizeof(u8) * SCREEN_WIDTH);

        render(player, test, entities);

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
    SDL_FreeSurface(test_sprite);
    free(z_buffer);
    free(pixels);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}