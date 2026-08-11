#ifndef PRINT_LINES_H
#define PRINT_LINES_H

/* Export the first limit lines to CSV */
int print_lines(const char* in_path, const char* csv_path, unsigned long long limit);

#endif
