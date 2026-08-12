#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "points/generate_points.h"
#include "points/rasterise_points.h"
#include "points/print_points.h"
#include "lines/generate_lines.h"
#include "lines/rasterise_lines.h"
#include "lines/print_lines.h"
#include "polygons/generate_polygons.h"
#include "polygons/rasterise_polygons.h"
#include "polygons/print_polygons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define make_dir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define make_dir(path) mkdir(path, 0755)
#endif

/* Fixed settings (not exposed as flags) */
#define DEFAULT_SEED 42
#define DEFAULT_CLUSTERS "clusters.csv"
#define OUTPUT_DIR "outputs"
#define PERF_LOG_PATH "performance_log.csv"

/* Entry point: generate, rasterise, and encode PNG */

static void print_help(void) {
    printf("Usage: run.exe [options]\n");
    printf("  --type points|lines|polygons|all\n");
    printf("  --dim N       square image size (N x N)\n");
    printf("  --count N     number of features to generate\n");
    printf("  --print N     dump first N features to CSV\n");
    printf("  --compact     two-stage sampling: cluster -> local centre\n");
    printf("                -> tight vertices (shorter lines / smaller polys)\n");
    printf("  --optimised1  polygons: incremental edge fill (faster pixel loop)\n");
}

/* Elapsed CPU time in seconds since start */
static double seconds_since(clock_t start) {
    return (double)(clock() - start) / (double)CLOCKS_PER_SEC;
}

/* Ensure outputs/ exists */
static void ensure_output_dir(void) {
    make_dir(OUTPUT_DIR);
}

/* Build path like outputs/20260811_223045_points_5_10x10.png */
static void make_output_path(
    char* out,
    size_t out_size,
    const char* feature,
    unsigned long long count,
    int dim,
    int compact
) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    if (compact) {
        snprintf(out, out_size,
                 "%s/%04d%02d%02d_%02d%02d%02d_%s_%llu_%dx%d_compact.png",
                 OUTPUT_DIR,
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec,
                 feature, count, dim, dim);
    } else {
        snprintf(out, out_size,
                 "%s/%04d%02d%02d_%02d%02d%02d_%s_%llu_%dx%d.png",
                 OUTPUT_DIR,
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec,
                 feature, count, dim, dim);
    }
}

/* Print the standard INFO / TIME report as two aligned columns */
static void print_report(
    const char* feature,
    unsigned long long count,
    int dim,
    int compact,
    int optimised1,
    double gen_s,
    double rast_s,
    double enc_s
) {
    char value[64];

    printf("INFO:\n");
    printf("-- %-14s %16s\n", "Feature", feature);

    snprintf(value, sizeof(value), "%llu", count);
    printf("-- %-14s %16s\n", "Count", value);

    snprintf(value, sizeof(value), "%d x %d", dim, dim);
    printf("-- %-14s %16s\n", "Grid", value);

    printf("-- %-14s %16s\n", "Compact", compact ? "yes" : "no");
    printf("-- %-14s %16d\n", "Optimised", optimised1 ? 1 : 0);

    printf("TIME:\n");
    snprintf(value, sizeof(value), "%.3f s", gen_s);
    printf("-- %-14s %16s\n", "Generation", value);

    snprintf(value, sizeof(value), "%.3f s", rast_s);
    printf("-- %-14s %16s\n", "Rasterisation", value);

    snprintf(value, sizeof(value), "%.3f s", enc_s);
    printf("-- %-14s %16s\n", "Encoding", value);
}

/* Write PNG under outputs/; copies path into out_path on success */
static int write_output_png(
    const char* feature,
    unsigned long long count,
    int dim,
    int compact,
    const unsigned char* grid,
    char* out_path,
    size_t out_path_size
) {
    ensure_output_dir();
    make_output_path(out_path, out_path_size, feature, count, dim, compact);

    if (!stbi_write_png(out_path, dim, dim, 1, grid, dim)) {
        fprintf(stderr, "failed to write %s\n", out_path);
        return 1;
    }
    return 0;
}

/* Append one run to the performance log CSV (creates file + header if needed).
 * optimised: 0/1 for polygons; pass -1 for points/lines (logged as empty/null). */
