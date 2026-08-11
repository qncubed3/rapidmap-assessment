#include "rasterise_points.h"
#include "../common.h"

/* Rasterise points from a .pts file onto a grayscale grid */
int rasterise_points(
    const char* in_path, 
    unsigned char* grid,
    int width, int height,
    unsigned long long* out_count,
    unsigned long long* out_pixels_on
) {
    
    FILE* in;
    struct Header header;
    struct Point batch[BATCH_SIZE];
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
    if (header.magic != MAGIC_POINTS) {
        fprintf(stderr, "bad magic (not a points file)\n");
        fclose(in);
        return 1;
    }

    done = 0;
    pixels_on = 0;

    /* Stream features in batches */
    while (done < header.count) {
        unsigned long long n = header.count - done;
        unsigned long long i;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }
        if (fread(batch, sizeof(struct Point), (size_t)n, in) != (size_t)n) {
            perror("fread points");
            fclose(in);
            return 1;
        }
        for (i = 0; i < n; i++) {
            int x, y;
            if (to_pixel(batch[i].lon, batch[i].lat, header, width, height, &x, &y)) {
                set_pixel(grid, width, height, x, y, &pixels_on);
            }
        }
        done = done + n;
    }

    fclose(in);
    if (out_count) *out_count = header.count;
    if (out_pixels_on) *out_pixels_on = pixels_on;
    return 0;
}
