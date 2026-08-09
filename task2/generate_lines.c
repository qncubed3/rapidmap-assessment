#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

/* --- hardcoded settings (change these, then rebuild) --- */
#define LINE_COUNT    100000000
#define OUT_PATH      "lines.lns"
#define SEED          42
#define CLUSTER_PATH  "clusters.csv"
#define CLUSTER_STD   0.15f          /* spread around each town (degrees) */
#define MAX_CLUSTERS  64

#define BBOX_MIN_LON 140.0f
#define BBOX_MAX_LON 150.0f
#define BBOX_MIN_LAT -40.0f
#define BBOX_MAX_LAT -30.0f

#define BATCH_SIZE 10000

static Point clusters[MAX_CLUSTERS];
static int cluster_count = 0;

static float rand_uniform(float lo, float hi) {
    float fraction = (float)rand() / ((float)RAND_MAX + 1.0f);
    return lo + fraction * (hi - lo);
}

/* Approx normal(mean, std) using sum of 12 uniforms. */
static float rand_normal(float mean, float std) {
    float sum = 0.0f;
    int i;
    for (i = 0; i < 12; i++) {
        sum = sum + rand_uniform(0.0f, 1.0f);
    }
    return mean + (sum - 6.0f) * std;
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int load_clusters(const char *path) {
    FILE *f = fopen(path, "r");
    char line[256];

    if (!f) {
        perror("fopen clusters");
        return 0;
    }

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

/* One random point near a given cluster centre. */
static Point point_near(Point centre) {
    Point p;
    p.lon = clampf(rand_normal(centre.lon, CLUSTER_STD), BBOX_MIN_LON, BBOX_MAX_LON);
    p.lat = clampf(rand_normal(centre.lat, CLUSTER_STD), BBOX_MIN_LAT, BBOX_MAX_LAT);
    return p;
}

/*
 * Pick one cluster, then two random points around it.
 * That pair is one line: lon1,lat1,lon2,lat2.
 */
static Line random_line(void) {
    Point centre = clusters[rand() % cluster_count];
    Point a = point_near(centre);
    Point b = point_near(centre);
    Line line;
    line.lon1 = a.lon;
    line.lat1 = a.lat;
    line.lon2 = b.lon;
    line.lat2 = b.lat;
    return line;
}

int main(void) {
    srand(SEED);

    if (load_clusters(CLUSTER_PATH) < 1) {
        fprintf(stderr, "need at least one cluster in %s\n", CLUSTER_PATH);
        return 1;
    }
    fprintf(stderr, "lines from %d towns, std=%.3f deg\n", cluster_count, CLUSTER_STD);

    clock_t t0 = clock();

    FILE *f = fopen(OUT_PATH, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    Header header;
    header.magic = MAGIC_LINES;
    header.version = 1;
    header.count = LINE_COUNT;
    header.min_lon = BBOX_MIN_LON;
    header.max_lon = BBOX_MAX_LON;
    header.min_lat = BBOX_MIN_LAT;
    header.max_lat = BBOX_MAX_LAT;

    if (fwrite(&header, sizeof(Header), 1, f) != 1) {
        perror("fwrite header");
        fclose(f);
        return 1;
    }

    Line batch[BATCH_SIZE];
    unsigned long long written = 0;

    while (written < LINE_COUNT) {
        unsigned long long this_batch = LINE_COUNT - written;
        if (this_batch > BATCH_SIZE) {
            this_batch = BATCH_SIZE;
        }

        unsigned long long i;
        for (i = 0; i < this_batch; i++) {
            batch[i] = random_line();
        }

        if (fwrite(batch, sizeof(Line), (size_t)this_batch, f) != (size_t)this_batch) {
            perror("fwrite lines");
            fclose(f);
            return 1;
        }

        written = written + this_batch;
    }

    fclose(f);

    double seconds = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
    double bytes = (double)(sizeof(Header) + (unsigned long long)LINE_COUNT * sizeof(Line));
    double megabytes = bytes / (1024.0 * 1024.0);
    fprintf(stderr, "wrote %d lines (%.2f MB) to %s in %.3f s\n",
            LINE_COUNT, megabytes, OUT_PATH, seconds);

    return 0;
}
