# Task 2 — Rasterisation at scale

## Pipeline overview

The deliverable is a C pipeline (`run.exe`) that, for points, lines, or polygons:

1. **Generates** synthetic 2d features into a binary file (`.pts` / `.lns` / `.pol`)
2. Optionally **prints** the first N features to CSV (`--print`)
3. **Rasterises** onto an N x N black/white grid
4. **Encodes** a PNG under `outputs/`

Timings are reported for generation, rasterisation, and encoding separately, and appended to `performance_log.csv`.

## Hardware

Benchmarks in `performance_log.csv` were measured on:

- **CPU:** Intel(R) Core(TM) i7-1065G7 (4 physical cores / 8 logical processors)
- **RAM:** 16 GB
- **OS:** Windows 10 Home
- **Build:** `gcc -O2` via MinGW (`mingw32-make`)

GPU was not used (CPU-only rasteriser).

## Assumptions

- Render extent:
  - Small enough to minimise disortions and capture Victoria: lon [140, 150], lat [-40, -30]
- Map projections:
  - We do not perform any transformations, lon/lat are treated as cartesian coordinates. The lines and polygons we generate are at a sufficiently local scale that Earth's curvature is negligible and geodesics can be treated as staight lines. For higher resolutions, there may be some deviations from the true shape.
- Data generation:
  - We create synthetic data by choosing 40 cities around Victoria, and using a 2d normal distribution to sample points locally.
- Output and resolution:
  - A png file with either white or black pixels, depening if it itnersects a vector feature or not. Resolution can be adjusted, we test with 4096x4096, 8192x8192 and 16384x16384.
- No antialiasing
  - None for simplicity, pixels are either off (0) or on (255).
- Points stamp a single pixel.
- Lines are stroked with Bresenham (1-pixel wide); not anti-aliased.
- Polygons are filled triangles only (three vertices)
- Batching:
  - Features are streamed in batches of `BATCH_SIZE` (10000) so we do not hold all features in RAM.



## Build and run

Requires **gcc** (MinGW on Windows) and `mingw32-make`.

```powershell
cd task2
mingw32-make
.\run.exe --type polygons --dim 4096 --count 100000
```

Useful flags:

```text
--type points|lines|polygons|all
--dim N
--count N
--print N
--compact
--optimised1          (polygons only)
```

Examples:

```powershell
run.exe --type points --dim 4096 --count 1000000
run.exe --type lines --dim 8192 --count 5000000
run.exe --type polygons --dim 16384 --count 1000 --optimised1
run.exe --type all --dim 4096 --count 100000 --compact
```



### Benchmark matrix

Edit the lists in `run_matrix.ps1`, then:

```powershell
./run_matrix.ps1
```



## Analysis

`analyse.ipynb` loads `performance_log.csv` / `performance_log_poly.csv` and plots rasterisation time vs count (and the optimised vs baseline box plot).

### Points

![Points rasterisation time vs count](assets/points_performance.png)

### Lines

![Lines rasterisation time vs count](assets/line_performance.png)

### Polygons

![Polygons rasterisation time vs count](assets/polygon_performance.png)

### Optimised vs baseline

![Optimised vs baseline polygon fill](assets/optimised_comparison.png)

## Effect of synthetic data shape on timings

An experiement was run on 1,000,000 polygons (triangles) on a 8192x8192 grid. Several runs were performed with compact mode on, meaning that the triangles generated were of much smaller scale, with their vertices clustered closer together. The rasterisation time was consistently 1.7 seconds.

 Several runs were also donw with compact mode off, meaning the size of triangles were much larger - their vertices could be distributed anywhere around the central cluster point. This raised the rasterisation time significantly to around 53 seconds.

This checks out, as the cluster standard deviation is 0.15 degrees, while the compact standard deviation is 0.02 degrees, roughly 7 times more, equating to roughly 50 times more pixels, explaining the scale factor we see in the timings.

## Debrief questions



### Where does this break down, and what would we change?

We are able to generate and rasterise up to 100M points and lines up to a resolution of 16384x16384, and a resolution of 4096x4096 for polygons (triangles). We did not run out of memory since features are streamed in batches and the main RAM cost is the image grid. The main bottleneck is time.

### Is the fastest thing we built what we would put in production?

Production mapping would require testing with polygons beyond triangles, polygons with holes, implementation of CRS and projections, color and feature overlap considerations, antialising.

## Polygon fill algorithm

We implement the method to rasterise a triangle using the following [guide](https://jtsorlinis.github.io/rendering-tutorial/). 

Given three vertices (x1, y1), (x2, y2) and (x3, y3), we can compute its signed area by (x2-x1)(y3-y1)-(y2-y1)(x3-x1)/2. The sign tells us about the orientation of the vertices - positive for clockwise, and negative for counterclockwise. We will standardise the orientation so that all vertices are oredered counterclockwise, by testing the area and swapping a pair if required.

For the rasterisation algorithm, we will first loop through all polygons. For each polygon, compute the bounding box, by taking the maximum and minimum x and y coordinates. Then, we will loop through each pixel in this bounding box and test whether it is inside the triangle.

To test this, we will create a function that determines which side of an edge a point is on. If it is on the left side of all edges (recall we are working with a positive vertex orientation), then the point is inside the triangle.

Given a start and end vertex A, B with coordinates (ax, ay) and (bx, by), and a test point (px, py), the edge function is defined by (xb-xa)(py-ay)-(by-ay)(px-ax). If it is positive, the point is on the left side of the edge from A to B.

Therefore, for each pixel in the bounding box, we compute the above quantity for all three edges to determine if the point is in the polygon. This is the first algorithm implented, called `fill_triangle_original` in `rasterise_polygons.c`.

There are some simple optimisations that can made. First, observe for every pixel, we are performing multiple arithmetic operations to test if the point lies in the triangle. This can be simplified. Observe we can rewrite the edge function as A*px+B*py+C where A=by-ay, B=bx-ax and C=-ax*A-ay*B. 

Since A, B, C are independent of px and py, it suffices to compute these constants once for every triangle. The, for every pixel, moving along the x axis correpsonds to incrementing by A, and moving along the y axis correpsonds to incrementing by B. This will save some artihmetic operations for each pixel, cutting down time.

Furthermore, since we are working with triangles, a convex shape, as we move along a row of pixels, we only enter and exit the polygon once. Therefore, once a pixel is no longer stamped, we can move to the next row, and avoid computations on pixels that will not be in the image.

When running the program, the flag`--optimised1` switches to the optimised algorithm.

See `analyse.ipynb` for a box-plot comparison on repeated runs comparing optimised and non optimised on 1000 features on a 16384x16384 grid.