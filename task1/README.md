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

- **MGA2020 Zone 55** is a projected CRS on **GDA2020** which is a static, Australian-plate-fixed datum, so coordinates of a ground mark do not change with time.
- **WGS 84** is a family of realisations (G1762, G2139, G2296, …), each aligned to an ITRF at some epoch. A mark on the ground will move over time.

### Choices made in this implementation

- **Realisation:** Fix a realisation for WGS 84 as **WGS 84 (G2296)**, and treat it as coincident with **ITRF2020** at the centimetre level (Technical Manual Section 3.5). A true G2296 transform could therefore differ from ours by a few millimetres.
- **Height:** Ellipsoidal height is taken as **0 m**, we are only considering 2D inputs and outputs. Height only enters the Cartesian/Helmert step.
- **Ellipsoid:** **GRS80** for every conversion between geographic and Cartesian, including the WGS84 side. The true WGS 84 ellipsoid has (`1/f = 298.257223563` vs GRS80 `1/f=298.257222101`) so there is a negligible difference.

## Implementation sketch

WGS84 and MGA2020 are not the same kind of thing: MGA2020 is a projected CRS on the **GDA2020** datum. WGS 84 is also a family of datums. Typically, it refers to the latest realisation. Currently this is G2296 realised at year 2024, which we will fix. We will convert from WGS 84 to GDA2020 first, then apply a projection to MGA2020 to obtain the full transformation.

A Helmert transformtion converts geocentric Cartesian coordinates (X, Y, Z) from one terrestrial reference frame to another and will be used as the bridge between WGS 84 (G2296) and GDA2020. This is implemented in `helmert.py` and is described in Equation 17 on page 27. We will also need to convert between geographical coordinates (lat, lon, height) to cartesian coordinates. The equations can be found on page 24-25, and we implement these in `cartesian.py`. 

In Table 3.5 of the GDA Technical Manual, we have the 14 parameters to transform from ITRF202 to GDA2020. We will treat WGS 84 (G2296) as coincident with ITRF2020, as mentioned in Section 3.5, however this is only at the centimeter level as noted. Therefore, we may expect a few mm different between our implementation, and a true transformation from WGS84 (G2296).