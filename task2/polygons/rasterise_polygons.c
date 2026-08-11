#include "rasterise_polygons.h"
#include "../common.h"

/*
 * Edge function: tells which side of directed edge A->B the point P is on.
 *   > 0  => P is to the left of A->B
 *   = 0  => P is on the line through A and B
 *   < 0  => P is to the right of A->B
 */
static int edge(int ax, int ay, int bx, int by, int px, int py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

/* Fill all pixels covered by the triangle */
static void fill_triangle(
    unsigned char* grid, 
    int width, int height,
    int x0, int y0, int x1, int y1, int x2, int y2,
    unsigned long long* pixels_on
) {

    int min_x = x0;
    int max_x = x0;
    int min_y = y0;
    int max_y = y0;
    int area;
    int x, y;

    /* Bounding box of the triangle */
    if (x1 < min_x) min_x = x1;
    if (x1 > max_x) max_x = x1;
    if (y1 < min_y) min_y = y1;
    if (y1 > max_y) max_y = y1;
    if (x2 < min_x) min_x = x2;
    if (x2 > max_x) max_x = x2;
    if (y2 < min_y) min_y = y2;
    if (y2 > max_y) max_y = y2;

    /* Clip to image bounds */
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= width) max_x = width - 1;
    if (max_y >= height) max_y = height - 1;

    /* Signed area of the full triangle (also used as winding test) */
    area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) {
        return;  /* Degenerate: all three points are colinear */
    }
    /* Flip two vertices so the triangle is counter-clockwise */
    if (area < 0) {
        int tx = x1;
        int ty = y1;
        x1 = x2;
        y1 = y2;
        x2 = tx;
        y2 = ty;
    }

    /*T est every pixel in the bounding box. */
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            int w0 = edge(x1, y1, x2, y2, x, y);  /* (x, y) vs edge 1 -> 2 */
            int w1 = edge(x2, y2, x0, y0, x, y);  /* (x, y) vs edge 2 -> 0 */
            int w2 = edge(x0, y0, x1, y1, x, y);  /* (x, y) vs edge 0 -> 1 */

            /* If the point is on the left of all three edges, it is inside the triangle */
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                set_pixel(grid, width, height, x, y, pixels_on);
            }
        }
    }
}

/* Rasterise triangles from a .pol file onto a grayscale grid */
int rasterise_polygons(
    const char* in_path, 
    unsigned char* grid,
    int width, int height,
    unsigned long long* out_count,
    unsigned long long* out_pixels_on
) {
    
    FILE* in;
    struct Header header;
    struct Triangle batch[BATCH_SIZE];
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
    if (header.magic != MAGIC_POLYS) {
        fprintf(stderr, "bad magic (not a polygons file)\n");
        fclose(in);
        return 1;
    }

    done = 0;
    pixels_on = 0;

    /* Stream triangles from disk in batches so we do not load everything at once */
    while (done < header.count) {
        unsigned long long n = header.count - done;
        unsigned long long i;

        /* Cap this round at BATCH_SIZE */
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }

        /* Read the next batch of triangles */
        if (fread(batch, sizeof(struct Triangle), (size_t)n, in) != (size_t)n) {
            perror("fread polys");
            fclose(in);
            return 1;
        }

        /* Project each triangle to pixels, then fill it */
        for (i = 0; i < n; i++) {
            int x0, y0, x1, y1, x2, y2;
            int ok0 = to_pixel(batch[i].lon1, batch[i].lat1, header, width, height, &x0, &y0);
            int ok1 = to_pixel(batch[i].lon2, batch[i].lat2, header, width, height, &x1, &y1);
            int ok2 = to_pixel(batch[i].lon3, batch[i].lat3, header, width, height, &x2, &y2);

            /* Skip triangles with a vertex outside the image */
            if (!ok0 || !ok1 || !ok2) {
                continue;
            }

            fill_triangle(grid, width, height, x0, y0, x1, y1, x2, y2, &pixels_on);
        }

        done = done + n;
    }

    fclose(in);

    /* Optional outputs for the caller */
    if (out_count) *out_count = header.count;
    if (out_pixels_on) *out_pixels_on = pixels_on;
    return 0;
}
