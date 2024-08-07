#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct _v2_f32 {
    f32 x;
    f32 y;
} v2;

typedef struct _v2_i32 {
    i32 x;
    i32 y;
} v2i;

#define ASSERT(_e, ...)                                                                            \
    if (!(_e)) {                                                                                   \
        fprintf(stderr, __VA_ARGS__);                                                              \
        exit(1);                                                                                   \
    }
#define MIN(_a, _b)                                                                                \
    ({                                                                                             \
        __typeof__(_a) __a = (_a), __b = (_b);                                                     \
        __a < __b ? __a : __b;                                                                     \
    })
#define MAX(_a, _b)                                                                                \
    ({                                                                                             \
        __typeof__(_a) __a = (_a), __b = (_b);                                                     \
        __a > __b ? __a : __b;                                                                     \
    })
#define CLIP(_x, _min, _max) (MIN(MAX(_x, _min), _max))
static inline f32 dot(v2 v0, v2 v1) { return (v0.x * v1.x) + (v0.y * v1.y); }
static inline f32 length(v2 vl) { return sqrtf(dot(vl, vl)); }
static inline f32 dist(v2 v0, v2 v1) { return length((v2) { .x = v1.x - v0.x, .y = v1.y - v0.y }); }

#define SCREEN_WIDTH 365
#define SCREEN_HEIGHT 235

#define MOVEMENT_SPEED 2.33f
#define ROTATION_SPEED 2.33f

#define MAPSIZE_R 12
#define MAPSIZE_C 8

#define TEXTURE_COUNT 10
const char* texure_names[TEXTURE_COUNT] = {
    "assets/_missing.png", "assets/wall0.png",  "assets/wall1.png",    "assets/wall2.png",
    "assets/floor1.png",   "assets/floor2.png", "assets/ceiling1.png", "assets/ceiling2.png",
    "assets/sprite.png",   "assets/bullet.png",
};

const u8 map[MAPSIZE_R][MAPSIZE_C] = {
    { 1, 1, 1, 1, 1, 1, 1, 2 },
    { 1, 5, 5, 5, 5, 5, 5, 2 },
    { 1, 5, 5, 5, 5, 5, 5, 2 },
    { 1, 5, 5, 5, 5, 5, 5, 2 },
    { 1, 5, 5, 5, 2, 7, 7, 2 },
    { 1, 7, 7, 3, 3, 7, 7, 2 },
    { 1, 7, 7, 7, 7, 7, 7, 2 },
    { 1, 7, 7, 7, 7, 7, 7, 2 },
    { 1, 7, 7, 7, 7, 7, 7, 2 },
    { 1, 7, 7, 7, 7, 7, 7, 2 },
    { 1, 7, 7, 7, 7, 7, 7, 2 },
    { 3, 3, 3, 3, 3, 3, 3, 2 }
};

typedef struct cell_t {
    bool is_wall;
    u8 wall_tex, floor_tex, ceili_tex;
} cell_t;

const cell_t cell_types[] = {
    [1] = { .is_wall = true, .wall_tex = 1, },
    [2] = { .is_wall = true, .wall_tex = 2, },
    [3] = { .is_wall = true, .wall_tex = 3, },
    [4] = { .is_wall = false, .floor_tex = 4, .ceili_tex = 6, },
    [5] = { .is_wall = false, .floor_tex = 5, .ceili_tex = 6, },
    [6] = { .is_wall = false, .floor_tex = 4, .ceili_tex = 7, },
    [7] = { .is_wall = false, .floor_tex = 5, .ceili_tex = 7, },
};

#define MAX_SPRITES 10
typedef struct sprite_t {
    v2 position;
    u8 tex;
} sprite_t;

static struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;
    SDL_Surface* game_textures[TEXTURE_COUNT];
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    f32 z_buffer[SCREEN_WIDTH];

    struct player {
        v2 position, direction, camera_plane;
    } player;

    struct {
        sprite_t arr[MAX_SPRITES];
        u32 count;
    } sprites;
} state;

int distance_to_player_cmp(const void* a, const void* b) {
    v2 a_v2 = *(v2*)a;
    v2 b_v2 = *(v2*)b;

    f32 a_dist = dist(state.player.position, a_v2);
    f32 b_dist = dist(state.player.position, b_v2);

    if (a_dist < b_dist)
        return -1;
    else if (a_dist > b_dist)
        return 1;
    return 0;
}

