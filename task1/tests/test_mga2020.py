"""
Tests for converting between GDA2020 geographic and MGA2020 grid coordinates.

Verification data from the GDA2020 Technical Manual v1.8:
  - Table 5.1 (p. 56):  GDA2020 geographic coordinates  (Flinders, Buninyong)
  - Table C-1 (p. 81):  expected MGA2020 E/N            (Flinders, Buninyong)
  - Table 3.1 (p. 29):  GDA2020 geographic coordinates  (Alice Springs / ALIC)

Zone 55: Flinders Peak, Buninyong — checked against Table C-1 and pyproj.
Zone 53: Alice Springs — checked against pyproj only.
"""

import pytest
from pyproj import Transformer
from src.mga2020 import geographic_to_mga2020, mga2020_to_geographic
from src.utils import dms_to_decimal


# Flinders Peak, Zone 55 — Table 5.1 / Table C-1
FLINDERS_LAT = dms_to_decimal(37, 57, 3.72030, negative=True)
FLINDERS_LON = dms_to_decimal(144, 25, 29.52440)
FLINDERS_E = 273741.297
FLINDERS_N = 5796489.777

# Buninyong, Zone 55 — Table 5.1 / Table C-1
BUNINYONG_LAT = dms_to_decimal(37, 39, 10.15610, negative=True)
BUNINYONG_LON = dms_to_decimal(143, 55, 35.38390)
BUNINYONG_E = 228854.051
BUNINYONG_N = 5828259.038

# Alice Springs (ALIC), Zone 53 — Table 3.1 (no published MGA E/N)
ALIC_LAT = dms_to_decimal(23, 40, 12.39650, negative=True)
ALIC_LON = dms_to_decimal(133, 53, 7.87779)


class Test_GDA_to_MGA:
    def test_flinders(self):
        E, N, _ = geographic_to_mga2020(FLINDERS_LAT, FLINDERS_LON, zone=55)
        assert E == pytest.approx(FLINDERS_E, abs=1e-3)
        assert N == pytest.approx(FLINDERS_N, abs=1e-3)

    def test_flinders_pyproj(self):
        # EPSG:7844 = GDA2020 geographic, EPSG:7855 = MGA2020 Zone 55
        t = Transformer.from_crs("EPSG:7844", "EPSG:7855", always_xy=True)
        E, N = t.transform(FLINDERS_LON, FLINDERS_LAT)
        assert E == pytest.approx(FLINDERS_E, abs=1e-3)
        assert N == pytest.approx(FLINDERS_N, abs=1e-3)

    def test_buninyong(self):
        E, N, _ = geographic_to_mga2020(BUNINYONG_LAT, BUNINYONG_LON, zone=55)
        assert E == pytest.approx(BUNINYONG_E, abs=1e-3)
        assert N == pytest.approx(BUNINYONG_N, abs=1e-3)

    def test_buninyong_pyproj(self):
        t = Transformer.from_crs("EPSG:7844", "EPSG:7855", always_xy=True)
        E, N = t.transform(BUNINYONG_LON, BUNINYONG_LAT)
        assert E == pytest.approx(BUNINYONG_E, abs=1e-3)
        assert N == pytest.approx(BUNINYONG_N, abs=1e-3)

    def test_alice_springs(self):
        # No MGA2020 for this point, so compare our result to pyproj instead.
        E_ours, N_ours, _ = geographic_to_mga2020(ALIC_LAT, ALIC_LON, zone=53)
        # EPSG:7853 = MGA2020 Zone 53
        t = Transformer.from_crs("EPSG:7844", "EPSG:7853", always_xy=True)
        E_pyproj, N_pyproj = t.transform(ALIC_LON, ALIC_LAT)
        assert E_ours == pytest.approx(E_pyproj, abs=1e-3)
        assert N_ours == pytest.approx(N_pyproj, abs=1e-3)


class Test_MGA_to_GDA:
    def test_flinders(self):
        lat, lon = mga2020_to_geographic(FLINDERS_E, FLINDERS_N, zone=55)
        assert lat == pytest.approx(FLINDERS_LAT, abs=1e-8)
        assert lon == pytest.approx(FLINDERS_LON, abs=1e-8)

    def test_flinders_pyproj(self):
        # EPSG:7855 = MGA2020 Zone 55, EPSG:7844 = GDA2020 geographic
        t = Transformer.from_crs("EPSG:7855", "EPSG:7844", always_xy=True)
        lon, lat = t.transform(FLINDERS_E, FLINDERS_N)
        assert lat == pytest.approx(FLINDERS_LAT, abs=1e-8)
        assert lon == pytest.approx(FLINDERS_LON, abs=1e-8)

    def test_buninyong(self):
        lat, lon = mga2020_to_geographic(BUNINYONG_E, BUNINYONG_N, zone=55)
        assert lat == pytest.approx(BUNINYONG_LAT, abs=1e-8)
        assert lon == pytest.approx(BUNINYONG_LON, abs=1e-8)

    def test_buninyong_pyproj(self):
        t = Transformer.from_crs("EPSG:7855", "EPSG:7844", always_xy=True)
        lon, lat = t.transform(BUNINYONG_E, BUNINYONG_N)
        assert lat == pytest.approx(BUNINYONG_LAT, abs=1e-8)
        assert lon == pytest.approx(BUNINYONG_LON, abs=1e-8)

    def test_alice_springs(self):
        # No MGA2020 coordinates for this point — get E/N from pyproj, then invert with our function.
        t = Transformer.from_crs("EPSG:7844", "EPSG:7853", always_xy=True)
        E, N = t.transform(ALIC_LON, ALIC_LAT)
        lat, lon = mga2020_to_geographic(E, N, zone=53)
        assert lat == pytest.approx(ALIC_LAT, abs=1e-9)
        assert lon == pytest.approx(ALIC_LON, abs=1e-9)
