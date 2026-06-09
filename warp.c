/*
 * warp.c - Warp-speed starfield (SDL2)
 *
 * 3-D stars fly toward the camera with perspective projection.
 * Near stars are larger, brighter, and whiter; far stars are
 * tiny blue dots.  Rendered as filled circles via SDL2.
 *
 * Build:
 *   gcc warp.c -o warp -lm $(sdl2-config --cflags --libs)
 * Run:
 *   ./warp
 *
 * Controls:
 *   ESC or Q  - quit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <SDL.h>

#define N      300
#define WIN_W  960
#define WIN_H  640

static volatile int running = 1;
static void handle_sigint(int s) { (void)s; running = 0; }

int main(void)
{
    signal(SIGINT, handle_sigint);
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Warp Speed Starfield",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surf = SDL_GetWindowSurface(win);
    SDL_PixelFormat *fmt = surf->format;
    int pitch = surf->pitch / 4;

    double sx[N], sy[N], sz[N];
    for (int i = 0; i < N; i++) {
        sx[i] = (rand() % 2000 - 1000) / 1000.0;
        sy[i] = (rand() % 2000 - 1000) / 1000.0;
        sz[i] = (rand() % 1000) / 1000.0 + 0.001;
    }

    Uint32 col_bg = SDL_MapRGB(fmt, 5, 5, 15);

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
            if (ev.type == SDL_KEYDOWN &&
                (ev.key.keysym.sym == SDLK_ESCAPE ||
                 ev.key.keysym.sym == SDLK_q))
                running = 0;
        }

        for (int i = 0; i < N; i++) {
            sz[i] -= 0.015;
            if (sz[i] <= 0.001) {
                sx[i] = (rand() % 2000 - 1000) / 1000.0;
                sy[i] = (rand() % 2000 - 1000) / 1000.0;
                sz[i] = 1.0;
            }
        }

        if (SDL_MUSTLOCK(surf))
            SDL_LockSurface(surf);

        Uint32 *pixels = (Uint32 *)surf->pixels;

        for (int y = 0; y < WIN_H; y++) {
            Uint32 *row = &pixels[y * pitch];
            for (int x = 0; x < WIN_W; x++)
                row[x] = col_bg;
        }

        for (int i = 0; i < N; i++) {
            double px = sx[i] / sz[i];
            double py = sy[i] / sz[i];
            int cx = (int)(px * WIN_W / 2 + WIN_W / 2);
            int cy = (int)(py * WIN_H / 2 + WIN_H / 2);

            if (cx < 0 || cx >= WIN_W || cy < 0 || cy >= WIN_H)
                continue;

            float t = 1.0f - (float)sz[i];
            int rad = (int)(t * 2.5f) + 1;

            Uint32 color = SDL_MapRGB(fmt,
                (Uint8)(50  + t * 205),
                (Uint8)(80  + t * 175),
                (Uint8)(150 + t * 105));

            for (int dy = -rad; dy <= rad; dy++) {
                for (int dx = -rad; dx <= rad; dx++) {
                    if (dx * dx + dy * dy > rad * rad) continue;
                    int sx = cx + dx, sy = cy + dy;
                    if (sx >= 0 && sx < WIN_W && sy >= 0 && sy < WIN_H)
                        pixels[sy * pitch + sx] = color;
                }
            }
        }

        if (SDL_MUSTLOCK(surf))
            SDL_UnlockSurface(surf);

        SDL_UpdateWindowSurface(win);

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < 25)
            SDL_Delay(25 - elapsed);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
