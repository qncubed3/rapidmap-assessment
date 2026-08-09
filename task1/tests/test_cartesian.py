"""
Tests for Cartesian <-> geographic coordinate conversion.

Verification data: Alice Springs (ALIC), Section 3.1.1, Table 3.1, page 29
of the GDA2020 Technical Manual v1.8 (both GDA94 and GDA2020).
"""

import numpy as np
import pytest
from pyproj import Transformer

from src.cartesian import cartesian_to_geographic, geographic_to_cartesian
from src.utils import dms_to_decimal


# Alice Springs (ALIC) - Table 3.1, p. 29
XYZ_GDA94   = np.array([-4052051.7643, 4212836.2017, -2545106.0245])
LAT_GDA94   = dms_to_decimal(23, 40, 12.446019, negative=True)
LON_GDA94   = dms_to_decimal(133, 53, 7.847844)
H_GDA94     = 603.3466

XYZ_GDA2020 = np.array([-4052052.7379, 4212835.9897, -2545104.5898])
LAT_GDA2020 = dms_to_decimal(23, 40, 12.39650, negative=True)
LON_GDA2020 = dms_to_decimal(133, 53, 7.87779)
H_GDA2020   = 603.2489


class Test_Cartesian_to_Geographic:
    def test_gda94(self):
        lat, lon, h = cartesian_to_geographic(XYZ_GDA94)
        assert lat == pytest.approx(LAT_GDA94, abs=1e-7)
        assert lon == pytest.approx(LON_GDA94, abs=1e-7)
        assert h   == pytest.approx(H_GDA94,   abs=1e-3)

    def test_gda94_pyproj(self):
        # EPSG:4348 = GDA94 geocentric (XYZ), EPSG:4283 = GDA94 geographic
        transformer = Transformer.from_crs("EPSG:4348", "EPSG:4283", always_xy=True)
        X, Y, Z = XYZ_GDA94
        lon, lat, h = transformer.transform(X, Y, Z)
        assert lat == pytest.approx(LAT_GDA94, abs=1e-7)
        assert lon == pytest.approx(LON_GDA94, abs=1e-7)
        assert h   == pytest.approx(H_GDA94,   abs=1e-3)

    def test_gda2020(self):
        lat, lon, h = cartesian_to_geographic(XYZ_GDA2020)
        assert lat == pytest.approx(LAT_GDA2020, abs=1e-7)
        assert lon == pytest.approx(LON_GDA2020, abs=1e-7)
        assert h   == pytest.approx(H_GDA2020,   abs=1e-3)

    def test_gda2020_pyproj(self):
        # EPSG:7842 = GDA2020 geocentric (XYZ), EPSG:7844 = GDA2020 geographic
        transformer = Transformer.from_crs("EPSG:7842", "EPSG:7844", always_xy=True)
        X, Y, Z = XYZ_GDA2020
        lon, lat, h = transformer.transform(X, Y, Z)
        assert lat == pytest.approx(LAT_GDA2020, abs=1e-7)
        assert lon == pytest.approx(LON_GDA2020, abs=1e-7)
        assert h   == pytest.approx(H_GDA2020,   abs=1e-3)


class Test_Geographic_to_Cartesian:
    def test_gda94(self):
        XYZ = geographic_to_cartesian(LAT_GDA94, LON_GDA94, H_GDA94)
        np.testing.assert_allclose(XYZ, XYZ_GDA94, atol=1e-3)

    def test_gda94_pyproj(self):
        # EPSG:4283 = GDA94 geographic, EPSG:4348 = GDA94 geocentric (XYZ)
        transformer = Transformer.from_crs("EPSG:4283", "EPSG:4348", always_xy=True)
        X, Y, Z = transformer.transform(LON_GDA94, LAT_GDA94, H_GDA94)
        assert X == pytest.approx(XYZ_GDA94[0], abs=1e-3)
        assert Y == pytest.approx(XYZ_GDA94[1], abs=1e-3)
        assert Z == pytest.approx(XYZ_GDA94[2], abs=1e-3)

    def test_gda2020(self):
        XYZ = geographic_to_cartesian(LAT_GDA2020, LON_GDA2020, H_GDA2020)
        np.testing.assert_allclose(XYZ, XYZ_GDA2020, atol=1e-3)

    def test_gda2020_pyproj(self):
        # EPSG:7844 = GDA2020 geographic, EPSG:7842 = GDA2020 geocentric (XYZ)
        transformer = Transformer.from_crs("EPSG:7844", "EPSG:7842", always_xy=True)
        X, Y, Z = transformer.transform(LON_GDA2020, LAT_GDA2020, H_GDA2020)
        assert X == pytest.approx(XYZ_GDA2020[0], abs=1e-3)
        assert Y == pytest.approx(XYZ_GDA2020[1], abs=1e-3)
        assert Z == pytest.approx(XYZ_GDA2020[2], abs=1e-3)


class Test_RoundTrip:
    def test_gda94(self):
        lat, lon, h = cartesian_to_geographic(XYZ_GDA94)
        XYZ_rt = geographic_to_cartesian(lat, lon, h)
        np.testing.assert_allclose(XYZ_rt, XYZ_GDA94, atol=1e-4)

    def test_gda2020(self):
        lat, lon, h = cartesian_to_geographic(XYZ_GDA2020)
        XYZ_rt = geographic_to_cartesian(lat, lon, h)
        np.testing.assert_allclose(XYZ_rt, XYZ_GDA2020, atol=1e-4)
