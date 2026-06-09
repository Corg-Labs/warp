CC     = gcc
CFLAGS = -O3 -Wall
LDLIBS = -lm
SDL    = $(shell sdl2-config --cflags --libs)

warp: warp.c
	$(CC) $(CFLAGS) warp.c -o warp $(SDL) $(LDLIBS)

run: warp
	./warp

clean:
	rm -f warp

.PHONY: run clean
