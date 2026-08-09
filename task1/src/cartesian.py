"""
Conversions between geocentric Cartesian (X, Y, Z) and geodetic geographic
(latitude, longitude, ellipsoidal height) coordinates.

Equations follow the GDA2020 Technical Manual v1.8, Section 2.1.2, equations 1-13.
"""

import numpy as np

from .ellipsoid import Ellipsoid, GRS80


def cartesian_to_geographic(
    XYZ: np.ndarray,
    ellipsoid: Ellipsoid = GRS80,
) -> tuple[float, float, float]:
    """
    Convert geocentric Cartesian coordinates to geodetic geographic coordinates.

    Uses the Bowring parametric latitude method, equations 1-7, page 24

    Parameters:
        XYZ: np.ndarray
            Geocentric Cartesian coordinates [X, Y, Z] in metres.
        ellipsoid: Ellipsoid
            Reference ellipsoid (default GRS80).

    Returns:
        (lat_deg, lon_deg, h)
            Geodetic latitude (degrees, positive north),
            longitude (degrees, positive east),
            ellipsoidal height (metres).
    """
    X, Y, Z = XYZ
    a   = ellipsoid.a
    f   = ellipsoid.f
    e2  = ellipsoid.e2

    # Eq. 1 — longitude
    lam = np.arctan2(Y, X)

    # Eq. 4 — distance from Z-axis
    p = np.sqrt(X**2 + Y**2)

    # Eq. 6 — geocentric distance
    r = np.sqrt(p**2 + Z**2)

    # Eq. 5 — parametric (reduced) latitude u
    u = np.arctan((Z / p) * ((1.0 - f) + e2 * a / r))

    # Eq. 2 — geodetic latitude φ
    numerator   = Z * (1.0 - f) + e2 * a * np.sin(u) ** 3
    denominator = (1.0 - f) * (p - e2 * a * np.cos(u) ** 3)
    phi = np.arctan2(numerator, denominator)

    # Eq. 3 — ellipsoidal height
    h = p * np.cos(phi) + Z * np.sin(phi) - a * np.sqrt(1.0 - e2 * np.sin(phi) ** 2)

    return np.degrees(phi), np.degrees(lam), h


def geographic_to_cartesian(
    lat_deg: float,
    lon_deg: float,
    h: float,
    ellipsoid: Ellipsoid = GRS80,
) -> np.ndarray:
    """
    Convert geodetic geographic coordinates to geocentric Cartesian coordinates.

    Uses Equations 8-12, page 25.

    Parameters:
        lat_deg: float          Geodetic latitude in degrees (positive north).
        lon_deg: float          Geodetic longitude in degrees (positive east).
        h: float                Ellipsoidal height in metres.
        ellipsoid: Ellipsoid    Reference ellipsoid (default GRS80).

    Returns:
        np.ndarray  [X, Y, Z] geocentric Cartesian coordinates in metres.
    """
    phi = np.radians(lat_deg)
    lam = np.radians(lon_deg)
    a   = ellipsoid.a
    e2  = ellipsoid.e2

    # Eq. 11 — radius of curvature in the prime vertical
    nu = a / np.sqrt(1.0 - e2 * np.sin(phi) ** 2)

    # Eqs. 8–10
    X = (nu + h) * np.cos(phi) * np.cos(lam)
    Y = (nu + h) * np.cos(phi) * np.sin(lam)
    Z = ((1.0 - e2) * nu + h) * np.sin(phi)

    return np.array([X, Y, Z])
