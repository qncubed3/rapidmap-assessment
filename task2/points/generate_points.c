#include "generate_points.h"
#include "../common.h"

/* Generate clustered points and write them to a binary .pts file */
int generate_points(
    const char* out_path,
    unsigned long long count,
    unsigned int seed,
    const char* clusters_path,
    int compact
) {
    struct Point clusters[MAX_CLUSTERS];
    int cluster_count;
    FILE* f;
    struct Header header;
    struct Point batch[BATCH_SIZE];
    unsigned long long written;

    /* Seed RNG for reproducible output */
    srand(seed);

    cluster_count = load_clusters(clusters_path, clusters, MAX_CLUSTERS);
    if (cluster_count < 1) {
        fprintf(stderr, "need at least one cluster in %s\n", clusters_path);
        return 1;
    }

    f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    /* Write header */
    header.magic = MAGIC_POINTS;
    header.count = count;
    header.min_lon = MIN_LON;
    header.max_lon = MAX_LON;
    header.min_lat = MIN_LAT;
    header.max_lat = MAX_LAT;

    if (fwrite(&header, sizeof(header), 1, f) != 1) {
        perror("fwrite header");
        fclose(f);
        return 1;
    }

    /* Generate and write points in batches */
    written = 0;
    while (written < count) {
        unsigned long long n = count - written;
        unsigned long long i;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }
        for (i = 0; i < n; i++) {
            /* Pick a town, then sample near it (optionally two-stage / compact) */
            struct Point centre = clusters[rand() % cluster_count];
            if (compact) {
                struct Point local = point_near(centre, CLUSTER_STD);
                batch[i] = point_near(local, COMPACT_STD);
            } else {
                batch[i] = point_near(centre, CLUSTER_STD);
            }
        }
        if (fwrite(batch, sizeof(struct Point), (size_t)n, f) != (size_t)n) {
            perror("fwrite points");
            fclose(f);
            return 1;
        }
        written = written + n;
    }

    fclose(f);
    return 0;
}