void rotate_player(f32 angle) {
    v2 old_direction = { state.player.direction.x, state.player.direction.y };
    v2 old_camera_plane = { state.player.camera_plane.x, state.player.camera_plane.y };

    state.player.direction.x = old_direction.x * cosf(angle) - old_direction.y * sinf(angle);
    state.player.direction.y = old_direction.x * sinf(angle) + old_direction.y * cosf(angle);

    state.player.camera_plane.x = old_camera_plane.x * cosf(angle) -
                                  old_camera_plane.y * sinf(angle);
    state.player.camera_plane.y = old_camera_plane.x * sinf(angle) +
                                  old_camera_plane.y * cosf(angle);
}

void calculate_line_draw_start_end(i32* draw_start, i32* draw_end, f32 perpendicular_wall_dist) {
    i32 line_height = (i32)(SCREEN_HEIGHT / perpendicular_wall_dist);
    *draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
    *draw_end = line_height / 2 + SCREEN_HEIGHT / 2;
}

u32 get_argb_from_position(f32 x, f32 y, SDL_Surface* map1) {
    SDL_LockSurface(map1);
    i32 x_index = MIN((i32)(x * map1->w), map1->w);
    i32 y_index = MIN((i32)(y * map1->h), map1->h);

    SDL_PixelFormat* pixelFormat = map1->format;

    i32 pixelIndex = (y_index * map1->pitch) + (x_index * pixelFormat->BytesPerPixel);

    u8* _pixels = (u8*)map1->pixels;

    u8 red, green, blue, alpha;
    SDL_GetRGBA(*(u32*)(_pixels + pixelIndex), pixelFormat, &red, &green, &blue, &alpha);
    SDL_UnlockSurface(map1);

    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void render_sprite(sprite_t* sprite) {

    // sprite position relative to the player
    f32 sprite_x = sprite->position.x - state.player.position.x;
    f32 sprite_y = sprite->position.y - state.player.position.y;

    // transform sprite with the inverse camera matrix
    f32 inv_det = 1.0 / (state.player.camera_plane.x * state.player.direction.y -
                         state.player.direction.x * state.player.camera_plane.y);
    f32 transform_x = inv_det *
                      (state.player.direction.y * sprite_x - state.player.direction.x * sprite_y);
    f32 transform_y = inv_det * (-state.player.camera_plane.y * sprite_x +
                                 state.player.camera_plane.x * sprite_y);

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
        f32 tex_x = (f32)(stripe - draw_start_x) / (f32)(draw_end_x - draw_start_x);
        if (transform_y > 0 && stripe > 0 && stripe < SCREEN_WIDTH &&
            transform_y < state.z_buffer[stripe]) {
            i32 actual_draw_start_y = CLIP(draw_start_y, 0, SCREEN_HEIGHT);
            i32 actual_draw_end_y = CLIP(draw_end_y, 0, SCREEN_HEIGHT);

            for (i32 y = actual_draw_start_y; y < actual_draw_end_y; y++) {

                f32 tex_y = (f32)(y - draw_start_y) / (f32)(draw_end_y - draw_start_y);
                u32 color = get_argb_from_position(tex_x, tex_y, state.game_textures[sprite->tex]);

                if ((color & 0xFFFFFFFF) != 0) {
                    // state.z_buffer[stripe] = transform_y;
                    state.pixels[y * SCREEN_WIDTH + stripe] = color;
                }
            }
        }
    }
}

