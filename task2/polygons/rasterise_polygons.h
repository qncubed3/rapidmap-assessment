#ifndef RASTERISE_POLYGONS_H
#define RASTERISE_POLYGONS_H

/* Rasterise a .pol file onto grid.
 * If optimised1 is non-zero, use incremental edge stepping in the fill loop. */
int rasterise_polygons(const char* in_path, unsigned char* grid,
                       int width, int height,
                       unsigned long long* out_count,
                       unsigned long long* out_pixels_on,
                       int optimised1);

#endif
