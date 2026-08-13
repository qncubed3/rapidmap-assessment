"""
Minimal Streamlit UI for WGS84 (G2296) <-> MGA2020 Zone 55.

Run from task1/:
    streamlit run app.py
"""

import streamlit as st
from pyproj import Transformer

from src.pipeline import wgs84_to_mga2020, mga2020_to_wgs84
from src.utils import distance_metres_en, distance_metres_geographic


_EPOCH = 2020.0

st.set_page_config(page_title="Task 1 converter", layout="centered")
st.title("WGS84 ↔ MGA2020 Zone 55")

direction = st.radio(
    "Direction",
    ("WGS84 (G2296) → MGA2020 Zone 55", "MGA2020 Zone 55 → WGS84 (G2296)"),
)

if direction.startswith("WGS84"):
    c1, c2 = st.columns(2)
    lat_s = c1.text_input("Latitude (°)", value="-37.9510334167")
    lon_s = c2.text_input("Longitude (°)", value="144.4248678889")

    if st.button("Convert"):
        lat = float(lat_s)
        lon = float(lon_s)
        E, N, zone = wgs84_to_mga2020(lat, lon, epoch=_EPOCH)

        t_g2296 = Transformer.from_crs("EPSG:10606", "EPSG:7855", always_xy=True)
        E_g2296, N_g2296 = t_g2296.transform(lon, lat)

        t_itrf = Transformer.from_crs("EPSG:9990", "EPSG:7855", always_xy=True)
        E_itrf, N_itrf = t_itrf.transform(lon, lat, tt=_EPOCH)[:2]

        st.subheader("Results")
        st.table({
            "Easting (m)": {
                "Ours": f"{E:.6f}",
                "pyproj G2296 (EPSG:10606)": f"{E_g2296:.6f}",
                "pyproj ITRF2020 (EPSG:9990)": f"{E_itrf:.6f}",
            },
            "Northing (m)": {
                "Ours": f"{N:.6f}",
                "pyproj G2296 (EPSG:10606)": f"{N_g2296:.6f}",
                "pyproj ITRF2020 (EPSG:9990)": f"{N_itrf:.6f}",
            },
        })
        st.caption(f"Zone {zone}")
        st.write(
            f"Difference vs G2296 (EPSG:10606): `{distance_metres_en(E, N, E_g2296, N_g2296):.6f}` m"
        )
        st.write(
            f"Difference vs ITRF2020 (EPSG:9990): `{distance_metres_en(E, N, E_itrf, N_itrf):.6f}` m"
        )

else:
    c1, c2 = st.columns(2)
    easting_s = c1.text_input("Easting (m)", value="273741.297")
    northing_s = c2.text_input("Northing (m)", value="5796489.777")

    if st.button("Convert"):
        easting = float(easting_s)
        northing = float(northing_s)
        lat, lon = mga2020_to_wgs84(easting, northing, epoch=_EPOCH)

        t_g2296 = Transformer.from_crs("EPSG:7855", "EPSG:10606", always_xy=True)
        lon_g2296, lat_g2296 = t_g2296.transform(easting, northing)

        t_itrf = Transformer.from_crs("EPSG:7855", "EPSG:9990", always_xy=True)
        lon_itrf, lat_itrf = t_itrf.transform(easting, northing, tt=_EPOCH)[:2]

        st.subheader("Results")
        st.table({
            "Latitude (°)": {
                "Ours": f"{lat:.10f}",
                "pyproj G2296 (EPSG:10606)": f"{lat_g2296:.10f}",
                "pyproj ITRF2020 (EPSG:9990)": f"{lat_itrf:.10f}",
            },
            "Longitude (°)": {
                "Ours": f"{lon:.10f}",
                "pyproj G2296 (EPSG:10606)": f"{lon_g2296:.10f}",
                "pyproj ITRF2020 (EPSG:9990)": f"{lon_itrf:.10f}",
            },
        })
        st.write(
            "Difference vs G2296 (EPSG:10606): "
            f"`{distance_metres_geographic(lat, lon, lat_g2296, lon_g2296):.6f}` m"
        )
        st.write(
            "Difference vs ITRF2020 (EPSG:9990): "
            f"`{distance_metres_geographic(lat, lon, lat_itrf, lon_itrf):.6f}` m"
        )
