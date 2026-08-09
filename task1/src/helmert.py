"""
Helmert coordinate transformations between terrestrial reference frames.
"""

import numpy as np
from dataclasses import dataclass


# Conversion from arcseconds to radians
_AS_TO_RAD = np.pi / (180 * 3600)


@dataclass(frozen=True)
class HelmertParams:
    """
    Helmert transformation parameters.

    Units:
        translations: metres
        rotations: arcseconds
        scale: ppm
        rates: per year
    """

    tx: float
    ty: float
    tz: float

    rx: float
    ry: float
    rz: float

    sc: float

    # Rates (optional)
    dtx: float = 0.0
    dty: float = 0.0
    dtz: float = 0.0

    drx: float = 0.0
    dry: float = 0.0
    drz: float = 0.0

    dsc: float = 0.0


def helmert14(
    XYZ: np.ndarray,
    params: HelmertParams,
    t: float,
    t0: float,
) -> np.ndarray:
    """
    14-parameter Helmert transformation. Transforms geocentric Cartesian
    coordinates between two reference frames.

    Parameters:
        XYZ: np.ndarray
            Coordinates in the initial frame (metres)
        params: HelmertParams
            Helmert transformation parameters
        t: float
            Observation epoch (years)
        t0: float
            Reference epoch (years)

    Returns:
        np.ndarray
            Target Cartesian coordinates
    """

    dt = t - t0

    # Propagate parameters to observation epoch
    Tx = params.tx + params.dtx * dt
    Ty = params.ty + params.dty * dt
    Tz = params.tz + params.dtz * dt

    # Convert rotations from arcseconds to radians
    Rx = (params.rx + params.drx * dt) * _AS_TO_RAD
    Ry = (params.ry + params.dry * dt) * _AS_TO_RAD
    Rz = (params.rz + params.drz * dt) * _AS_TO_RAD

    # Scale: ppm
    m = 1.0 + (params.sc + params.dsc * dt) * 1e-6

    # Linearised rotation matrix (coordinate-frame convention)
    R = np.array([
        [ 1.0,   Rz,  -Ry],
        [-Rz,   1.0,   Rx],
        [ Ry,  -Rx,   1.0],
    ])

    T = np.array([Tx, Ty, Tz])

    return T + m * (R @ XYZ)


def helmert7(
    XYZ: np.ndarray,
    params: HelmertParams,
) -> np.ndarray:
    """
    7-parameter Helmert transformation with no time dependencies.

    Parameters:
        XYZ: np.ndarray
            Coordinates in the initial frame (metres)
        params: HelmertParams
            Helmert transformation parameters

    Returns:
        np.ndarray
            Target Cartesian coordinates
    """

    return helmert14(
        XYZ,
        params=params,
        t=0.0,
        t0=0.0,
    )

