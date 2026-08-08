import numpy as np
import pytest

from src.cartesian import cartesian_to_geographic, geographic_to_cartesian


class TestAliceSprings:
    """
    Verify Cartesian <-> geographic conversions against example 3.1.1 of the
    GDA2020 Technical Manual v1.8 (Alice Springs, ALIC).

    GDA94 and GDA2020 both use the GRS80 ellipsoid, so the same conversion
    code applies to both coordinate sets.
    """

    # GDA94 values (Section 3.1.1)
    XYZ_GDA94 = np.array([-4052051.7643, 4212836.2017, -2545106.0245])
    LAT_GDA94 = -(23 + 40/60 + 12.446019/3600)
    LON_GDA94 =  (133 + 53/60 + 7.847844/3600)
    H_GDA94   = 603.3466

    # GDA2020 values (Section 3.1.1)
    XYZ_GDA2020 = np.array([-4052052.7379, 4212835.9897, -2545104.5898])
    LAT_GDA2020 = -(23 + 40/60 + 12.39650/3600)
    LON_GDA2020 =  (133 + 53/60 + 7.87779/3600)
    H_GDA2020   = 603.2489

    def test_gda94_cartesian_to_geographic(self):
        lat, lon, h = cartesian_to_geographic(self.XYZ_GDA94)
        assert lat == pytest.approx(self.LAT_GDA94, abs=1e-7)
        assert lon == pytest.approx(self.LON_GDA94, abs=1e-7)
        assert h   == pytest.approx(self.H_GDA94,   abs=1e-3)

    def test_gda94_geographic_to_cartesian(self):
        XYZ = geographic_to_cartesian(self.LAT_GDA94, self.LON_GDA94, self.H_GDA94)
        np.testing.assert_allclose(XYZ, self.XYZ_GDA94, atol=1e-3)

    def test_gda2020_cartesian_to_geographic(self):
        lat, lon, h = cartesian_to_geographic(self.XYZ_GDA2020)
        assert lat == pytest.approx(self.LAT_GDA2020, abs=1e-7)
        assert lon == pytest.approx(self.LON_GDA2020, abs=1e-7)
        assert h   == pytest.approx(self.H_GDA2020,   abs=1e-3)

    def test_gda2020_geographic_to_cartesian(self):
        XYZ = geographic_to_cartesian(self.LAT_GDA2020, self.LON_GDA2020, self.H_GDA2020)
        np.testing.assert_allclose(XYZ, self.XYZ_GDA2020, atol=1e-3)

    def test_roundtrip_gda94(self):
        """Cartesian → geographic → Cartesian should recover original to sub-mm."""
        lat, lon, h = cartesian_to_geographic(self.XYZ_GDA94)
        XYZ_rt = geographic_to_cartesian(lat, lon, h)
        np.testing.assert_allclose(XYZ_rt, self.XYZ_GDA94, atol=1e-4)

    def test_roundtrip_gda2020(self):
        """Cartesian → geographic → Cartesian should recover original to sub-mm."""
        lat, lon, h = cartesian_to_geographic(self.XYZ_GDA2020)
        XYZ_rt = geographic_to_cartesian(lat, lon, h)
        np.testing.assert_allclose(XYZ_rt, self.XYZ_GDA2020, atol=1e-4)
