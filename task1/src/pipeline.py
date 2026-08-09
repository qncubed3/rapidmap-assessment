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


def wgs84_to_mga2020(lat_deg, lon_deg, epoch=2020.0, zone=55):
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
