/* warp.c - Warp-speed starfield in the terminal.
 * Build: gcc warp.c -o warp -lm */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define W 80
#define H 40
#define N 200

static volatile int running = 1;
static void stop(int s){ (void)s; running = 0; }

int main(void){
    signal(SIGINT, stop);
    srand((unsigned)time(NULL));
    double sx[N], sy[N], sz[N];
    for (int i = 0; i < N; i++){
        sx[i] = (rand()%2000-1000)/1000.0;
        sy[i] = (rand()%2000-1000)/1000.0;
        sz[i] = (rand()%1000)/1000.0 + 0.001;
    }
    char buf[H][W];
    const char *grad = ".:-=+*#%@";
    printf("\033[2J\033[?25l");
    while (running){
        for (int y=0;y<H;y++) for (int x=0;x<W;x++) buf[y][x]=' ';
        for (int i = 0; i < N; i++){
            sz[i] -= 0.012;
            if (sz[i] <= 0.001){
                sx[i] = (rand()%2000-1000)/1000.0;
                sy[i] = (rand()%2000-1000)/1000.0;
                sz[i] = 1.0;
            }
            double px = sx[i]/sz[i];
            double py = sy[i]/sz[i];
            int x = (int)(px*W/2 + W/2);
            int y = (int)(py*H/2 + H/2);
            if (x>=0 && x<W && y>=0 && y<H){
                int b = (int)((1.0 - sz[i]) * 8);
                if (b<0) b=0; if (b>8) b=8;
                buf[y][x] = grad[b];
            }
        }
        printf("\033[H");
        for (int y=0;y<H;y++){ fwrite(buf[y],1,W,stdout); putchar('\n'); }
        fflush(stdout);
        usleep(40000);
    }
    printf("\033[0m\033[?25h\033[2J\033[H");
    return 0;
}
