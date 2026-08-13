"""
Named Helmert transformation parameter sets.

All values sourced from the relevant geodetic technical manuals.
Reference epochs where applicable are defined alongside each parameter set.
"""

from .helmert import HelmertParams


# GDA94 -> GDA2020
# Source: Geocentric Datum of Australia 2020 Technical Manual v1.8, Section 3.1.1
GDA94_TO_GDA2020 = HelmertParams(
    tx=0.06155,
    ty=-0.01087,
    tz=-0.04019,

    rx=-0.0394924,
    ry=-0.0327221,
    rz=-0.0328979,

    sc=-0.009994,
)


# ITRF2020 -> GDA2020
# Source: GDA2020 Technical Manual v1.8, Table 3.5 (as published)
# Reference epoch t0 = 2020.0
#
# Agrees with pyproj EPSG:9988 → EPSG:7842. Disagrees with the
# worked-example GDA2020 coordinates in §3.5.1 by ~0.53 m.
# Reverse the transformation by negating all parameters.
ITRF2020_TO_GDA2020 = HelmertParams(
    tx=-0.0014,
    ty=-0.0014,
    tz=0.0024,

    rx=0.0,
    ry=0.0,
    rz=0.0,

    sc=-0.00042,

    dtx=0.0,
    dty=-0.0001,
    dtz=0.0002,

    drx=0.00150379,
    dry=0.00118346,
    drz=0.00120716,

    dsc=0.0,
)

# ATRF2014/ITRF2014 -> GDA2020 (Australian Plate Motion Model)
# Source: GDA2020 Technical Manual v1.8, Table 3.3
# Reference epoch t0 = 2020.0
ITRF2014_TO_GDA2020 = HelmertParams(
    tx=0.0,
    ty=0.0,
    tz=0.0,

    rx=0.0,
    ry=0.0,
    rz=0.0,

    sc=0.0,

    dtx=0.0,
    dty=0.0,
    dtz=0.0,

    drx=0.00150379,
    dry=0.00118346,
    drz=0.00120716,

    dsc=0.0,
)