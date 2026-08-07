# Task 1 — Coordinate transformation
Write two functions:

1. **WGS84 geographic → MGA2020 Zone 55** (easting, northing)
2. **MGA2020 Zone 55 → WGS84 geographic** (latitude, longitude)

**Requirements:**

- **Implement the projection mathematics yourself.** Do not simply wrap pyproj, GDAL or an equivalent library for the transformation itself. We want to see that you can read a specification and turn it into working code.
- **Validate against an authoritative source.** Find published control coordinates from Geoscience Australia or the ICSM GDA2020 Technical Manual and test against them. Cite where you got them.
- **You may absolutely use a library to check your answer** — that is exactly what we would do. Show us the comparison.

**Think carefully about what the input actually is.** "WGS84" and "MGA2020" are not the same kind of thing, and the relationship between them is not fixed. If you conclude that the question as asked is incomplete, say so and tell us what you'd need to know. There is a right answer to this and it is worth more than the code.