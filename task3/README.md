# Task 3 — Object detection from a dataset you built yourself

In this task, we will train a model to detect street signs from dashcam footage. More specfically, our model will attempt to identify different classes of common signs, which are:

- Stop signs
- Give way signs (including roundabout signs)
- Direction signs (green freeway / main-road signs)
- Speed limit signs
- Pedestrian signs
- Other (keep left, detour, roadwork, chevrons, misc regulatory signs, etc.)

## Dataset

**Size:** **635** images used for the training run with a **571 train / 64 validation** random split.

**Collection:** Frames were pulled from several hours of my own dashcam footage (`extract_frames.py`, one frame every 2 seconds) to prevent similar angles from showing up. This produces around 5000 images, which was manually cut down. Only frames with signs were kept, and images during periods of longer stops were removed. Captures cover local streets and highways, in rain, clear sky, sun/glare, and darker conditions. The camera has a fisheye lens, so signs appear at different screen positions with varying distortion. Some signs are partly occluded (e.g. by another vehicle or another sign).

**Labelling:** Done in Label Studio with bounding boxes. Only signs that were clearly identifiable were labelled, to avoid introducing noise to the model. Images can contain one sign or several signs of different classes in the same frame.

### Class distribution

Object counts from the same export that was split for training (**1,329** boxes: 1,190 train / 139 val):


| Class      | Train | Val | Total | Share |
| ---------- | ----- | --- | ----- | ----- |
| speed      | 466   | 44  | 510   | 38.4% |
| other      | 340   | 41  | 381   | 28.7% |
| direction  | 221   | 28  | 249   | 18.7% |
| pedestrian | 109   | 15  | 124   | 9.3%  |
| give way   | 47    | 10  | 57    | 4.3%  |
| stop       | 7     | 1   | 8     | 0.6%  |


This imbalance is expected for real dashcam data (speed and misc signs show up constantly; stops are rare on the routes I drove). This will have effects on evaluation:

- The model is biased toward frequent classes (speed / other / direction).
- **stop** is too scarce, with only one instance in the validation set, so a high stop mAP is not meaningful.
- **other** is large but noisy (many different sign types under one label), which helps explain its weaker precision/recall.



## Model

We use the YOLO11s model, serving as a lighter, fast, but still accurate model. This was trained on Google Collab on the NVDIA T4 GPU. The code for the training can be found in `YOLO_Street_Sign.ipynb`. We train for 60 epochs with an image size of 640, taking about 20 minutes in total.

## Metrics

A full set of metrics can be found in `task3/train`. We will use a combination of metrics to assess evaluation performance. Accuracy alone is insufficient as there may be missed signs, false detections, or inaccurate bounding boxes. 


| Class      | Images | Instances | Precision | Recall | mAP50 | mAP50-95 |
| ---------- | ------ | --------- | --------- | ------ | ----- | -------- |
| all        | 64     | 139       | 0.878     | 0.818  | 0.842 | 0.589    |
| direction  | 23     | 28        | 0.884     | 0.893  | 0.855 | 0.691    |
| give way   | 7      | 10        | 0.976     | 0.900  | 0.906 | 0.639    |
| other      | 26     | 41        | 0.684     | 0.537  | 0.545 | 0.342    |
| pedestrian | 7      | 15        | 1.000     | 0.854  | 0.980 | 0.598    |
| speed      | 21     | 44        | 0.771     | 0.727  | 0.774 | 0.468    |
| stop       | 1      | 1         | 0.955     | 1.000  | 0.995 | 0.796    |


Main metrics to consider:

- **mAP50**: average precision over classes with IoU (intersection over union) ≥ 0.5 (box roughly in the right place and correct class).
- **mAP50-95**: stricter localisation (IoU from 0.5 to 0.95) - shows how tight the boxes are, not just whether a sign was found.
- **Precision / Recall**: whether detections are trustworthy vs whether signs are being missed.

Direction is the best performing class overall, with many examples throughout training. Give way and pedestrain signs also perform well, despite fewer examples. These signs are often quite distinct from other signs, giving us a higher precision and recall. 

Speed signs perform worse. There are a few potential factors. LED signs on freeways where glare may have an effect on classification. During labelling, some LED signs affected by glare were labelled, which may introduce too much noise, resulting in the model incorrectly labelling other objects as speed, reducing precision. Furthermore, speed signs are often round, which the model may confuse with other objects. There are numbers in a speed sign, giving us a greater variety of appearances. Many other signs contains numbers, which may lead to false positives.

Stop reports high metrics, but is unreliable as only one instance is in the validation set. 

## Demo

Streamlit app (`app.py`) for live demo at the debrief: upload or use sample images/videos and run the trained model with a confidence slider.

```powershell
cd task3
.\.venv\Scripts\Activate.ps1
streamlit run app.py
```



## Where it fails

- **other**: The weakest class with a mAP50 of 0.55 and a recall of 0.54. There are many sign types, so the model more easily misses them, or classifies background objects as "other", for example, billboards. 
- **stop / give way**: too few examples to be reliable (stop had 1 val instance), even though strong metrics were reported. In future, more examples should be collected, and class distribution should be analysed before training to ensure there are a sufficient number.

