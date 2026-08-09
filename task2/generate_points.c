#include <stdio.h>   /* printf, fprintf, fopen, fwrite, fclose, perror */
#include <stdlib.h>  /* rand, srand */
#include <time.h>    /* clock, CLOCKS_PER_SEC */
#include "common.h"  /* Header, Point, MAGIC_POINTS */

/* --- hardcoded settings (change these, then rebuild) --- */
#define POINT_COUNT   100000000
#define OUT_PATH      "points.pts"
#define SEED          42
#define CLUSTER_MODE  0              /* 0 = uniform in bbox, 1 = clusters */
#define CLUSTER_PATH  "clusters.csv"
#define CLUSTER_STD   0.15f          /* spread around each town (degrees) */
#define MAX_CLUSTERS  64

/* Bounding box: Victoria, Australia (degrees). */
#define BBOX_MIN_LON 140.0f
#define BBOX_MAX_LON 150.0f
#define BBOX_MIN_LAT -40.0f
#define BBOX_MAX_LAT -30.0f

/* How many points we build in memory before writing them to disk at once. */
#define BATCH_SIZE 10000

static Point clusters[MAX_CLUSTERS];
static int cluster_count = 0;

/* Random float in [lo, hi). */
static float rand_uniform(float lo, float hi) {
    float fraction = (float)rand() / ((float)RAND_MAX + 1.0f);
    return lo + fraction * (hi - lo);
}

/* Approx normal(mean, std) using sum of 12 uniforms (no trig needed). */
static float rand_normal(float mean, float std) {
    float sum = 0.0f;
    int i;
    for (i = 0; i < 12; i++) {
        sum = sum + rand_uniform(0.0f, 1.0f);
    }
    /* sum of 12 U(0,1) is roughly mean 6, std 1 */
    return mean + (sum - 6.0f) * std;
}

/* Keep a value inside [lo, hi]. */
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Load lon,lat rows from clusters.csv (skips header, ignores name column). */
static int load_clusters(const char *path) {
    FILE *f = fopen(path, "r");
    char line[256];

    if (!f) {
        perror("fopen clusters");
        return 0;
    }

    /* Skip header line: lon,lat,name */
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }

    cluster_count = 0;
    while (cluster_count < MAX_CLUSTERS && fgets(line, sizeof(line), f)) {
        float lon, lat;
        if (sscanf(line, "%f,%f", &lon, &lat) == 2) {
            clusters[cluster_count].lon = lon;
            clusters[cluster_count].lat = lat;
            cluster_count = cluster_count + 1;
        }
    }

    fclose(f);
    return cluster_count;
}

/* Uniform random point inside the bbox. */
static Point random_point_uniform(void) {
    Point p;
    p.lon = rand_uniform(BBOX_MIN_LON, BBOX_MAX_LON);
    p.lat = rand_uniform(BBOX_MIN_LAT, BBOX_MAX_LAT);
    return p;
}

/* Pick a random town, then sample near it with a normal spread. */
static Point random_point_cluster(void) {
    Point centre = clusters[rand() % cluster_count];
    Point p;
    p.lon = clampf(rand_normal(centre.lon, CLUSTER_STD), BBOX_MIN_LON, BBOX_MAX_LON);
    p.lat = clampf(rand_normal(centre.lat, CLUSTER_STD), BBOX_MIN_LAT, BBOX_MAX_LAT);
    return p;
}

static Point random_point(void) {
    if (CLUSTER_MODE) {
        return random_point_cluster();
    }
    return random_point_uniform();
}

int main(void) {
    srand(SEED);

    if (CLUSTER_MODE) {
        if (load_clusters(CLUSTER_PATH) < 1) {
            fprintf(stderr, "need at least one cluster in %s\n", CLUSTER_PATH);
            return 1;
        }
        fprintf(stderr, "cluster mode: %d towns, std=%.3f deg\n",
                cluster_count, CLUSTER_STD);
    }

    clock_t t0 = clock();

    FILE *f = fopen(OUT_PATH, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    Header header;
    header.magic = MAGIC_POINTS;
    header.version = 1;
    header.count = POINT_COUNT;
    header.min_lon = BBOX_MIN_LON;
    header.max_lon = BBOX_MAX_LON;
    header.min_lat = BBOX_MIN_LAT;
    header.max_lat = BBOX_MAX_LAT;

    if (fwrite(&header, sizeof(Header), 1, f) != 1) {
        perror("fwrite header");
        fclose(f);
        return 1;
    }

    Point batch[BATCH_SIZE];
    unsigned long long written = 0;

    while (written < POINT_COUNT) {
        unsigned long long this_batch = POINT_COUNT - written;
        if (this_batch > BATCH_SIZE) {
            this_batch = BATCH_SIZE;
        }

        unsigned long long i;
        for (i = 0; i < this_batch; i++) {
            batch[i] = random_point();
        }

        if (fwrite(batch, sizeof(Point), (size_t)this_batch, f) != (size_t)this_batch) {
            perror("fwrite points");
            fclose(f);
            return 1;
        }

        written = written + this_batch;
    }

    fclose(f);

    double seconds = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
    double bytes = (double)(sizeof(Header) + (unsigned long long)POINT_COUNT * sizeof(Point));
    double megabytes = bytes / (1024.0 * 1024.0);
    fprintf(stderr, "wrote %d points (%.2f MB) to %s in %.3f s\n",
            POINT_COUNT, megabytes, OUT_PATH, seconds);

    return 0;
}
