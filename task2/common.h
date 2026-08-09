#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define MAGIC_POINTS 0x31535450u  /* "PTS1" little-endian */
#define MAGIC_LINES  0x314E4C4Cu  /* "LNS1" little-endian */
#define MAGIC_POLYS  0x314C4F50u  /* "POL1" little-endian */

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t count;
    float min_lon, max_lon, min_lat, max_lat;
} Header;

typedef struct {
    float lon, lat;
} Point;

/* One line segment: endpoint A then endpoint B. */
typedef struct {
    float lon1, lat1, lon2, lat2;
} Line;

/* One triangle: three lon/lat corners. */
typedef struct {
    float lon1, lat1, lon2, lat2, lon3, lat3;
} Triangle;

#endif
