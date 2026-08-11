# Task 2 build
#   make              build run.exe (-O2)
#   make debug        build with -g -O0
#   make clean        remove exe and generated data
#   make run ARGS="--type points --dim 10 --count 5 --print 5"
#   make matrix       run parameter matrix (see run_matrix.ps1 CONFIG)

CC      = gcc
CFLAGS  = -O2
TARGET  = run.exe

SRCS = \
	run.c \
	points/generate_points.c \
	points/rasterise_points.c \
	points/print_points.c \
	lines/generate_lines.c \
	lines/rasterise_lines.c \
	lines/print_lines.c \
	polygons/generate_polygons.c \
	polygons/rasterise_polygons.c \
	polygons/print_polygons.c

.PHONY: all debug clean run matrix

all: $(TARGET)

$(TARGET): $(SRCS) common.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

debug:
	$(MAKE) CFLAGS="-g -O0" all

clean:
	rm -f $(TARGET)
	rm -f points/*.pts points/*.csv points/output.png
	rm -f lines/*.lns lines/*.csv lines/output.png
	rm -f polygons/*.pol polygons/*.csv polygons/output.png
	rm -f outputs/*.png

run: $(TARGET)
	./$(TARGET) $(ARGS)

matrix: $(TARGET)
	powershell -ExecutionPolicy Bypass -File ./run_matrix.ps1
