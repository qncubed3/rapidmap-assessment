#include "generate_polygons.h"
#include "../common.h"

/* Generate clustered triangles and write them to a binary .pol file */
int generate_polygons(
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
    struct Triangle batch[BATCH_SIZE];
    unsigned long long written;

    srand(seed);  /* reproducible features */

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

    /* Write header (magic + count + bbox) */
    header.magic = MAGIC_POLYS;
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

    /* Each triangle: three corners near the same cluster (or compact local centre) */
    written = 0;
    while (written < count) {
        unsigned long long n = count - written;
        unsigned long long i;
        if (n > BATCH_SIZE) {
            n = BATCH_SIZE;
        }
        for (i = 0; i < n; i++) {
            struct Point centre = clusters[rand() % cluster_count];
            struct Point a, b, c;
            if (compact) {
                struct Point local = point_near(centre, CLUSTER_STD);
                a = point_near(local, COMPACT_STD);
                b = point_near(local, COMPACT_STD);
                c = point_near(local, COMPACT_STD);
            } else {
                a = point_near(centre, CLUSTER_STD);
                b = point_near(centre, CLUSTER_STD);
                c = point_near(centre, CLUSTER_STD);
            }
            batch[i].lon1 = a.lon;
            batch[i].lat1 = a.lat;
            batch[i].lon2 = b.lon;
            batch[i].lat2 = b.lat;
            batch[i].lon3 = c.lon;
            batch[i].lat3 = c.lat;
        }
        if (fwrite(batch, sizeof(struct Triangle), (size_t)n, f) != (size_t)n) {
            perror("fwrite polys");
            fclose(f);
            return 1;
        }
        written = written + n;
    }

    fclose(f);
    return 0;
}
