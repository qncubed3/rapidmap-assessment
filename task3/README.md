# Task 3 — Object detection from a dataset you built yourself
In this task, we will train a model to detect street signs from dashcam footage. More specfically, our model will attempt to identify different classes of common signs, which are:
- Stop signs
- Give way signs (including roundabout signs)
- Direction signs (the green signs on freeways and main roads)
- Speed limit signs
- Pedestrian signs
We will also include a class called "other" signs, which are for miscellaneous road signs such as "keep left", detour, roadwork, chevrons, and regulatory signs to name a few.

## Training Set
677 images, extracted from several hours of dashcam footage. Location of capture range from local street to highways, in various conditions, including rain, clear skies, sun (with glare), and darker conditions. Dashcam footage has a fisheye effect, so signs are also capture are multiple locations on screen with varying degrees of disortion. Some signs are also captured with partial obstructures, such as a vehicle in front.

## Labelling
Images are labelled with Label Studio. Only signs which were identifiable were labelled to avoid introducing noise to the model.

TODO: Finish writeup