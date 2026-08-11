#ifndef GENERATE_POINTS_H
#define GENERATE_POINTS_H

/* Write clustered points to a binary .pts file; returns 0 on success.
 * If compact is non-zero: pick cluster → local centre (CLUSTER_STD) →
 * point (COMPACT_STD). Otherwise sample once near the cluster. */
int generate_points(const char* out_path, unsigned long long count,
                    unsigned int seed, const char* clusters_path,
                    int compact);

#endif
