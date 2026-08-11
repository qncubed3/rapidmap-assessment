#include "print_points.h"
#include "../common.h"

/* Write the first limit points to CSV, or to stdout if csv_path is null */
int print_points(
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
    if (header.magic != MAGIC_POINTS) {
        fprintf(stderr, "bad magic (not a points file)\n");
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
        fprintf(csv, "lon,lat\n");
    }

    for (i = 0; i < n; i++) {
        struct Point p;
        if (fread(&p, sizeof(p), 1, f) != 1) {
            perror("fread point");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }
        if (csv) {
            fprintf(csv, "%f,%f\n", p.lon, p.lat);
        } else {
            printf("%llu: lon=%f lat=%f\n", i, p.lon, p.lat);
        }
    }

    if (csv) {
        fclose(csv);
    }

    fclose(f);
    return 0;
}
