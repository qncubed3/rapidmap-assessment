#ifndef GENERATE_LINES_H
#define GENERATE_LINES_H

/* Write clustered line segments to a binary .lns file.
 * If compact is non-zero: pick cluster → local centre → two endpoints
 * with COMPACT_STD (short segments). Otherwise both ends use CLUSTER_STD. */
int generate_lines(const char* out_path, unsigned long long count,
                   unsigned int seed, const char* clusters_path,
                   int compact);

#endif
