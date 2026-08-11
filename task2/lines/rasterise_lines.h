#ifndef RASTERISE_LINES_H
#define RASTERISE_LINES_H

/* Rasterise a .lns file onto grid */
int rasterise_lines(const char* in_path, unsigned char* grid,
                    int width, int height,
                    unsigned long long* out_count,
                    unsigned long long* out_pixels_on);

#endif
