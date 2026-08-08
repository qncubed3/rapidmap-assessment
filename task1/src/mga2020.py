"""
Geographic to MGA2020 grid coordinate conversion using the Krueger n-series
(transverse Mercator) equations.

Source: GDA2020 Technical Manual v1.8
  - Section 4.1.1.1  Forward transformation (geographic -> grid), Equations 19-33
  - Section 4.1.1.2  Inverse transformation (grid -> geographic), Equations 38-53
  - Appendix C, Table C-1  Sample data for verification
"""

import numpy as np

from .ellipsoid import Ellipsoid, GRS80


# MGA2020 / UTM projection constants
K0 = 0.9996           # central scale factor
FALSE_EASTING  = 500_000.0
FALSE_NORTHING = 10_000_000.0  # southern hemisphere


def central_meridian_degrees(zone):
    """Return the central meridian longitude in degrees for a given zone."""
    return 6.0 * zone - 183.0


def geographic_to_mga2020(lat_deg, lon_deg, zone=55, ellipsoid=GRS80):
    """
    Convert geographic coordinates (GDA2020) to MGA2020 grid coordinates.

    Source: GDA2020 Technical Manual v1.8, Section 4.1.1.1, Equations 19-33.

    Parameters:
        lat_deg: latitude in decimal degrees (negative for south)
        lon_deg: longitude in decimal degrees
        zone:    MGA2020 zone number (default 55)

    Returns:
        (easting, northing, zone) in metres
    """
    # convert degrees to radians
    lat = np.radians(lat_deg)
    lon = np.radians(lon_deg)
    lon0 = np.radians(central_meridian_degrees(zone))

    a = ellipsoid.a
    n = ellipsoid.n
    e = np.sqrt(ellipsoid.e2)

    # Eq. 21 — rectifying radius
    A = (a / (1 + n)) * (1 + n**2/4 + n**4/64 + n**6/256 + 25*n**8/16384)

    # Eq. 22 — series coefficients alpha (1 through 8)
    # index 0 is unused padding so we can write alpha[1], alpha[2], etc.
    alpha = [0.0] * 9

    alpha[1] = (n/2 - 2*n**2/3 + 5*n**3/16 + 41*n**4/180
                - 127*n**5/288 + 7891*n**6/37800 + 72161*n**7/387072
                - 18975107*n**8/50803200)

    alpha[2] = (13*n**2/48 - 3*n**3/5 + 557*n**4/1440 + 281*n**5/630
                - 1983433*n**6/1935360 + 13769*n**7/28800
                + 148003883*n**8/174182400)

    alpha[3] = (61*n**3/240 - 103*n**4/140 + 15061*n**5/26880
                + 167603*n**6/181440 - 67102379*n**7/29030400
                + 79682431*n**8/79833600)

    alpha[4] = (49561*n**4/161280 - 179*n**5/168 + 6601661*n**6/7257600
                + 97445*n**7/49896 - 40176129013*n**8/7664025600)

    alpha[5] = (34729*n**5/80640 - 3418889*n**6/1995840
                + 14644087*n**7/9123840 + 2605413599*n**8/622702080)

    alpha[6] = (212378941*n**6/319334400 - 30705481*n**7/10378368
                + 175214326799*n**8/58118860800)

    alpha[7] = 1522256789*n**7/1383782400 - 16759934899*n**8/3113510400

    alpha[8] = 1424729850961*n**8/743921418240

    # Eq. 24 — sigma
    sin_lat = np.sin(lat)
    sigma = np.sinh(e * np.arctanh(e * sin_lat))

    # Eq. 23 — conformal latitude (as tangent)
    tan_lat = np.tan(lat)
    tan_conf_lat = tan_lat * np.sqrt(1 + sigma**2) - sigma * np.sqrt(1 + tan_lat**2)

    # Eq. 25 — longitude difference from central meridian
    d_lon = lon - lon0

    # Eqs. 26-27 — Gauss-Schreiber coordinates
    xi_prime  = np.arctan2(tan_conf_lat, np.cos(d_lon))
    eta_prime = np.arcsinh(np.sin(d_lon) / np.sqrt(tan_conf_lat**2 + np.cos(d_lon)**2))

    # Eqs. 28-29 — apply series correction (8 terms)
    xi  = xi_prime
    eta = eta_prime
    for r in range(1, 9):
        xi  += alpha[r] * np.sin(2*r * xi_prime) * np.cosh(2*r * eta_prime)
        eta += alpha[r] * np.cos(2*r * xi_prime) * np.sinh(2*r * eta_prime)

    # Eqs. 30-31 — transverse Mercator X, Y
    X = A * eta
    Y = A * xi

    # Eqs. 32-33 — apply scale factor and false origin
    easting  = K0 * X + FALSE_EASTING
    northing = K0 * Y + FALSE_NORTHING

    return easting, northing, zone


