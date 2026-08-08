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


if __name__ == "__main__":
    from src.parameters import GDA94_TO_GDA2020, ITRF2020_TO_GDA2020, ITRF2014_TO_GDA2020

    # Section 3.1.1: GDA94 -> GDA2020, Alice Springs (ALIC)
    # Expected: [-4052052.7379, 4212835.9897, -2545104.5898]
    XYZ_gda94_alic = np.array([
        -4052051.7643,
         4212836.2017,
        -2545106.0245,
    ])
    # print("3.1.1 GDA94 -> GDA2020 (Alice Springs):")
    # print(helmert7(XYZ_gda94_alic, GDA94_TO_GDA2020))

    # Section 3.3.1: ATRF2014/ITRF2014 -> GDA2020 at epoch 2018.0, Alice Springs (ALIC)
    # Expected: [-4052052.7373, 4212835.9835, -2545104.5867]
    XYZ_itrf2014_alic = np.array([
        -4052052.6588,
         4212835.9938,
        -2545104.6946,
    ])
    # print("\n3.3.1 ITRF2014 -> GDA2020 (Alice Springs, t=2018.0):")
    # print(helmert14(XYZ_itrf2014_alic, ITRF2014_TO_GDA2020, t=2018.0, t0=2020.0))

    # Section 3.5.1: WGS 84 (G2296) -> GDA2020, Melbourne (MOBS)
    # WGS 84 obs on 14 Feb 2024 => coincident with ITRF2020 at mid-year 2024 => t = 2024.5
    # Expected: [-4130636.759, 2894953.142, -3890530.249]
    XYZ_itrf2020_mobs = np.array([
        -4130636.582,
         2894953.120,
        -3890530.446,
    ])
    print("\n3.5.1 WGS84(G2296)/ITRF2020 -> GDA2020 (Melbourne, t=2024.5):")
    print(helmert14(XYZ_itrf2020_mobs, ITRF2020_TO_GDA2020, t=2024.5, t0=2020.0))

