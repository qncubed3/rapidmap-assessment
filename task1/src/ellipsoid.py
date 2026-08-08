"""
Reference ellipsoid definitions.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class Ellipsoid:
    """
    Reference ellipsoid defined by semi-major axis and inverse flattening.

    Units:
        a: metres
        inv_f: dimensionless
    """

    a: float
    inv_f: float

    @property
    def f(self) -> float:
        return 1.0 / self.inv_f

    @property
    def b(self) -> float:
        return self.a * (1.0 - self.f)

    @property
    def e2(self) -> float:
        """First eccentricity squared: ε² = 2f - f²"""
        return 2.0 * self.f - self.f ** 2

    @property
    def n(self) -> float:
        """Third flattening: n = (a - b) / (a + b)"""
        return self.f / (2.0 - self.f)


# GDA2020 and GDA94 both use GRS80
# Source: GDA2020 Technical Manual v1.8, Table 1.2 / Table 1.6
GRS80 = Ellipsoid(a=6378137, inv_f=298.257222101)
