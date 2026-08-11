#ifndef GENERATE_POLYGONS_H
#define GENERATE_POLYGONS_H

/* Write clustered triangles to a binary .pol file.
 * If compact is non-zero: pick cluster → local centre → three corners
 * with COMPACT_STD (small triangles). Otherwise corners use CLUSTER_STD. */
int generate_polygons(const char* out_path, unsigned long long count,
                      unsigned int seed, const char* clusters_path,
                      int compact);

#endif
