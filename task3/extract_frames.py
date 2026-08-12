"""Pull frames from all mp4s under VIDEO_ROOT into extracted_frames/."""

import cv2
from pathlib import Path

VIDEO_ROOT = Path(r"D:\Video")  # dashcam dump
OUTPUT_DIR = Path(__file__).resolve().parent / "extracted_frames"
INTERVAL_SECONDS = 2  # one frame every N seconds


def find_mp4s(directory):
    # case-sensitive on linux but windows doesn't care
    return sorted(p for p in directory.rglob("*.mp4") if p.is_file())


def extract_frames(video_path, output_dir, interval_seconds):
    cap = cv2.VideoCapture(str(video_path))
    if not cap.isOpened():
        print(f"couldn't open {video_path}")
        return 0

    fps = cap.get(cv2.CAP_PROP_FPS)
    n_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    if fps <= 0 or n_frames <= 0:
        print(f"bad fps/frame count for {video_path}")
        cap.release()
        return 0

    step = max(1, round(fps * interval_seconds))
    name = video_path.stem
    extracted = 0
    i = 0

    print(f"\n{video_path}")
    print(f"  {fps:.1f} fps, {n_frames} frames, ~{n_frames / fps / 60:.1f} min")

    while True:
        ok, frame = cap.read()
        if not ok:
            break

        if i % step == 0:
            # keep original frame index in the filename
            out = output_dir / f"{name}_frame_{i:08d}.jpg"
            if cv2.imwrite(str(out), frame, [cv2.IMWRITE_JPEG_QUALITY, 95]):
                extracted += 1

        i += 1
        print(
            f"\r  {100 * i / n_frames:5.1f}%  {i}/{n_frames}  saved {extracted}",
            end="",
            flush=True,
        )

    cap.release()
    print(f"\n  done — {extracted} frames")
    return extracted


def main():
    if not VIDEO_ROOT.is_dir():
        raise SystemExit(f"VIDEO_ROOT not found: {VIDEO_ROOT}")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    videos = find_mp4s(VIDEO_ROOT)

    print(f"input:  {VIDEO_ROOT}")
    print(f"output: {OUTPUT_DIR}")
    print(f"found {len(videos)} mp4(s)")
    if not videos:
        return

    total = 0
    for n, video in enumerate(videos, 1):
        print(f"\n[{n}/{len(videos)}]")
        total += extract_frames(video, OUTPUT_DIR, INTERVAL_SECONDS)

    print(f"\nall done — {total} frames from {len(videos)} videos -> {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
