#ifndef COMMON_H
#define COMMON_H

/*
 * Shared types and helpers for generate / rasterise / print.
 * Lon/lat are treated as a flat equirectangular plane over a fixed bbox.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* File-type identifiers written at the start of each binary file */
#define MAGIC_POINTS 0x31535450u
#define MAGIC_LINES  0x314E4C4Cu
#define MAGIC_POLYS  0x314C4F50u

/* Victoria bounding box in lon/lat degrees */
#define MIN_LON 140.0f
#define MAX_LON 150.0f
#define MIN_LAT -40.0f
#define MAX_LAT -30.0f

/* Standard deviation for cluster sampling (degrees) */
#define CLUSTER_STD 0.15f
/* With --compact: vertices jitter around a local centre (tighter than CLUSTER_STD) */
#define COMPACT_STD 0.02f
/* Features written per I/O batch */
#define BATCH_SIZE 10000
/* Maximum cluster centres loaded from clusters.csv */
#define MAX_CLUSTERS 64

/* Binary file header */
struct Header {
    unsigned int magic;                        /* File type identifier */
    unsigned long long count;                  /* Number of features */
    float min_lon, max_lon, min_lat, max_lat;  /* Bounding box */
};

/* Geographic point */
struct Point {
    float lon, lat;
};

/* Line segment */
struct Line {
    float lon1, lat1, lon2, lat2;
};

/* Triangle */
struct Triangle {
    float lon1, lat1, lon2, lat2, lon3, lat3;
};

/* Clamp v to the range [lo, hi] */
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Uniform random float in [lo, hi) */
static float rand_uniform(float lo, float hi) {
    float t = (float)rand() / ((float)RAND_MAX + 1.0f);
    return lo + t * (hi - lo);
}

/* Approximate normal(mean, std) via sum of 12 uniforms */
static float rand_normal(float mean, float std) {
    float sum = 0.0f;
    int i;
    for (i = 0; i < 12; i++) {
        sum = sum + rand_uniform(0.0f, 1.0f);
    }
    /* Sum of 12 U(0,1) has mean ~6 and std ~1 */
    return mean + (sum - 6.0f) * std;
}

/* Load cluster centres from clusters.csv */
static int load_clusters(const char* path, struct Point* clusters, int max_n) {
    FILE* f = fopen(path, "r");
    char line[256];
    int n = 0;

    if (!f) {
        perror("fopen clusters");
        return 0;
    }

    /* Skip CSV header row */
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }

    while (n < max_n && fgets(line, sizeof(line), f)) {
        float lon, lat;
        if (sscanf(line, "%f,%f", &lon, &lat) == 2) {
            clusters[n].lon = lon;
            clusters[n].lat = lat;
            n = n + 1;
        }
    }

    fclose(f);
    return n;
}

/* Sample a point near a cluster centre */
static struct Point point_near(struct Point centre, float std) {
    struct Point p;
    p.lon = clampf(rand_normal(centre.lon, std), MIN_LON, MAX_LON);
    p.lat = clampf(rand_normal(centre.lat, std), MIN_LAT, MAX_LAT);
    return p;
}

/* Map lon/lat to pixel coordinates; returns 1 if inside the image */
static int to_pixel(float lon, float lat, struct Header h,
                    int width, int height, int* x, int* y) {
    *x = (int)((lon - h.min_lon) / (h.max_lon - h.min_lon) * width);
    /* Flip latitude so north maps to the top of the image */
    *y = (int)((h.max_lat - lat) / (h.max_lat - h.min_lat) * height);
    if (*x < 0 || *x >= width || *y < 0 || *y >= height) {
        return 0;
    }
    return 1;
}

/* Set a pixel to white and increment the lit-pixel count once */
static void set_pixel(unsigned char* grid, int width, int height,
                      int x, int y, unsigned long long* pixels_on) {
    int i;
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    i = y * width + x;
    if (grid[i] == 0) {
        *pixels_on = *pixels_on + 1;
    }
    grid[i] = 255;
}

#endif
