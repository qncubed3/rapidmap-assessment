#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

#define IN_PATH      "lines.lns"
#define WIDTH        4096
#define HEIGHT       4096
#define BATCH_SIZE   1000
#define WRITE_OUTPUT 0
#define OUT_PATH     "out_lines.grid"

/* lon/lat -> pixel. Returns 1 if inside the grid, else 0. */
static int point_to_pixel(float lon, float lat, Header h, int *x, int *y) {
    *x = (int)((lon - h.min_lon) / (h.max_lon - h.min_lon) * WIDTH);
    *y = (int)((h.max_lat - lat) / (h.max_lat - h.min_lat) * HEIGHT);

    if (*x < 0 || *x >= WIDTH || *y < 0 || *y >= HEIGHT) {
        return 0;
    }
    return 1;
}

/* Turn one grid cell on (and count it the first time). */
static void set_pixel(unsigned char *grid, int x, int y, unsigned long long *pixels_on) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }
    if (grid[y * WIDTH + x] == 0) {
        *pixels_on = *pixels_on + 1;
    }
    grid[y * WIDTH + x] = 1;
}

/* Bresenham: walk from (x0,y0) to (x1,y1) and light each pixel on the path. */
static void draw_line(unsigned char *grid, int x0, int y0, int x1, int y1,
                      unsigned long long *pixels_on) {
    
    // Calculate absolute differences
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    // Determine step direction
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // Initialize error margin
    int err = dx - dy;

    for (;;) {
        set_pixel(grid, x0, y0, pixels_on);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;

        // Adjust x coordinate and error margin
        if (e2 > -dy) {
            err = err - dy;
            x0 = x0 + sx;
        }

        // Adjust y coordinate and error margin
        if (e2 < dx) {
            err = err + dx;
            y0 = y0 + sy;
        }
    }
}

int main(void) {
    /* Open the lines file. */
    FILE *in = fopen(IN_PATH, "rb");
    if (!in) {
        perror("fopen");
        return 1;
    }

    /* Read header and check this is really a lines file. */
    Header header;
    fread(&header, sizeof(Header), 1, in);
    if (header.magic != MAGIC_LINES) {
        fprintf(stderr, "bad magic\n");
        fclose(in);
        return 1;
    }

    /* Empty on/off grid in RAM. */
    unsigned char *grid = malloc((size_t)WIDTH * (size_t)HEIGHT);
    if (!grid) {
        fprintf(stderr, "malloc grid failed\n");
        fclose(in);
        return 1;
    }
    memset(grid, 0, (size_t)WIDTH * (size_t)HEIGHT);

    Line batch[BATCH_SIZE];
    unsigned long long done = 0;
    unsigned long long pixels_on = 0;

    /* Timed: read batches, project endpoints, draw lines. */
    clock_t t0 = clock();

    while (done < header.count) {
        /* How many lines to read this round (at most BATCH_SIZE). */
        unsigned long long n = header.count - done;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }

        fread(batch, sizeof(Line), (size_t)n, in);

        /* Rasterise each line in the batch. */
        unsigned long long i;
        for (i = 0; i < n; i++) {
            int x0, y0, x1, y1;
            int ok0 = point_to_pixel(batch[i].lon1, batch[i].lat1, header, &x0, &y0);
            int ok1 = point_to_pixel(batch[i].lon2, batch[i].lat2, header, &x1, &y1);

            /* Skip if either endpoint is outside the grid. */
            if (!ok0 || !ok1) {
                continue;
            }

            draw_line(grid, x0, y0, x1, y1, &pixels_on);
        }

        done = done + n;
    }

    clock_t t1 = clock();
    fclose(in);

    double seconds = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
    fprintf(stderr, "compute: %llu lines -> %dx%d grid in %.3f s (%llu pixels on)\n",
            (unsigned long long)header.count, WIDTH, HEIGHT, seconds, pixels_on);

    /* Optional: dump the raw grid to disk (off by default). */
    if (WRITE_OUTPUT) {
        FILE *out = fopen(OUT_PATH, "wb");
        fwrite(grid, 1, (size_t)WIDTH * (size_t)HEIGHT, out);
        fclose(out);
        fprintf(stderr, "wrote raw grid to %s\n", OUT_PATH);
    }

    free(grid);
    return 0;
}