void render() {
    for (i32 x = 0; x < SCREEN_WIDTH; x++) {
        f32 camera_x = 2 * (x / (f32)SCREEN_WIDTH) - 1;

        const v2 ray = {
            .x = state.player.direction.x + state.player.camera_plane.x * camera_x,
            .y = state.player.direction.y + state.player.camera_plane.y * camera_x,
        };

        v2i map_pos = { (i32)state.player.position.x, (i32)state.player.position.y };

        v2 side_dist;
        v2 delta_dist = { fabsf(1 / ray.x), fabsf(1 / ray.y) };
        v2 step;

        if (ray.x < 0) {
            step.x = -1;
            side_dist.x = (state.player.position.x - map_pos.x) * delta_dist.x;
        } else {
            step.x = 1;
            side_dist.x = (map_pos.x + 1.0f - state.player.position.x) * delta_dist.x;
        }
        if (ray.y < 0) {
            step.y = -1;
            side_dist.y = (state.player.position.y - map_pos.y) * delta_dist.y;
        } else {
            step.y = 1;
            side_dist.y = (map_pos.y + 1.0f - state.player.position.y) * delta_dist.y;
        }

        bool hit = false;
        i32 side;

        while (!hit) {
            if (side_dist.x < side_dist.y) {
                side_dist.x += delta_dist.x;
                map_pos.x += step.x;
                side = 0;
            } else {
                side_dist.y += delta_dist.y;
                map_pos.y += step.y;
                side = 1;
            }

            if (cell_types[map[map_pos.y][map_pos.x]].is_wall) {
                hit = true;
            }
        }

        f32 perpendicular_wall_dist;
        if (side == 0) {
            perpendicular_wall_dist = (map_pos.x - state.player.position.x + (1 - step.x) / 2) /
                                      ray.x;
        } else {
            perpendicular_wall_dist = (map_pos.y - state.player.position.y + (1 - step.y) / 2) /
                                      ray.y;
        }

        f32 cell_dist = 0;
        if (side != 0) {
            cell_dist = state.player.position.x + perpendicular_wall_dist * ray.x;
        } else {
            cell_dist = state.player.position.y + perpendicular_wall_dist * ray.y;
        }
        cell_dist = cell_dist - (i32)cell_dist;

        i32 draw_start, draw_end;
        calculate_line_draw_start_end(&draw_start, &draw_end, perpendicular_wall_dist);

        float wall_x;
        if (side == 0) {
            wall_x = state.player.position.y + perpendicular_wall_dist * ray.y;
        } else {
            wall_x = state.player.position.x + perpendicular_wall_dist * ray.x;
        }
        wall_x -= floor(wall_x);

        float floor_x_wall, floor_y_wall;
        if (side == 0 && ray.x > 0) {
            floor_x_wall = map_pos.x;
            floor_y_wall = map_pos.y + wall_x;
        } else if (side == 0 && ray.x < 0) {
            floor_x_wall = map_pos.x + 1.0;
            floor_y_wall = map_pos.y + wall_x;
        } else if (side == 1 && ray.y > 0) {
            floor_x_wall = map_pos.x + wall_x;
            floor_y_wall = map_pos.y;
        } else {
            floor_x_wall = map_pos.x + wall_x;
            floor_y_wall = map_pos.y + 1.0;
        }

        float dist_wall, dist_player, current_dist;
        dist_wall = perpendicular_wall_dist;
        dist_player = 0.0;

        if (draw_end < 0)
            draw_end = SCREEN_HEIGHT;

        SDL_Surface* floor_texture = NULL;
        SDL_Surface* ceiling_texture = NULL;
        for (i32 y = draw_end; y < SCREEN_HEIGHT; y++) {
            current_dist = SCREEN_HEIGHT / (2.0 * y - SCREEN_HEIGHT);
            float weight = (current_dist - dist_player) / (dist_wall - dist_player);
            float current_floor_x = weight * floor_x_wall +
                                    (1.0 - weight) * state.player.position.x;
            float current_floor_y = weight * floor_y_wall +
                                    (1.0 - weight) * state.player.position.y;

            i32 map_x = (i32)floorf(current_floor_x);
            i32 map_y = (i32)floorf(current_floor_y);

            map_x = CLIP(map_x, 0, MAPSIZE_C - 1);
            map_y = CLIP(map_y, 0, MAPSIZE_R - 1);

            u8 cell_id = map[map_y][map_x];

            floor_texture = state.game_textures[cell_types[cell_id].floor_tex];
            ceiling_texture = state.game_textures[cell_types[cell_id].ceili_tex];

            if (cell_types[cell_id].is_wall) {
                floor_texture = state.game_textures[cell_types[cell_id].wall_tex];
            }

            i32 floor_tex_x = (i32)(current_floor_x * floor_texture->w) % floor_texture->w;
            i32 floor_tex_y = (i32)(current_floor_y * floor_texture->h) % floor_texture->h;

            u32 floor_color = get_argb_from_position((float)floor_tex_x / floor_texture->w,
                                                     (float)floor_tex_y / floor_texture->h,
                                                     floor_texture);

            u32 ceiling_color = get_argb_from_position((float)floor_tex_x / ceiling_texture->w,
                                                       (float)floor_tex_y / ceiling_texture->h,
                                                       ceiling_texture);

            state.pixels[y * SCREEN_WIDTH + x] = floor_color;
            state.pixels[(SCREEN_HEIGHT - y) * SCREEN_WIDTH + x] = ceiling_color;
        }

        i32 actual_start = CLIP(draw_start, 0, SCREEN_HEIGHT);
        i32 actual_end = CLIP(draw_end, 0, SCREEN_HEIGHT);

        for (i32 y = actual_start; y < actual_end; y++) {

            u32 color = get_argb_from_position(
                    cell_dist,
                    (f32)(y - draw_start) / (f32)(draw_end - draw_start),
                    state.game_textures[cell_types[map[map_pos.y][map_pos.x]].wall_tex]);

            if (side == 1) {
                u32 r = ((color & 0x00FF0000) * 0x90) >> 8, g = ((color & 0x0000FF00) * 0x90) >> 8,
                    b = ((color & 0x000000FF) * 0x90) >> 8;

                color = 0xFF000000 | (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);
            }

            state.pixels[(y * SCREEN_WIDTH) + x] = color;
        }
        state.z_buffer[x] = perpendicular_wall_dist;
    }
}

