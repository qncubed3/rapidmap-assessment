"""
Tests for geographic <-> MGA2020 conversion.

Verification data taken from two tables in the GDA2020 Technical Manual v1.8
that share the same two control points:
  - Geographic coordinates (lat/lon): Table 5.1, p. 56
  - MGA2020 grid coordinates (E/N):   Table C-1, p. 81

Both control points lie within Zone 55 (central meridian 147°E).
"""

import pytest
from src.mga2020 import geographic_to_mga2020, mga2020_to_geographic


def dms_to_decimal(d: int, m: int, s: float, negative: bool = False) -> float:
    """Convert degrees / minutes / seconds to decimal degrees."""
    value = d + m / 60.0 + s / 3600.0
    return -value if negative else value


class Test_Geographic_to_MGA2020_Flinders:
    """
    Flinders Peak, Zone 55.
    Table 5.1 (p. 56):  φ = -37°57'03.72030"  λ = 144°25'29.52440"
    Table C-1 (p. 81):  E = 273741.297 m       N = 5796489.777 m
    """

    lat = dms_to_decimal(37, 57, 3.72030, negative=True)
    lon = dms_to_decimal(144, 25, 29.52440)
    expected_E = 273741.297
    expected_N = 5796489.777

    def test_easting(self):
        E, _, _ = geographic_to_mga2020(self.lat, self.lon, zone=55)
        assert E == pytest.approx(self.expected_E, abs=1e-3)

    def test_northing(self):
        _, N, _ = geographic_to_mga2020(self.lat, self.lon, zone=55)
        assert N == pytest.approx(self.expected_N, abs=1e-3)

    def test_round_trip(self):
        E, N, zone = geographic_to_mga2020(self.lat, self.lon, zone=55)
        lat_rt, lon_rt = mga2020_to_geographic(E, N, zone=zone)
        assert lat_rt == pytest.approx(self.lat, abs=1e-9)
        assert lon_rt == pytest.approx(self.lon, abs=1e-9)


class Test_Geographic_to_MGA2020_Buninyong:
    """
    Buninyong, Zone 55.
    Table 5.1 (p. 56):  φ = -37°39'10.15610"  λ = 143°55'35.38390"
    Table C-1 (p. 81):  E = 228854.051 m       N = 5828259.038 m
    """

    lat = dms_to_decimal(37, 39, 10.15610, negative=True)
    lon = dms_to_decimal(143, 55, 35.38390)
    expected_E = 228854.051
    expected_N = 5828259.038

    def test_easting(self):
        E, _, _ = geographic_to_mga2020(self.lat, self.lon, zone=55)
        assert E == pytest.approx(self.expected_E, abs=1e-3)

    def test_northing(self):
        _, N, _ = geographic_to_mga2020(self.lat, self.lon, zone=55)
        assert N == pytest.approx(self.expected_N, abs=1e-3)

    def test_round_trip(self):
        E, N, zone = geographic_to_mga2020(self.lat, self.lon, zone=55)
        lat_rt, lon_rt = mga2020_to_geographic(E, N, zone=zone)
        assert lat_rt == pytest.approx(self.lat, abs=1e-9)
        assert lon_rt == pytest.approx(self.lon, abs=1e-9)
