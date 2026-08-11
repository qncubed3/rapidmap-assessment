#ifndef RASTERISE_POINTS_H
#define RASTERISE_POINTS_H

/* Rasterise a .pts file onto grid; returns 0 on success */
int rasterise_points(
    const char* in_path, 
    unsigned char* grid,
    int width, int height,
    unsigned long long* out_count,
    unsigned long long* out_pixels_on
);

#endif
