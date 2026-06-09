# Warp Speed Starfield in C

A classic warp-speed starfield — 300 stars in 3-D space fly toward the camera with perspective projection, rendered as glowing filled circles via SDL2.

Far stars appear as tiny blue dots at the centre of the screen; near stars grow large, bright white, and race toward the edges — the classic "jump to hyperspace" visual.

---

# Features

- 3‑D starfield with perspective projection (`x/z`, `y/z`)
- Continuous star recycling — stars that pass the camera respawn at the far field
- Depth-based brightness: blue-tinted far stars → white-hot near stars
- Depth-based size: 1 px (far) to 4 px radius (near)
- 300 stars at ~40 fps
- True 24‑bit color rendering via SDL2 framebuffer
- 960 × 640 window
- Keyboard and Ctrl-C quit handling
- Written entirely in C

---

# How It Works

Each star is a 3‑D point `(sx, sy, sz)` in a cube roughly `[-1, 1]³`. Every frame, `sz` is decremented (the star moves toward the camera). Perspective division `x = sx/sz, y = sy/sz` projects the star onto the screen — near stars (small `sz`) spread outward, creating the warp stretch. When `sz` passes the camera, the star is recycled to a random position at `sz = 1.0`. Each star is drawn as a filled circle whose radius and colour are interpolated from depth.

---

# Tutorial / Rendering Pipeline

## 1. The Star Array

Stars are stored in three parallel `double` arrays — position in world space:

```c
#define N  300

double sx[N], sy[N], sz[N];
```

`sx`, `sy` span roughly `[-1, 1]`; `sz` (depth) runs from `0.001` (at the camera) to `1.0` (far field). Stars are initialised randomly within this volume:

```c
sx[i] = (rand() % 2000 - 1000) / 1000.0;
sy[i] = (rand() % 2000 - 1000) / 1000.0;
sz[i] = (rand() % 1000) / 1000.0 + 0.001;
```

---

## 2. Forward Motion (Warp)

Each frame, every star's depth decreases — it flies toward the camera:

```c
sz[i] -= 0.015;
```

When a star passes the camera plane (`sz ≤ 0.001`), it recycles to a random far-field position, keeping the stream continuous:

```c
if (sz[i] <= 0.001) {
    sx[i] = (rand() % 2000 - 1000) / 1000.0;
    sy[i] = (rand() % 2000 - 1000) / 1000.0;
    sz[i] = 1.0;
}
```

---

## 3. Perspective Projection

3‑D `(sx, sy, sz)` is projected to 2‑D screen coordinates `(cx, cy)` using the classic perspective divide:

```c
double px = sx[i] / sz[i];
double py = sy[i] / sz[i];
int cx = (int)(px * WIN_W / 2 + WIN_W / 2);
int cy = (int)(py * WIN_H / 2 + WIN_H / 2);
```

Dividing by `sz` means near stars (small `sz`) project far from centre, creating the outward rush. Far stars (large `sz`) cluster near the centre. The `WIN_W/2` and `WIN_H/2` scaling maps the `[-1, 1]` projected range to the full window.

---

## 4. Depth-Based Color and Size

The brightness factor `t` is derived from depth:

```c
float t = 1.0f - (float)sz[i];   /* 0 = far, ~1 = near */
int rad = (int)(t * 2.5f) + 1;    /* 1–4 px radius */
```

Radius grows from 1 pixel (far) to 4 pixels (near), giving a sense of approach.

The colour interpolates from dim blue to white-hot:

```c
Uint32 color = SDL_MapRGB(fmt,
    (Uint8)(50  + t * 205),    /*  50 → 255 */
    (Uint8)(80  + t * 175),    /*  80 → 255 */
    (Uint8)(150 + t * 105));   /* 150 → 255 */
```

| Depth      | `t` | Radius | RGB (approx)   | Visual       |
|------------|-----|--------|----------------|--------------|
| Far (1.0)  | 0.0 | 1      | 50, 80, 150   | Dim blue dot |
| Mid (0.5)  | 0.5 | 2      | 152, 167, 202 | Light blue   |
| Near (0.0) | 1.0 | 4      | 255, 255, 255 | White circle |

---

## 5. Circle Rendering

Each star is drawn as a filled circle using a bounding-box scan with a radius-squared test:

```c
for (int dy = -rad; dy <= rad; dy++)
    for (int dx = -rad; dx <= rad; dx++) {
        if (dx * dx + dy * dy > rad * rad) continue;
        int sx = cx + dx, sy = cy + dy;
        if (sx in bounds && sy in bounds)
            pixels[sy * pitch + sx] = color;
    }
```

---

## 6. Background and Clearing

The background is a near-black navy. Each frame clears the pixel buffer row-by-row before drawing stars:

```c
Uint32 col_bg = SDL_MapRGB(fmt, 5, 5, 15);

for (int y = 0; y < WIN_H; y++) {
    Uint32 *row = &pixels[y * pitch];
    for (int x = 0; x < WIN_W; x++) row[x] = col_bg;
}
```

---

## 7. Frame Timing

A 40 fps cap keeps the animation smooth:

```c
Uint32 elapsed = SDL_GetTicks() - frame_start;
if (elapsed < 25) SDL_Delay(25 - elapsed);
```

---

# Build

```
git clone <this-repo>
cd warp
make
```

Or manually:

```
gcc warp.c -o warp -lm $(sdl2-config --cflags --libs)
```

**Dependencies:** SDL2 and a C compiler (`gcc`/`clang`).

Install SDL2 via Homebrew:

```
brew install sdl2
```

Or apt:

```
sudo apt install libsdl2-dev
```

---

# Run

```
./warp
```

Controls:
- **ESC** or **Q** — quit
- **Ctrl-C** — quit (fallback)

---

# Customizing

Edit constants at the top of `warp.c`:

- `N` — star count (more = denser field, slower)
- `WIN_W`, `WIN_H` — window resolution
- `sz[i] -= 0.015` — warp speed (higher = faster)
- Color and radius parameters (`50 + t * 205`, `t * 2.5 + 1`)
- Background colour `col_bg`

---

# Concepts Practiced

- 3‑D perspective projection via homogeneous division
- Depth-based LOD: size and colour scale with `z`
- Continuous particle recycling (object pooling)
- Circle rasterisation with bounding-box optimisation
- SDL2 surface/pixel-buffer graphics
- Real-time animation with frame-rate capping

---

# Dependencies

- `SDL.h` — window management and pixel buffer
- `stdio.h` — stderr diagnostics
- `stdlib.h` — `rand`, `srand`, `NULL`
- `string.h` — `memset`
- `math.h` — `sqrt`
- `time.h` — `time` for RNG seed
- `signal.h` — `SIGINT` handler
