#include <stdio.h>
#include "common.h"

#define NUM_TO_PRINT 1000
#define SAVE_CSV     1          /* 1 = write CSV, 0 = print to screen */
#define CSV_PATH     "lines.csv"

int main(void) {
    FILE *f = fopen("lines.lns", "rb");
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

    if (header.magic != MAGIC_LINES) {
        fprintf(stderr, "bad magic (not a lines file)\n");
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
        fprintf(csv, "lon1,lat1,lon2,lat2\n");
    }

    int i;
    for (i = 0; i < n; i++) {
        Line line;
        if (fread(&line, sizeof(Line), 1, f) != 1) {
            perror("fread line");
            if (csv) fclose(csv);
            fclose(f);
            return 1;
        }

        if (SAVE_CSV) {
            fprintf(csv, "%f,%f,%f,%f\n", line.lon1, line.lat1, line.lon2, line.lat2);
        } else {
            printf("%d: (%f,%f) -> (%f,%f)\n",
                   i, line.lon1, line.lat1, line.lon2, line.lat2);
        }
    }

    if (csv) {
        fclose(csv);
        printf("wrote %d lines to %s\n", n, CSV_PATH);
    }

    fclose(f);
    return 0;
}
