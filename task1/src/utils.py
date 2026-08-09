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
