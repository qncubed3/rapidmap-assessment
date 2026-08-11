#ifndef RASTERISE_POLYGONS_H
#define RASTERISE_POLYGONS_H

/* Rasterise a .pol file onto grid */
int rasterise_polygons(const char* in_path, unsigned char* grid,
                       int width, int height,
                       unsigned long long* out_count,
                       unsigned long long* out_pixels_on);

#endif
