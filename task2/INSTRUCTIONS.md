# Task 2 — Rasterisation at scale
Build the fastest rasteriser you reasonably can, converting large WGS84 vector datasets into imagery:

- 100,000 → 1,000,000 → 100,000,000 points
- 100,000 → 1,000,000 → 100,000,000 lines
- 100,000 → 1,000,000 → 100,000,000 polygons

**The top of that ladder may not be reachable on your hardware, and that is fine.** If 100 million defeats you, show us where it broke, what the binding constraint actually was — memory, I/O, CPU — and what you would need to get past it. A well-evidenced "this is the wall and here is why" is worth more to us than a number you cannot stand behind.

**What we're looking for:**

- **A reproducible benchmark**, not just a number. Someone else should be able to clone the repo, run one command, and get comparable timings on their own machine. State your hardware, and state clearly what is and isn't included in the timing (data generation? file I/O? encoding the image?).
- **Honest numbers.** A slower approach reported accurately beats a faster one reported loosely.
- **Your reasoning about the approach**, and what you tried that didn't work. Dead ends are interesting to us.

**Things the brief doesn't tell you** — decide, and write down what you decided: output resolution and extent; the coordinate system you render in and why; whether polygons are filled, stroked or both; what happens where features overlap; whether antialiasing matters; how you generate realistic test data at each scale, and how much the shape of that synthetic data changes your benchmark.

**Two questions we'll ask at the debrief**, so have a view:

- Where does your approach break down, and what would you change to get past it?
- Is the fastest thing you built the thing you would actually put into production? If not, what's the difference?

Language, libraries and presentation layer are entirely your choice.

