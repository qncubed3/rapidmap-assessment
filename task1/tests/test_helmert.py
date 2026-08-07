import numpy as np

from src.helmert import helmert7, HelmertParams


def test_gda94_to_gda2020():
    """
    Test GDA94 -> GDA2020 Helmert transformation against example from 
    Geocentric Datum of Australia 2020 Technical Manual Version 1.8, Section 3.1.1 page 29.
    """

    # Input GDA94 Cartesian coordinates (metres)
    XYZ_gda94 = np.array([
        -4052051.7643,
         4212836.2017,
        -2545106.0245
    ])

    # Expected GDA2020 Cartesian coordinates (metres)
    expected = np.array([
        -4052052.7379,
         4212835.9897,
        -2545104.5898
    ])

    # GDA94 -> GDA2020 transformation parameters
    params = HelmertParams(
        tx=0.06155,
        ty=-0.01087,
        tz=-0.04019,

        rx=-0.0394924,
        ry=-0.0327221,
        rz=-0.0328979,

        sc=-0.009994,
    )

    result = helmert7(
        XYZ_gda94,
        params
    )

    # Helmert transformations are sensitive to rounding.
    # Check to sub-millimetre precision.
    np.testing.assert_allclose(
        result,
        expected,
        atol=1e-4
    )