i32 main(i32 argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);
    state.window = SDL_CreateWindow("demo",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    1280,
                                    720,
                                    SDL_WINDOW_ALLOW_HIGHDPI);

    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_SOFTWARE);
    state.screen_texture = SDL_CreateTexture(state.renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCREEN_WIDTH,
                                             SCREEN_HEIGHT);

    for (i32 i = 0; i < TEXTURE_COUNT; i++) {
        state.game_textures[i] = IMG_Load(texure_names[i]);
        ASSERT(state.game_textures[i] != NULL, "Couldn't load texture %s.", texure_names[i]);
    }

    state.player = (struct player) {
        .camera_plane = { 0.0f, 0.66f },
        .direction = { 1.0f, 0.0f  },
        .position = { 2.0f, 2.0f  }
    };

    state.sprites.arr[0] = (sprite_t) {
        (v2) { 2.4f, 3.6f },
        8
    };
    state.sprites.arr[1] = (sprite_t) {
        (v2) { 2.3f, 3.7f },
        8
    };
    state.sprites.count = 2;

    u32 fps = 0, last_time_fps = 0, current_time = 0, last_time = 0;
    f32 delta_time = 0.0f;
    char buffer[32];
    bool running = true;
    SDL_Event event;

    while (running) {
        // FPS
        fps++;
        if (SDL_GetTicks() - last_time_fps > 1000) {
            snprintf(buffer, sizeof(buffer), "demo [%d]", fps);
            SDL_SetWindowTitle(state.window, buffer);
            fps = 0;
            last_time_fps = SDL_GetTicks();
        }
        current_time = SDL_GetTicks();
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }
        const u8* keys = SDL_GetKeyboardState(NULL);
        v2 pos_delta = { 0.0f, 0.0f };

        if (keys[SDL_SCANCODE_LEFT]) {
            rotate_player(-ROTATION_SPEED * delta_time);
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            rotate_player(ROTATION_SPEED * delta_time);
        }
        if (keys[SDL_SCANCODE_UP]) {
            pos_delta.x = state.player.direction.x * MOVEMENT_SPEED * delta_time;
            pos_delta.y = state.player.direction.y * MOVEMENT_SPEED * delta_time;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            pos_delta.x = -state.player.direction.x * MOVEMENT_SPEED * delta_time;
            pos_delta.y = -state.player.direction.y * MOVEMENT_SPEED * delta_time;
        }

        // check collisions
        u8 cell_id = map[(i32)(state.player.position.y)]
                        [(i32)(state.player.position.x + pos_delta.x)];
        if (cell_types[cell_id].is_wall) {
            pos_delta.x = 0.0f;
        }
        cell_id = map[(i32)(state.player.position.y + pos_delta.y)][(i32)(state.player.position.x)];
        if (cell_types[cell_id].is_wall) {
            pos_delta.y = 0.0f;
        }
        state.player.position.x += pos_delta.x;
        state.player.position.y += pos_delta.y;

        // sort sprites by distance to player
        qsort(state.sprites.arr, state.sprites.count, sizeof(sprite_t), distance_to_player_cmp);

        memset(state.pixels, 0, sizeof(u32) * SCREEN_WIDTH * SCREEN_HEIGHT);
        memset(state.z_buffer, 255, sizeof(u8) * SCREEN_WIDTH);

        render();
        for (i32 i = 0; i < state.sprites.count; i++) {
            render_sprite(&state.sprites.arr[i]);
        }

        SDL_RenderClear(state.renderer);
        SDL_UpdateTexture(state.screen_texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(state.renderer, state.screen_texture, NULL, NULL, 0.0, NULL, 0);
        SDL_RenderPresent(state.renderer);
    }

    // cleanup
    for (i32 i = 0; i < TEXTURE_COUNT; i++) {
        SDL_FreeSurface(state.game_textures[i]);
    }

    SDL_DestroyTexture(state.screen_texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();

    return 0;
}