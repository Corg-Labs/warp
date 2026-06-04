# Starfield Warp

Warp-speed starfield animation in the terminal, in C.

A small, self-contained demo written in **pure C** — no external libraries,
just the standard library and POSIX. Part of the [Corg-Labs](https://github.com/Corg-Labs)
collection of single-file C programs.

---

## How It Works

1. Stars are stored in 3D and pulled toward the camera each frame
2. Perspective projection makes near stars spread outward fast
3. Brightness is mapped to a character ramp by depth
4. Stars that pass the camera respawn far away

---

# Tutorial

This tutorial walks through the entire source (`warp.c`) step by step, explaining
the data model, the physics, the projection math, and the rendering pipeline.

## 1. Configuration Constants

Three `#define` constants set the canvas size and the number of stars:

```c
#define W 80
#define H 40
#define N 200
```

`W` and `H` are the terminal columns and rows used for drawing. `N` is the
star count. All three can be tweaked — increase `N` for a denser field or
adjust `W`/`H` to match a larger terminal.

## 2. Star Data Structure

Each star is represented by a 3-D coordinate stored in three parallel arrays:

```c
double sx[N], sy[N], sz[N];
```

`sx` and `sy` are the horizontal and vertical positions in "world space",
ranging roughly from −1.0 to +1.0. `sz` is depth: values close to 1.0 mean
the star is far away; values close to 0.0 mean it is almost at the camera.
Stars are initialised with random `sx`/`sy` and a random positive `sz`:

```c
sx[i] = (rand()%2000-1000)/1000.0;
sy[i] = (rand()%2000-1000)/1000.0;
sz[i] = (rand()%1000)/1000.0 + 0.001;
```

## 3. The Warp Effect — Moving Stars Toward the Camera

Every frame each star's depth is decremented by a fixed step:

```c
sz[i] -= 0.012;
```

This simulates flying forward through space. When a star passes the camera
(`sz` falls to or below `0.001`), it is recycled to a random position far away:

```c
if (sz[i] <= 0.001){
    sx[i] = (rand()%2000-1000)/1000.0;
    sy[i] = (rand()%2000-1000)/1000.0;
    sz[i] = 1.0;
}
```

Resetting `sz` to `1.0` puts the star at the far end of the field, so the
stream is continuous.

## 4. Perspective Projection

Real 3-D perspective divides the world-space `x` and `y` coordinates by depth
`z`. The smaller `z` is, the larger the projected position — near stars appear
to fly toward the edges of the screen:

```c
double px = sx[i]/sz[i];
double py = sy[i]/sz[i];
int x = (int)(px*W/2 + W/2);
int y = (int)(py*H/2 + H/2);
```

Multiplying by `W/2` (or `H/2`) maps the [-1, 1] projected range to half the
canvas width/height, and adding `W/2` (or `H/2`) centres the origin in the
middle of the terminal.

## 5. Brightness Mapping via a Character Gradient

Depth is also used to choose the character that represents the star. The
closer the star, the brighter the glyph:

```c
const char *grad = ".:-=+*#%@";
...
int b = (int)((1.0 - sz[i]) * 8);
if (b<0) b=0; if (b>8) b=8;
buf[y][x] = grad[b];
```

`1.0 - sz[i]` converts depth to brightness: a value near 0 (far star) maps to
index 0 (`'.'`), and a value near 1 (close star) maps to index 8 (`'@'`). The
clamp guards against floating-point edge cases.

## 6. Double-Buffered Rendering

To avoid flicker the program writes into a character buffer first, then blits
the whole frame at once:

```c
char buf[H][W];
...
for (int y=0;y<H;y++) for (int x=0;x<W;x++) buf[y][x]=' ';
/* ... fill buf with star glyphs ... */
printf("\033[H");
for (int y=0;y<H;y++){ fwrite(buf[y],1,W,stdout); putchar('\n'); }
fflush(stdout);
```

`\033[2J` clears the terminal once at startup, then each frame uses `\033[H`
(cursor to home) to overwrite in place — no scrolling, no clear-screen flash.
`fwrite` writes one full row per call for efficient I/O.

## 7. Frame Rate and Clean Exit

`usleep(40000)` pauses for 40 ms between frames, giving roughly 25 fps.
A `SIGINT` handler flips a `volatile` flag so the loop exits cleanly:

```c
static volatile int running = 1;
static void stop(int s){ (void)s; running = 0; }
...
signal(SIGINT, stop);
...
while (running){ ... usleep(40000); }
printf("\033[0m\033[?25h\033[2J\033[H");
```

On exit the terminal is restored: `\033[0m` resets attributes, `\033[?25h`
shows the cursor again, and `\033[2J\033[H` clears the screen.

---

## Build

```
gcc warp.c -o warp -lm
```

## Run

```
./warp
```

## Controls

Press **Ctrl-C** to quit.
