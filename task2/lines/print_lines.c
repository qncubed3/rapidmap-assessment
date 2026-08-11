#include "print_lines.h"
#include "../common.h"

/* Write the first limit lines to CSV, or to stdout if csv_path is null */
int print_lines(const char* in_path, const char* csv_path, unsigned long long limit) {
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
    if (header.magic != MAGIC_LINES) {
        fprintf(stderr, "bad magic (not a lines file)\n");
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
        fprintf(csv, "lon1,lat1,lon2,lat2\n");
    }

    for (i = 0; i < n; i++) {
        struct Line line;
        if (fread(&line, sizeof(line), 1, f) != 1) {
            perror("fread line");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }
        if (csv) {
            fprintf(csv, "%f,%f,%f,%f\n", line.lon1, line.lat1, line.lon2, line.lat2);
        } else {
            printf("%llu: (%f,%f) -> (%f,%f)\n",
                   i, line.lon1, line.lat1, line.lon2, line.lat2);
        }
    }

    if (csv) {
        fclose(csv);
    }

    fclose(f);
    return 0;
}