def mga2020_to_geographic(easting, northing, zone=55, ellipsoid=GRS80):
    """
    Convert MGA2020 grid coordinates back to geographic coordinates (GDA2020).

    Source: GDA2020 Technical Manual v1.8, Section 4.1.1.2, Equations 38-53.

    Parameters:
        easting:  MGA2020 easting in metres
        northing: MGA2020 northing in metres
        zone:     MGA2020 zone number (default 55)

    Returns:
        (lat_deg, lon_deg) in decimal degrees
    """
    lon0 = np.radians(central_meridian_degrees(zone))

    n = ellipsoid.n
    a = ellipsoid.a
    e = np.sqrt(ellipsoid.e2)

    # Eq. 21 — rectifying radius
    A = (a / (1 + n)) * (1 + n**2/4 + n**4/64 + n**6/256 + 25*n**8/16384)

    # Eq. 38 — series coefficients beta (1 through 8)
    beta = [0.0] * 9

    beta[1] = (-n/2 + 2*n**2/3 - 37*n**3/96 + n**4/360
               + 81*n**5/512 - 96199*n**6/604800 + 5406467*n**7/38707200
               - 7944359*n**8/67737600)

    beta[2] = (-n**2/48 - n**3/15 + 437*n**4/1440 + 46*n**5/105
               - 1118711*n**6/3870720 + 51841*n**7/1209600
               + 24749483*n**8/348364800)

    beta[3] = (-17*n**3/480 + 37*n**4/840 + 209*n**5/4480
               - 5569*n**6/90720 - 9261899*n**7/58060800
               + 6457463*n**8/17740800)

    beta[4] = (-4397*n**4/161280 + 11*n**5/504 + 830251*n**6/7257600
               + 466511*n**7/2494800 - 324154477*n**8/7664025600)

    beta[5] = (-4583*n**5/161280 + 108847*n**6/3991680
               - 8005831*n**7/63866880 - 22894433*n**8/124540416)

    beta[6] = (-20648693*n**6/638668800 + 16363163*n**7/518918400
               + 2204645983*n**8/12915302400)

    beta[7] = -219941297*n**7/5535129600 + 497323811*n**8/12454041600

    beta[8] = -191773887257*n**8/3719607091200

    # Eqs. 39-40 — remove scale factor and false origin
    X = (easting  - FALSE_EASTING)  / K0
    Y = (northing - FALSE_NORTHING) / K0

    # Eqs. 41-42 — normalise by rectifying radius
    eta = X / A
    xi  = Y / A

    # Eqs. 43-44 — apply inverse series correction (8 terms)
    eta_prime = eta
    xi_prime  = xi
    for r in range(1, 9):
        eta_prime += beta[r] * np.cos(2*r * xi) * np.sinh(2*r * eta)
        xi_prime  += beta[r] * np.sin(2*r * xi) * np.cosh(2*r * eta)

    # Eq. 45 — conformal latitude as tangent
    t_prime = np.sin(xi_prime) / np.sqrt(np.sinh(eta_prime)**2 + np.cos(xi_prime)**2)

    # Eqs. 46-51 — Newton-Raphson iteration to recover geodetic latitude
    t = t_prime
    for _ in range(10):
        sigma  = np.sinh(e * np.arctanh(e * t / np.sqrt(1 + t**2)))
        f      = t * np.sqrt(1 + sigma**2) - sigma * np.sqrt(1 + t**2) - t_prime
        f_dash = ((np.sqrt(1 + sigma**2) * np.sqrt(1 + t**2) - sigma * t)
                  * (1 - e**2) * np.sqrt(1 + t**2)
                  / (1 + (1 - e**2) * t**2))
        t -= f / f_dash

    # Eq. 51 — geodetic latitude
    lat = np.arctan(t)

    # Eqs. 52-53 — geodetic longitude
    d_lon = np.arctan2(np.sinh(eta_prime), np.cos(xi_prime))
    lon   = lon0 + d_lon

    return np.degrees(lat), np.degrees(lon)
