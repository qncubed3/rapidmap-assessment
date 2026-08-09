#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

#define IN_PATH      "polys.pol"
#define WIDTH        4096
#define HEIGHT       4096
#define BATCH_SIZE   10000
#define WRITE_OUTPUT 0
#define OUT_PATH     "out_polys.grid"

static int point_to_pixel(float lon, float lat, Header h, int *x, int *y) {
    *x = (int)((lon - h.min_lon) / (h.max_lon - h.min_lon) * WIDTH);
    *y = (int)((h.max_lat - lat) / (h.max_lat - h.min_lat) * HEIGHT);

    if (*x < 0 || *x >= WIDTH || *y < 0 || *y >= HEIGHT) {
        return 0;
    }
    return 1;
}

static void set_pixel(unsigned char *grid, int x, int y, unsigned long long *pixels_on) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }
    if (grid[y * WIDTH + x] == 0) {
        *pixels_on = *pixels_on + 1;
    }
    grid[y * WIDTH + x] = 1;
}

/* Edge function: > 0 means (px,py) is on the left of directed edge a->b. */
static int edge(int ax, int ay, int bx, int by, int px, int py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

/*
 * Fill the triangle (inclusive). Walks the bounding box and turns on
 * every pixel that is inside (same side of all three edges).
 */
static void fill_triangle(unsigned char *grid,
                          int x0, int y0, int x1, int y1, int x2, int y2,
                          unsigned long long *pixels_on) {
    int min_x = x0;
    int max_x = x0;
    int min_y = y0;
    int max_y = y0;

    if (x1 < min_x) min_x = x1;
    if (x1 > max_x) max_x = x1;
    if (y1 < min_y) min_y = y1;
    if (y1 > max_y) max_y = y1;
    if (x2 < min_x) min_x = x2;
    if (x2 > max_x) max_x = x2;
    if (y2 < min_y) min_y = y2;
    if (y2 > max_y) max_y = y2;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= WIDTH) max_x = WIDTH - 1;
    if (max_y >= HEIGHT) max_y = HEIGHT - 1;

    /* Orient so the triangle is counter-clockwise for a consistent inside test. */
    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) {
        return; /* degenerate (all points colinear) */
    }
    if (area < 0) {
        int tx = x1, ty = y1;
        x1 = x2;
        y1 = y2;
        x2 = tx;
        y2 = ty;
    }

    int y, x;
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            int w0 = edge(x1, y1, x2, y2, x, y);
            int w1 = edge(x2, y2, x0, y0, x, y);
            int w2 = edge(x0, y0, x1, y1, x, y);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                set_pixel(grid, x, y, pixels_on);
            }
        }
    }
}

int main(void) {
    FILE *in = fopen(IN_PATH, "rb");
    if (!in) {
        perror("fopen");
        return 1;
    }

    Header header;
    fread(&header, sizeof(Header), 1, in);
    if (header.magic != MAGIC_POLYS) {
        fprintf(stderr, "bad magic\n");
        fclose(in);
        return 1;
    }

    unsigned char *grid = malloc((size_t)WIDTH * (size_t)HEIGHT);
    if (!grid) {
        fprintf(stderr, "malloc grid failed\n");
        fclose(in);
        return 1;
    }
    memset(grid, 0, (size_t)WIDTH * (size_t)HEIGHT);

    Triangle batch[BATCH_SIZE];
    unsigned long long done = 0;
    unsigned long long pixels_on = 0;

    clock_t t0 = clock();

    while (done < header.count) {
        unsigned long long n = header.count - done;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }

        fread(batch, sizeof(Triangle), (size_t)n, in);

        unsigned long long i;
        for (i = 0; i < n; i++) {
            int x0, y0, x1, y1, x2, y2;
            int ok0 = point_to_pixel(batch[i].lon1, batch[i].lat1, header, &x0, &y0);
            int ok1 = point_to_pixel(batch[i].lon2, batch[i].lat2, header, &x1, &y1);
            int ok2 = point_to_pixel(batch[i].lon3, batch[i].lat3, header, &x2, &y2);

            if (!ok0 || !ok1 || !ok2) {
                continue;
            }

            fill_triangle(grid, x0, y0, x1, y1, x2, y2, &pixels_on);
        }

        done = done + n;
    }

    clock_t t1 = clock();
    fclose(in);

    double seconds = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
    fprintf(stderr, "compute: %llu triangles -> %dx%d grid in %.3f s (%llu pixels on)\n",
            (unsigned long long)header.count, WIDTH, HEIGHT, seconds, pixels_on);

    if (WRITE_OUTPUT) {
        FILE *out = fopen(OUT_PATH, "wb");
        fwrite(grid, 1, (size_t)WIDTH * (size_t)HEIGHT, out);
        fclose(out);
        fprintf(stderr, "wrote raw grid to %s\n", OUT_PATH);
    }

    free(grid);
    return 0;
}
