#ifndef PRINT_POINTS_H
#define PRINT_POINTS_H

/* Export the first limit points to CSV, or stdout if csv_path is null */
int print_points(const char* in_path, const char* csv_path, unsigned long long limit);

#endif
