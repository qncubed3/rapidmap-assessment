"""Small helpers for DMS and approximate ground distances."""

import math


# Mean metres per degree of latitude (good enough for mm–cm demos)
_M_PER_DEG_LAT = 111_320.0


def dms_to_decimal(d: int, m: int, s: float, negative: bool = False) -> float:
    """
    Convert degrees / minutes / seconds to decimal degrees.

    Parameters:
        d: degrees
        m: minutes
        s: seconds
        negative: set True for south latitudes or west longitudes

    Example:
        37°57'03.72030" S  ->  dms_to_decimal(37, 57, 3.72030, negative=True)
    """
    value = d + m / 60.0 + s / 3600.0
    return -value if negative else value


def metres_per_degree_lat() -> float:
    """Approximate metres per degree of latitude."""
    return _M_PER_DEG_LAT


def metres_per_degree_lon(lat_deg: float) -> float:
    """Approximate metres per degree of longitude at a given latitude."""
    return _M_PER_DEG_LAT * math.cos(math.radians(lat_deg))


def degrees_to_metres(dlat: float, dlon: float, lat_deg: float) -> tuple[float, float]:
    """
    Convert latitude/longitude deltas (degrees) to metres.

    Returns:
        (east_m, north_m) — local flat-earth approximation at lat_deg
    """
    north_m = dlat * metres_per_degree_lat()
    east_m = dlon * metres_per_degree_lon(lat_deg)
    return east_m, north_m


def distance_metres_geographic(
    lat1: float, lon1: float, lat2: float, lon2: float
) -> float:
    """Approximate ground distance between two lat/lon points (metres)."""
    east_m, north_m = degrees_to_metres(lat2 - lat1, lon2 - lon1, lat1)
    return math.hypot(east_m, north_m)


def distance_metres_en(e1: float, n1: float, e2: float, n2: float) -> float:
    """Euclidean distance between two easting/northing points (metres)."""
    return math.hypot(e2 - e1, n2 - n1)
