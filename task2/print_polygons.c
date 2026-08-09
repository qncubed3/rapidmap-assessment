#include <stdio.h>
#include "common.h"

#define NUM_TO_PRINT 1000
#define SAVE_CSV     1
#define CSV_PATH     "polys.csv"

int main(void) {
    FILE *f = fopen("polys.pol", "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    Header header;
    if (fread(&header, sizeof(Header), 1, f) != 1) {
        perror("fread header");
        fclose(f);
        return 1;
    }

    if (header.magic != MAGIC_POLYS) {
        fprintf(stderr, "bad magic (not a polygons file)\n");
        fclose(f);
        return 1;
    }

    printf("count=%llu version=%u\n",
           (unsigned long long)header.count, header.version);
    printf("bbox lon=[%f, %f] lat=[%f, %f]\n",
           header.min_lon, header.max_lon, header.min_lat, header.max_lat);

    int n = NUM_TO_PRINT;
    if (header.count < (unsigned long long)NUM_TO_PRINT) {
        n = (int)header.count;
    }

    FILE *csv = NULL;
    if (SAVE_CSV) {
        csv = fopen(CSV_PATH, "w");
        if (!csv) {
            perror("fopen csv");
            fclose(f);
            return 1;
        }
        fprintf(csv, "lon1,lat1,lon2,lat2,lon3,lat3\n");
    }

    int i;
    for (i = 0; i < n; i++) {
        Triangle t;
        if (fread(&t, sizeof(Triangle), 1, f) != 1) {
            perror("fread triangle");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }

        if (SAVE_CSV) {
            fprintf(csv, "%f,%f,%f,%f,%f,%f\n",
                    t.lon1, t.lat1, t.lon2, t.lat2, t.lon3, t.lat3);
        } else {
            printf("%d: (%f,%f) (%f,%f) (%f,%f)\n",
                   i, t.lon1, t.lat1, t.lon2, t.lat2, t.lon3, t.lat3);
        }
    }

    if (csv) {
        fclose(csv);
        printf("wrote %d triangles to %s\n", n, CSV_PATH);
    }

    fclose(f);
    return 0;
}
