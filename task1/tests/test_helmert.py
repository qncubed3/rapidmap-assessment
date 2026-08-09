"""
Tests for Helmert coordinate transformations.

Verification data taken from the GDA2020 Technical Manual v1.8:
  - Section 3.1.1, Example: GDA94 -> GDA2020, Alice Springs (ALIC)
  - Section 3.3.1, Example: ITRF2014 -> GDA2020, Alice Springs at epoch 2018.0
  - Section 3.5.1, Example: ITRF2020/WGS84 -> GDA2020, Melbourne (MOBS) at epoch 2024.5
"""

import pytest

from src.helmert import helmert7, helmert14
from src.parameters import GDA94_TO_GDA2020, ITRF2014_TO_GDA2020, ITRF2020_TO_GDA2020


class Test_GDA94_to_GDA2020:
    """
    GDA94 -> GDA2020 7-parameter Helmert transformation.
    Source: GDA2020 Technical Manual v1.8, Section 3.1.1 (page 29).
    Testing coordinates: Alice Springs (ALIC)
    """

    XYZ_gda94               = [-4052051.7643,  4212836.2017, -2545106.0245]
    XYZ_gda2020_expected    = [-4052052.7379,  4212835.9897, -2545104.5898]

    def test_helmert7(self):
        result = helmert7(self.XYZ_gda94, GDA94_TO_GDA2020)
        assert result == pytest.approx(self.XYZ_gda2020_expected, abs=1e-4)


class Test_ITRF2014_to_GDA2020:
    """
    ATRF2014/ITRF2014 -> GDA2020 14-parameter transformation (Australian PMM).
    Source: GDA2020 Technical Manual v1.8, Section 3.3.1 (page 33).
    Reference epoch t0 = 2020.0.
    Testing coordinates: Alice Springs (ALIC)
    """

    # ITRF2014 at epoch 2018.0, Alice Springs (ALIC)
    XYZ_itrf2014            = [-4052052.6588, 4212835.9938, -2545104.6946]
    XYZ_gda2020_expected    = [-4052052.7373, 4212835.9835, -2545104.5867]

    def test_helmert14_at_2018(self):
        result = helmert14(self.XYZ_itrf2014, ITRF2014_TO_GDA2020, t=2018.0, t0=2020.0)
        assert result == pytest.approx(self.XYZ_gda2020_expected, abs=1e-4)


class Test_ITRF2020_to_GDA2020:
    """
    WGS 84 (G2296) / ITRF2020 -> GDA2020 14-parameter transformation.
    Source: GDA2020 Technical Manual v1.8, Section 3.5.1.
    Reference epoch t0 = 2020.0.
    WGS 84 observation on 14 Feb 2024 is coincident with ITRF2020 at mid-year 2024 (t = 2024.5).
    """

    # ITRF2020 at epoch 2024.5, Melbourne (MOBS)
    XYZ_itrf2020         = [-4130636.582, 2894953.120, -3890530.446]
    XYZ_gda2020_expected = [-4130636.759, 2894953.142, -3890530.249]

    def test_helmert14_at_2024_5(self):
        result = helmert14(self.XYZ_itrf2020, ITRF2020_TO_GDA2020, t=2024.5, t0=2020.0)
        assert result == pytest.approx(self.XYZ_gda2020_expected, abs=1e-3)
