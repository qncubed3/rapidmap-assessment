#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

#define IN_PATH      "points.pts"
#define WIDTH        16384
#define HEIGHT       16384
#define BATCH_SIZE   10000
#define WRITE_OUTPUT 0              /* 1 = dump raw on/off grid, 0 = skip (for timing) */
#define OUT_PATH     "out.grid"

/* lon/lat -> pixel. Returns 1 if inside the grid, else 0. */
static int point_to_pixel(Point p, Header h, int *x, int *y) {
    *x = (int)((p.lon - h.min_lon) / (h.max_lon - h.min_lon) * WIDTH);
    *y = (int)((h.max_lat - p.lat) / (h.max_lat - h.min_lat) * HEIGHT);

    if (*x < 0 || *x >= WIDTH || *y < 0 || *y >= HEIGHT) {
        return 0;
    }
    return 1;
}

int main(void) {

    // Read in points file
    FILE *in = fopen(IN_PATH, "rb");
    if (!in) {
        perror("fopen");
        return 1;
    }

    // Read in header
    Header header;
    fread(&header, sizeof(Header), 1, in);
    if (header.magic != MAGIC_POINTS) {
        fprintf(stderr, "bad magic\n");
        return 1;
    }

    /* on/off grid: 0 = empty, 1 = hit. Points are streamed in batches. */
    unsigned char *grid = malloc(WIDTH * HEIGHT);
    memset(grid, 0, WIDTH * HEIGHT);

    // Initialize batch
    Point batch[BATCH_SIZE];
    unsigned long long done = 0;
    unsigned long long pixels_on = 0;

    /* Timed section: read batches + project + stamp. No image encode. */
    clock_t t0 = clock();

    while (done < header.count) {
        unsigned long long n = header.count - done;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }

        fread(batch, sizeof(Point), (size_t)n, in);

        // Rasterise points
        unsigned long long i;
        for (i = 0; i < n; i++) {

            // Convert point to pixel
            int x, y;
            if (point_to_pixel(batch[i], header, &x, &y)) {
                // Set computed pixel status
                if (grid[y * WIDTH + x] == 0) {
                    pixels_on = pixels_on + 1;
                }
                grid[y * WIDTH + x] = 1; /* on */
            }
        }

        done = done + n;
    }

    clock_t t1 = clock();
    fclose(in);

    double seconds = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
    fprintf(stderr, "compute: %llu points -> %dx%d grid in %.3f s (%llu pixels on)\n",
            (unsigned long long)header.count, WIDTH, HEIGHT, seconds, pixels_on);

    if (WRITE_OUTPUT) {
        FILE *out = fopen(OUT_PATH, "wb");
        fwrite(grid, 1, WIDTH * HEIGHT, out);
        fclose(out);
        fprintf(stderr, "wrote raw grid to %s\n", OUT_PATH);
    }

    free(grid);
    return 0;
}
