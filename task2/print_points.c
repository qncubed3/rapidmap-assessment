#include <stdio.h>
#include "common.h"

#define NUM_TO_PRINT 10000
#define SAVE_CSV     1          /* 1 = write CSV, 0 = print to screen */
#define CSV_PATH     "points.csv"

int main(void) {
    FILE *f = fopen("points.pts", "rb");
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

    if (header.magic != MAGIC_POINTS) {
        fprintf(stderr, "bad magic (not a points file)\n");
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
        fprintf(csv, "lon,lat\n");
    }

    int i;
    for (i = 0; i < n; i++) {
        Point p;
        if (fread(&p, sizeof(Point), 1, f) != 1) {
            perror("fread point");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }

        if (SAVE_CSV) {
            fprintf(csv, "%f,%f\n", p.lon, p.lat);
        } else {
            printf("%d: lon=%f lat=%f\n", i, p.lon, p.lat);
        }
    }

    if (csv) {
        fclose(csv);
        printf("wrote %d points to %s\n", n, CSV_PATH);
    }

    fclose(f);
    return 0;
}
