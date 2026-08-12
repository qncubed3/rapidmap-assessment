import streamlit as st
from pathlib import Path
from PIL import Image
from ultralytics import YOLO
import cv2
import tempfile

# Absolute path of current file / task3 folder
project_root = Path(__file__).resolve().parent

# Sources
IMAGE = "Image"
VIDEO = "Video"
SOURCES = [IMAGE, VIDEO]

# Image config
IMAGE_DIR = project_root / "web-images"
DEFAULT_IMAGE = IMAGE_DIR / "image1.jpg"
DEFAULT_DETECT_IMAGE = IMAGE_DIR / "detect_image1.jpg"

# Video config
VIDEO_DIR = project_root / "web-videos"
VIDEOS_DICT = {
    "Video 1": VIDEO_DIR / "test_video1.mp4",
}

st.set_page_config(
    page_title="YOLO",
    page_icon=":camera:",
    layout="wide",
)

# Model config
MODEL_DIR = project_root / "models"
DEFAULT_MODEL = MODEL_DIR / "my_model.pt"

# Header
st.header("YOLO Road Sign Detection")

# Sidebar
st.sidebar.title("Configuration")

# Select confidence level
confidence_level = st.sidebar.slider(
    "Confidence Level", min_value=0, max_value=100, value=50, step=1
) / 100

# Load model
try:
    model = YOLO(DEFAULT_MODEL)
except Exception as e:
    st.error(f"Error loading model: {e}")
    model = None

# Image/video config
st.sidebar.header("Image/Video Configuration")
source_radio = st.sidebar.radio("Select Source", SOURCES)

source_image = None
if source_radio == IMAGE:
    source_image = st.sidebar.file_uploader(
        "Upload Image", type=["jpg", "jpeg", "png"]
    )
    col1, col2 = st.columns(2)
    with col1:
        try:
            if source_image is None:
                default_image_path = str(DEFAULT_IMAGE)
                st.image(
                    default_image_path,
                    caption="Default Image",
                    use_container_width=True,
                )
            else:
                uploaded_image = Image.open(source_image)
                st.image(
                    source_image,
                    caption="Uploaded Image",
                    use_container_width=True,
                )
        except Exception as e:
            st.error(f"Error loading image: {e}")
    with col2:
        try:
            if source_image is None:
                default_detect_image_path = str(DEFAULT_DETECT_IMAGE)
                st.image(
                    default_detect_image_path,
                    caption="Detected Image",
                    use_container_width=True,
                )
            else:
                if st.sidebar.button("Detect Objects"):
                    result = model.predict(uploaded_image, conf=confidence_level)
                    boxes = result[0].boxes
                    result_plotted = result[0].plot()[:, :, ::-1]
                    st.image(
                        result_plotted,
                        caption="Detected Image",
                        use_container_width=True,
                    )

                    try:
                        with st.expander("Detection Results"):
                            for box in boxes:
                                st.write(
                                    f"Class: {box.cls[0].item()}, "
                                    f"Confidence: {box.conf[0].item()}, "
                                    f"Bounding Box: {box.xyxy[0].tolist()}"
                                )
                    except Exception as e:
                        st.error(f"Error displaying detection results: {e}")
                else:
                    st.warning(
                        "Please upload an image or select a default image to detect objects"
                    )
        except Exception as e:
            st.error(f"Error loading detect image: {e}")

elif source_radio == VIDEO:
    uploaded_video = st.sidebar.file_uploader(
        "Upload Video", type=["mp4", "avi", "mov", "mkv"]
    )
    source_video = st.sidebar.selectbox(
        "Or choose a default video", list(VIDEOS_DICT.keys())
    )

    if uploaded_video is not None:
        video_bytes = uploaded_video.getvalue()
        st.video(video_bytes)
    else:
        video_path = str(VIDEOS_DICT[source_video])
        st.video(Path(video_path).read_bytes())

    if st.sidebar.button("Detect Video Objects"):
        temp_path = None
        try:
            if uploaded_video is not None:
                suffix = Path(uploaded_video.name).suffix or ".mp4"
                tmp = tempfile.NamedTemporaryFile(delete=False, suffix=suffix)
                tmp.write(uploaded_video.getvalue())
                tmp.close()
                temp_path = tmp.name
                path_to_open = temp_path
            else:
                path_to_open = str(VIDEOS_DICT[source_video])

            video_cap = cv2.VideoCapture(path_to_open)
            if not video_cap.isOpened():
                raise RuntimeError(f"Could not open video: {path_to_open}")

            st_frame = st.empty()
            while True:
                success, image = video_cap.read()
                if not success or image is None or image.size == 0:
                    break

                # image = cv2.resize(image, (640, 480))
                result = model.predict(image, conf=confidence_level, verbose=False)
                result_plotted = result[0].plot()[:, :, ::-1]
                st_frame.image(
                    result_plotted,
                    channels="RGB",
                    use_container_width=True,
                )

            video_cap.release()
        except Exception as e:
            st.error(f"Error detecting video objects: {e}")
            st.warning("Please upload a video or choose a default one")
        finally:
            if temp_path and Path(temp_path).exists():
                try:
                    Path(temp_path).unlink()
                except OSError:
                    pass
