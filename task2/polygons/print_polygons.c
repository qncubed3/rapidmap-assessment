#include "print_polygons.h"
#include "../common.h"

/* Write the first limit triangles to CSV, or to stdout if csv_path is null */
int print_polygons(
    const char* in_path, 
    const char* csv_path, 
    unsigned long long limit
) {
    
    FILE* f;
    FILE* csv;
    struct Header header;
    unsigned long long n;
    unsigned long long i;

    f = fopen(in_path, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    if (fread(&header, sizeof(header), 1, f) != 1) {
        perror("fread header");
        fclose(f);
        return 1;
    }
    if (header.magic != MAGIC_POLYS) {
        fprintf(stderr, "bad magic (not a polygons file)\n");
        fclose(f);
        return 1;
    }

    n = limit;
    if (header.count < n) {
        n = header.count;
    }

    csv = NULL;
    if (csv_path) {
        csv = fopen(csv_path, "w");
        if (!csv) {
            perror("fopen csv");
            fclose(f);
            return 1;
        }
        fprintf(csv, "lon1,lat1,lon2,lat2,lon3,lat3\n");
    }

    for (i = 0; i < n; i++) {
        struct Triangle t;
        if (fread(&t, sizeof(t), 1, f) != 1) {
            perror("fread triangle");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }
        if (csv) {
            fprintf(csv, "%f,%f,%f,%f,%f,%f\n",
                    t.lon1, t.lat1, t.lon2, t.lat2, t.lon3, t.lat3);
        } else {
            printf("%llu: (%f,%f) (%f,%f) (%f,%f)\n",
                   i, t.lon1, t.lat1, t.lon2, t.lat2, t.lon3, t.lat3);
        }
    }

    if (csv) {
        fclose(csv);
    }

    fclose(f);
    return 0;
}
