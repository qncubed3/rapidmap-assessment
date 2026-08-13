# Task 1 — Coordinate transformation

## [Web Demo](https://wgs84-mga2020.streamlit.app/)

In this task, we implement the coordinate transformations between WGS84 geographic and MGA2020 Zone 55. This is implemented following GDA2020 Technical Manual, which can be found [here](https://www.anzlic.gov.au/sites/default/files/files/GDA2020%20Technical%20Manual%20V1.8_published%20%281%29.pdf). Validation is done with examples from the GDA2020 technical manaul, and the pyproj library. 

## Requirements

- **Python 3.11**

## Setup

From the repo root:

```powershell
cd task1
python -m venv .venv
```

Activate the virtual environment:

```powershell
# Windows (PowerShell)
.\.venv\Scripts\Activate.ps1
```

Install dependencies:

```powershell
pip install -r requirements.txt
```

## Tests

Tests can be found under `tests/` and cover:

- ellipsoid / Cartesian conversions (Table 3.1, Alice Springs)
- 7- and 14-parameter Helmert transformations (Examples 3.1.1, 3.3.1, 3.5.1)
- GDA2020 geographic ↔ MGA2020 (Table 5.1 / Table C-1: Flinders Peak, Buninyong; Alice Springs vs pyproj)
- end-to-end `wgs84_to_mga2020` / `mga2020_to_wgs84` (round-trip + pyproj)

To run the tests:

```powershell
pytest -v
```

## Demo

[Web Demo](https://wgs84-mga2020.streamlit.app/). The full transformation is in `pipeline.py`. A Streamlit frontend converts in both directions and compares against pyproj (`EPSG:10606` and `EPSG:7855`):

```powershell
streamlit run app.py
```

## Assumptions

### The question as asked is incomplete

**WGS84 and MGA2020 are not the same kind of object, and the map between them is not unique.**

- **MGA2020 Zone 55** is a projected CRS on **GDA2020** which is a static datum fixed to the Australian plate. Coordinates of a ground mark do not change with time.
- **WGS 84** is a family of realisations (G1762, G2139, G2296, …), each aligned to an ITRF at some epoch. A mark on the ground will move over time.

### Choices made in this implementation

- **Realisation:** Fix a realisation for WGS 84 as **WGS 84 (G2296)**, and treat it as coincident with **ITRF2020** at the centimetre level (Technical Manual Section 3.5). A true G2296 transform could therefore differ from ours by a few millimetres.
- **Helmert parameters:** Use Table 3.5 **as published** (`ITRF2020_TO_GDA2020`). This matches pyproj `EPSG:9988` → `EPSG:7842`. It disagrees with the worked-example GDA2020 coordinates in §3.5.1 by ~0.53 m. The inverse uses `_negate(ITRF2020_TO_GDA2020)`.
- **Height:** Ellipsoidal height is taken as **0 m**, we are only considering 2D inputs and outputs. Height only enters the Cartesian/Helmert step.
- **Ellipsoid:** **GRS80** for every conversion between geographic and Cartesian, including the WGS84 side. The true WGS 84 ellipsoid has (`1/f = 298.257223563` vs GRS80 `1/f=298.257222101`) so there is a negligible difference.

## Implementation sketch

To transform from WGS84 (G2296) to MGA2020, we convert to GDA2020 first, then apply a projection to MGA2020 to obtain the full transformation.

A Helmert transformtion converts geocentric Cartesian coordinates (X, Y, Z) from one terrestrial reference frame to another and will be used as the bridge between WGS 84 (G2296) and GDA2020. This is implemented in `helmert.py` and is described in Equation 17 on page 27. We will also need to convert between geographical coordinates (lat, lon, height) to cartesian coordinates. The equations can be found on page 24-25, and we implement these in `cartesian.py`. 

In Table 3.5 of the GDA Technical Manual, we have the 14 parameters to transform from ITRF2020 to GDA2020. We treat WGS 84 (G2296) as coincident with ITRF2020, as mentioned in Section 3.5, however this is only at the centimetre level as noted. We use the Table 3.5 parameters **as published** (`ITRF2020_TO_GDA2020`), which matches pyproj `EPSG:9988` → `EPSG:7842`. The reverse direction is `_negate(ITRF2020_TO_GDA2020)`.

## ITRF2020 → GDA2020 comparison and example error

Example 3.5.1 of the GDA2020 Technical Manual: Melbourne (MOBS) at epoch 2024.5.

**Input (ITRF2020):** X = −4130636.582, Y = 2894953.120, Z = −3890530.446

A likely error was picked up during testing. Applying the Helmert transformations with the parameters listed on table 3.5 in fact gave us the wrong expected output. Reversing the transformations aligned with the expected result. After testing with pyproj, I found it matched with our own implementation, and concluded that the transformation applied in the manual on example 3.5.1 was in the reverse order.


| Method                                                   | X (m)         | Y (m)        | Z (m)         | |Δ| vs §3.5.1 (m) |
| -------------------------------------------------------- | ------------- | ------------ | ------------- | ----------------- |
| Expected (manual §3.5.1)                                 | −4130636.759  | 2894953.142  | −3890530.249  | —                 |
| `helmert14` + `ITRF2020_TO_GDA2020` (as published, used) | −4130636.4050 | 2894953.0981 | −3890530.6427 | 0.531             |
| pyproj `EPSG:9988` → `EPSG:7842`                         | −4130636.4050 | 2894953.0981 | −3890530.6427 | 0.531             |
| `helmert14` + `_negate(ITRF2020_TO_GDA2020)`             | −4130636.7590 | 2894953.1419 | −3890530.2493 | 0.0003            |


The as-published parameters match pyproj. Negating Table 3.5 instead matches the §3.5.1 worked-example coordinates to 0.3 mm. We take the as-published / pyproj path as truth.