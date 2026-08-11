#include "rasterise_lines.h"
#include "../common.h"

/* Draw a line segment with Bresenham's algorithm */
static void draw_line(unsigned char* grid, int width, int height,
                      int x0, int y0, int x1, int y1,
                      unsigned long long* pixels_on) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx, sy, err;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = dx - dy;

    for (;;) {
        set_pixel(grid, width, height, x0, y0, pixels_on);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        {
            int e2 = 2 * err;
            if (e2 > -dy) {
                err = err - dy;
                x0 = x0 + sx;
            }
            if (e2 < dx) {
                err = err + dx;
                y0 = y0 + sy;
            }
        }
    }
}

/* Rasterise lines from a .lns file onto a grayscale grid */
int rasterise_lines(const char* in_path, unsigned char* grid,
                    int width, int height,
                    unsigned long long* out_count,
                    unsigned long long* out_pixels_on) {
    FILE* in;
    struct Header header;
    struct Line batch[BATCH_SIZE];
    unsigned long long done;
    unsigned long long pixels_on;

    in = fopen(in_path, "rb");
    if (!in) {
        perror("fopen");
        return 1;
    }

    if (fread(&header, sizeof(header), 1, in) != 1) {
        perror("fread header");
        fclose(in);
        return 1;
    }
    if (header.magic != MAGIC_LINES) {
        fprintf(stderr, "bad magic (not a lines file)\n");
        fclose(in);
        return 1;
    }

    done = 0;
    pixels_on = 0;

    while (done < header.count) {
        unsigned long long n = header.count - done;
        unsigned long long i;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }
        if (fread(batch, sizeof(struct Line), (size_t)n, in) != (size_t)n) {
            perror("fread lines");
            fclose(in);
            return 1;
        }
        for (i = 0; i < n; i++) {
            int x0, y0, x1, y1;
            int ok0 = to_pixel(batch[i].lon1, batch[i].lat1, header, width, height, &x0, &y0);
            int ok1 = to_pixel(batch[i].lon2, batch[i].lat2, header, width, height, &x1, &y1);
            /* Skip segments with an endpoint outside the image */
            if (!ok0 || !ok1) {
                continue;
            }
            draw_line(grid, width, height, x0, y0, x1, y1, &pixels_on);
        }
        done = done + n;
    }

    fclose(in);
    if (out_count) *out_count = header.count;
    if (out_pixels_on) *out_pixels_on = pixels_on;
    return 0;
}