static void append_performance_log(
    const char* feature,
    unsigned long long count,
    int dim,
    double gen_s,
    double rast_s,
    double enc_s,
    unsigned long long pixels_on,
    int optimised
) {
    FILE* f;
    int write_header = 0;
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char datetime[32];
    char line[512];
    char* old_csv = NULL;
    long old_size = 0;
    int need_migrate = 0;

    f = fopen(PERF_LOG_PATH, "rb");
    if (!f) {
        write_header = 1;
    } else {
        if (fgets(line, sizeof(line), f) != NULL) {
            if (strstr(line, "optimised") == NULL) {
                need_migrate = 1;
            }
        }
        if (need_migrate) {
            fseek(f, 0, SEEK_END);
            old_size = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (old_size > 0) {
                old_csv = (char*)malloc((size_t)old_size + 1);
                if (old_csv) {
                    size_t nread = fread(old_csv, 1, (size_t)old_size, f);
                    old_csv[nread] = '\0';
                }
            }
        }
        fclose(f);
    }

    if (need_migrate && old_csv) {
        /* Rewrite existing rows with an empty optimised column */
        char* row = old_csv;
        char* next;

        f = fopen(PERF_LOG_PATH, "wb");
        if (!f) {
            perror("fopen performance log");
            free(old_csv);
            return;
        }
        fprintf(f,
                "datetime,feature,count,grid,gen_s,rast_s,enc_s,total_s,pixels_on,optimised\n");
        /* skip old header */
        next = strchr(row, '\n');
        if (next) {
            row = next + 1;
        }
        while (*row) {
            next = strchr(row, '\n');
            if (next) {
                *next = '\0';
            }
            if (row[0] != '\0') {
                /* strip trailing CR if present */
                size_t len = strlen(row);
                if (len > 0 && row[len - 1] == '\r') {
                    row[len - 1] = '\0';
                }
                fprintf(f, "%s,\n", row);
            }
            if (!next) {
                break;
            }
            row = next + 1;
        }
        fclose(f);
        free(old_csv);
        write_header = 0;
    } else if (old_csv) {
        free(old_csv);
    }

    f = fopen(PERF_LOG_PATH, "a");
    if (!f) {
        perror("fopen performance log");
        return;
    }

    if (write_header) {
        fprintf(f,
                "datetime,feature,count,grid,gen_s,rast_s,enc_s,total_s,pixels_on,optimised\n");
    }

    snprintf(datetime, sizeof(datetime),
             "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    if (optimised < 0) {
        fprintf(f,
                "%s,%s,%llu,%d,%.6f,%.6f,%.6f,%.6f,%llu,\n",
                datetime,
                feature,
                count,
                dim,
                gen_s, rast_s, enc_s,
                gen_s + rast_s + enc_s,
                pixels_on);
    } else {
        fprintf(f,
                "%s,%s,%llu,%d,%.6f,%.6f,%.6f,%.6f,%llu,%d\n",
                datetime,
                feature,
                count,
                dim,
                gen_s, rast_s, enc_s,
                gen_s + rast_s + enc_s,
                pixels_on,
                optimised);
    }

    fclose(f);
}

/* Run the points pipeline */
static int run_points(
    int dim,
    unsigned long long count,
    unsigned long long print_n,
    int compact
) {
    unsigned char* grid;
    clock_t t0;
    double gen_s, rast_s, enc_s;
    unsigned long long pixels_on = 0;
    char png_path[256];

    /* 1) Generate binary .pts */
    t0 = clock();
    if (generate_points("points/points.pts", count, DEFAULT_SEED,
                        DEFAULT_CLUSTERS, compact) != 0) {
        return 1;
    }
    gen_s = seconds_since(t0);

    if (print_n > 0) {
        print_points("points/points.pts", "points/points.csv", print_n);
    }

    /* Grid alloc is outside rast_s (not counted as rasterisation) */
    grid = (unsigned char*)calloc((size_t)dim * (size_t)dim, 1);
    if (!grid) {
        fprintf(stderr, "out of memory for grid\n");
        return 1;
    }

    /* 2) Rasterise onto grayscale buffer */
    t0 = clock();
    if (rasterise_points("points/points.pts", grid, dim, dim, NULL, &pixels_on) != 0) {
        free(grid);
        return 1;
    }
    rast_s = seconds_since(t0);

    /* 3) Encode PNG */
    t0 = clock();
    if (write_output_png("points", count, dim, compact, grid,
                         png_path, sizeof(png_path)) != 0) {
        free(grid);
        return 1;
    }
    enc_s = seconds_since(t0);

    free(grid);
    print_report("Points", count, dim, compact, 0, gen_s, rast_s, enc_s);
    append_performance_log("points", count, dim, gen_s, rast_s, enc_s, pixels_on, -1);
    return 0;
}

