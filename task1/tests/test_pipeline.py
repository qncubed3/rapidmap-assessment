"""
Tests for the end-to-end WGS84 (G2296)<-> MGA2020 (Zone 55) pipeline.

Two types of tests:
  1. Round-trip: push a point through wgs84_to_mga2020 then mga2020_to_wgs84
     and verify it comes back to within floating-point precision.

  2. Cross-check against pyproj: verify our result agrees with the industry-
     standard library to within a few millimetres.
"""

import pytest
from pyproj import Transformer
from src.pipeline import wgs84_to_mga2020, mga2020_to_wgs84
from src.utils import dms_to_decimal


# Flinders Peak (Table 5.1, GDA2020 Technical Manual v1.8)
# Used here as a representative Zone 55 test point.
LAT = dms_to_decimal(37, 57, 3.72030, negative=True)
LON = dms_to_decimal(144, 25, 29.52440)


class Test_WGS84_to_MGA2020_RoundTrip:
    """
    Round-trip test: WGS84 -> MGA2020 -> WGS84.
    """

    def test_lat_lon_round_trip(self):
        easting, northing, zone = wgs84_to_mga2020(LAT, LON, epoch=2020.0)
        lat_rt, lon_rt = mga2020_to_wgs84(easting, northing, epoch=2020.0, zone=zone)
        assert lat_rt == pytest.approx(LAT, abs=1e-9)
        assert lon_rt == pytest.approx(LON, abs=1e-9)



class Test_WGS84_to_MGA2020_Pyproj:
    """
    Cross-check our result against pyproj.
    """

    def test_easting_northing_pyproj(self):
        # EPSG:10606 = WGS 84 (G2296), the specific realisation the pipeline targets
        transformer = Transformer.from_crs("EPSG:10606", "EPSG:7855", always_xy=True)
        E_pyproj, N_pyproj = transformer.transform(LON, LAT)

        E_ours, N_ours, _ = wgs84_to_mga2020(LAT, LON, epoch=2020.0)

        assert E_ours == pytest.approx(E_pyproj, abs=0.01)
        assert N_ours == pytest.approx(N_pyproj, abs=0.01)
