"""
End-to-end coordinate transformation pipeline.

Implements the two functions required by the task instructions:
  1. WGS84 geographic  ->  MGA2020 Zone 55  (easting, northing)
  2. MGA2020 Zone 55   ->  WGS84 geographic  (latitude, longitude)
"""

from .cartesian import geographic_to_cartesian, cartesian_to_geographic
from .helmert import helmert14, HelmertParams
from .mga2020 import geographic_to_mga2020, mga2020_to_geographic
from .parameters import ITRF2020_TO_GDA2020

# Reference epoch for GDA2020 (and ITRF2020 -> GDA2020 parameters)
_T0 = 2020.0


def wgs84_to_mga2020(lat_deg, lon_deg, epoch=2020, zone=55):
    """
    Convert WGS84 geographic coordinates to MGA2020 grid coordinates.

    Parameters:
        lat_deg: WGS84 latitude in decimal degrees (negative south)
        lon_deg: WGS84 longitude in decimal degrees
        epoch:   observation epoch as a decimal year (default 2020.0)
                 e.g. 2024.5 for mid-2024
        zone:    MGA2020 zone number (default 55)

    Returns:
        (easting, northing, zone) in metres
    """
    # Step 1 — WGS84 geographic -> Cartesian (height assumed 0)
    xyz_wgs84 = geographic_to_cartesian(lat_deg, lon_deg, h=0.0)

    # Step 2 — ITRF2020/WGS84 Cartesian -> GDA2020 Cartesian
    xyz_gda2020 = helmert14(xyz_wgs84, ITRF2020_TO_GDA2020, t=epoch, t0=_T0)

    # Step 3 — GDA2020 Cartesian -> GDA2020 geographic
    lat_gda2020, lon_gda2020, _ = cartesian_to_geographic(xyz_gda2020)

    # Step 4 — GDA2020 geographic -> MGA2020
    easting, northing, zone = geographic_to_mga2020(lat_gda2020, lon_gda2020, zone=zone)

    return easting, northing, zone


def mga2020_to_wgs84(easting, northing, epoch=2020.0, zone=55):
    """
    Convert MGA2020 grid coordinates to WGS84 geographic coordinates.

    Parameters:
        easting:  MGA2020 easting in metres
        northing: MGA2020 northing in metres
        epoch:    observation epoch as a decimal year (default 2020.0)
        zone:     MGA2020 zone number (default 55)

    Returns:
        (lat_deg, lon_deg) WGS84 latitude and longitude in decimal degrees
    """
    # Step 1 — MGA2020 -> GDA2020 geographic
    lat_gda2020, lon_gda2020 = mga2020_to_geographic(easting, northing, zone=zone)

    # Step 2 — GDA2020 geographic -> Cartesian (height assumed 0)
    xyz_gda2020 = geographic_to_cartesian(lat_gda2020, lon_gda2020, h=0.0)

    # Step 3 — GDA2020 Cartesian -> ITRF2020/WGS84 Cartesian (inverse Helmert)
    # Negate all parameters to reverse the transformation direction
    xyz_wgs84 = helmert14(xyz_gda2020, _negate(ITRF2020_TO_GDA2020), t=epoch, t0=_T0)

    # Step 4 — Cartesian -> WGS84 geographic
    lat_wgs84, lon_wgs84, _ = cartesian_to_geographic(xyz_wgs84)

    return lat_wgs84, lon_wgs84


def _negate(params):
    """Return a copy of HelmertParams with all values negated (reverses direction)."""
    return HelmertParams(
        tx=-params.tx,   ty=-params.ty,   tz=-params.tz,
        rx=-params.rx,   ry=-params.ry,   rz=-params.rz,
        sc=-params.sc,
        dtx=-params.dtx, dty=-params.dty, dtz=-params.dtz,
        drx=-params.drx, dry=-params.dry, drz=-params.drz,
        dsc=-params.dsc,
    )


def explore_itrf2020_to_gda2020_cart():
    """Compare ITRF2020 -> GDA2020 Cartesian for MOBS (§3.5.1) at epoch 2024.5."""
    from pyproj import Transformer

    xyz_itrf2020 = [-4130636.582, 2894953.120, -3890530.446]
    xyz_gda2020_expected = [-4130636.759, 2894953.142, -3890530.249]

    xyz_ours = helmert14(xyz_itrf2020, ITRF2020_TO_GDA2020, t=2024.5, t0=_T0)
    xyz_negated = helmert14(xyz_itrf2020, _negate(ITRF2020_TO_GDA2020), t=2024.5, t0=_T0)

    transformer = Transformer.from_crs("EPSG:9988", "EPSG:7842", always_xy=True)
    xyz_pyproj = transformer.transform(*xyz_itrf2020, 2024.5)[:3]

    print("ITRF2020 -> GDA2020 cart")
    print("expected", *xyz_gda2020_expected)
    print("ours    ", *xyz_ours)
    print("negated ", *xyz_negated)
    print("pyproj  ", *xyz_pyproj)


def explore_itrf2020_to_mga2020():
    """Compare ITRF2020 Cartesian -> MGA2020 Zone 55 for MOBS at epoch 2024.5."""
    from pyproj import Transformer

    xyz_itrf2020 = [-4130636.582, 2894953.120, -3890530.446]
    epoch = 2024.5
    zone = 55

    def to_mga(params):
        xyz_gda2020 = helmert14(xyz_itrf2020, params, t=epoch, t0=_T0)
        lat, lon, _ = cartesian_to_geographic(xyz_gda2020)
        return geographic_to_mga2020(lat, lon, zone=zone)

    e_ours, n_ours, z_ours = to_mga(ITRF2020_TO_GDA2020)
    e_negated, n_negated, z_negated = to_mga(_negate(ITRF2020_TO_GDA2020))

    transformer = Transformer.from_crs("EPSG:9988", "EPSG:7855", always_xy=True)
    e_pyproj, n_pyproj = transformer.transform(*xyz_itrf2020, epoch)[:2]

    print("ITRF2020 -> MGA2020")
    print("ours    ", e_ours, n_ours, z_ours)
    print("negated ", e_negated, n_negated, z_negated)
    print("pyproj  ", e_pyproj, n_pyproj, zone)


if __name__ == "__main__":
    explore_itrf2020_to_gda2020_cart()
    print()
    explore_itrf2020_to_mga2020()