/* Run the lines pipeline */
static int run_lines(
    int dim,
    unsigned long long count,
    unsigned long long print_n,
    int compact
) {
    unsigned char* grid;
    clock_t t0;
    double gen_s, rast_s, enc_s;
    unsigned long long pixels_on = 0;
    char png_path[256];

    /* 1) Generate binary .lns */
    t0 = clock();
    if (generate_lines("lines/lines.lns", count, DEFAULT_SEED,
                       DEFAULT_CLUSTERS, compact) != 0) {
        return 1;
    }
    gen_s = seconds_since(t0);

    if (print_n > 0) {
        print_lines("lines/lines.lns", "lines/lines.csv", print_n);
    }

    grid = (unsigned char*)calloc((size_t)dim * (size_t)dim, 1);
    if (!grid) {
        fprintf(stderr, "out of memory for grid\n");
        return 1;
    }

    /* 2) Rasterise onto grayscale buffer */
    t0 = clock();
    if (rasterise_lines("lines/lines.lns", grid, dim, dim, NULL, &pixels_on) != 0) {
        free(grid);
        return 1;
    }
    rast_s = seconds_since(t0);

    /* 3) Encode PNG */
    t0 = clock();
    if (write_output_png("lines", count, dim, compact, grid,
                         png_path, sizeof(png_path)) != 0) {
        free(grid);
        return 1;
    }
    enc_s = seconds_since(t0);

    free(grid);
    print_report("Lines", count, dim, compact, 0, gen_s, rast_s, enc_s);
    append_performance_log("lines", count, dim, gen_s, rast_s, enc_s, pixels_on, -1);
    return 0;
}

/* Run the polygons pipeline */
static int run_polygons(
    int dim,
    unsigned long long count,
    unsigned long long print_n,
    int compact,
    int optimised1
) {
    unsigned char* grid;
    clock_t t0;
    double gen_s, rast_s, enc_s;
    unsigned long long pixels_on = 0;
    char png_path[256];

    /* 1) Generate binary .pol */
    t0 = clock();
    if (generate_polygons("polygons/polys.pol", count, DEFAULT_SEED,
                          DEFAULT_CLUSTERS, compact) != 0) {
        return 1;
    }
    gen_s = seconds_since(t0);

    if (print_n > 0) {
        print_polygons("polygons/polys.pol", "polygons/polys.csv", print_n);
    }

    grid = (unsigned char*)calloc((size_t)dim * (size_t)dim, 1);
    if (!grid) {
        fprintf(stderr, "out of memory for grid\n");
        return 1;
    }

    /* 2) Rasterise onto grayscale buffer */
    t0 = clock();
    if (rasterise_polygons("polygons/polys.pol", grid, dim, dim, NULL,
                           &pixels_on, optimised1) != 0) {
        free(grid);
        return 1;
    }
    rast_s = seconds_since(t0);

    /* 3) Encode PNG */
    t0 = clock();
    if (write_output_png("polygons", count, dim, compact, grid,
                         png_path, sizeof(png_path)) != 0) {
        free(grid);
        return 1;
    }
    enc_s = seconds_since(t0);

    free(grid);
    print_report("Polygons", count, dim, compact, optimised1, gen_s, rast_s, enc_s);
    append_performance_log("polygons", count, dim, gen_s, rast_s, enc_s, pixels_on,
                           optimised1 ? 1 : 0);
    return 0;
}

int main(int argc, char** argv) {
    /* Defaults */
    const char* type = "points";
    int dim = 4096;
    unsigned long long count = 100000;
    unsigned long long print_n = 0;
    int compact = 0;
    int optimised1 = 0;
    int i;

    /* Parse command-line options */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            i++;
            type = argv[i];
        } else if (strcmp(argv[i], "--dim") == 0 && i + 1 < argc) {
            i++;
            dim = atoi(argv[i]);
        } else if (strcmp(argv[i], "--count") == 0 && i + 1 < argc) {
            i++;
            count = strtoull(argv[i], NULL, 10);
        } else if (strcmp(argv[i], "--print") == 0 && i + 1 < argc) {
            i++;
            print_n = strtoull(argv[i], NULL, 10);
        } else if (strcmp(argv[i], "--compact") == 0) {
            compact = 1;
        } else if (strcmp(argv[i], "--optimised1") == 0) {
            optimised1 = 1;
        } else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            print_help();
            return 1;
        }
    }

    if (dim <= 0) {
        fprintf(stderr, "bad --dim\n");
        return 1;
    }

    /* --type all runs points, then lines, then polygons */
    if (strcmp(type, "points") == 0 || strcmp(type, "all") == 0) {
        if (run_points(dim, count, print_n, compact) != 0) {
            return 1;
        }
    }

    if (strcmp(type, "lines") == 0 || strcmp(type, "all") == 0) {
        if (run_lines(dim, count, print_n, compact) != 0) {
            return 1;
        }
    }

    if (strcmp(type, "polygons") == 0 || strcmp(type, "all") == 0) {
        if (run_polygons(dim, count, print_n, compact, optimised1) != 0) {
            return 1;
        }
    }

    if (strcmp(type, "points") != 0 &&
        strcmp(type, "lines") != 0 &&
        strcmp(type, "polygons") != 0 &&
        strcmp(type, "all") != 0) {
        fprintf(stderr, "unknown --type %s\n", type);
        print_help();
        return 1;
    }

    return 0;
}
