"""
Minimal Streamlit UI for WGS84 (G2296) <-> MGA2020 Zone 55.

Run from task1/:
    streamlit run app.py
"""

import streamlit as st
from pyproj import Transformer

from src.pipeline import wgs84_to_mga2020, mga2020_to_wgs84
from src.utils import distance_metres_en, distance_metres_geographic


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
        E, N, zone = wgs84_to_mga2020(lat, lon)
        t = Transformer.from_crs("EPSG:10606", "EPSG:7855", always_xy=True)
        E_p, N_p = t.transform(lon, lat)
        diff = distance_metres_en(E, N, E_p, N_p)

        st.subheader("Results")
        st.table({
            "Easting (m)": {"Ours": f"{E:.6f}", "pyproj": f"{E_p:.6f}"},
            "Northing (m)": {"Ours": f"{N:.6f}", "pyproj": f"{N_p:.6f}"},
        })
        st.caption(f"Zone {zone}")
        st.write(f"Difference: `{diff:.6f}` m")

else:
    c1, c2 = st.columns(2)
    easting_s = c1.text_input("Easting (m)", value="273741.297")
    northing_s = c2.text_input("Northing (m)", value="5796489.777")

    if st.button("Convert"):
        easting = float(easting_s)
        northing = float(northing_s)
        lat, lon = mga2020_to_wgs84(easting, northing)
        t = Transformer.from_crs("EPSG:7855", "EPSG:10606", always_xy=True)
        lon_p, lat_p = t.transform(easting, northing)
        diff = distance_metres_geographic(lat, lon, lat_p, lon_p)

        st.subheader("Results")
        st.table({
            "Latitude (°)": {"Ours": f"{lat:.10f}", "pyproj": f"{lat_p:.10f}"},
            "Longitude (°)": {"Ours": f"{lon:.10f}", "pyproj": f"{lon_p:.10f}"},
        })
        st.write(f"Difference: `{diff:.6f}` m")
