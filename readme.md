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
