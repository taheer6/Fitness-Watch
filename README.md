# PulsePath Watch

## Overview
This project is a custom wearable watch focused on straight-to-the-point health and navigation data. The watch is being built to show heart rate, time, date, battery, steps, distance travelled, compass heading, and GPS information with a minimalistic UI.

<img width="645" height="647" alt="image" src="https://github.com/user-attachments/assets/f2f73612-1ebe-4456-acde-78cbe67cc1ef" />

## Heart Rate Sensor
The heart rate monitor is built from a custom optical sensing circuit instead of using a prebuilt module. In LTSpice, the photodiode was replaced with a current source that cycles between values so the circuit could be tested using simulated finger pulse behaviour before moving back to real hardware. The actual sensor uses a `BPW34` photodiode and was able to produce a visible pulse on the serial plotter along with a good BPM reading that was tested for accuracy.

<img width="998" height="553" alt="image" src="https://github.com/user-attachments/assets/1623cefa-b25f-4436-a9b1-51a74f4bc0c6" />

One of the first problems was the op amp choice. I originally used an `LM358`, but it was not the right fit for a low-current photodiode signal running on a `3.3V` supply. The sensor needed a rail-to-rail CMOS op amp that could handle this kind of analog front end properly, so I switched to the `OPA2320`, which gave much better results and allowed the pulse waveform to come through more cleanly.

<img width="1002" height="428" alt="image" src="https://github.com/user-attachments/assets/200e9f8f-be78-4ff4-87c0-5d91f3404b34" />

The main issue still left in the pulse sensor is light sensitivity. The `BPW34` is very sensitive and has a relatively large active area, which helps it detect light well but also makes it easier for ambient light and movement to introduce ripple into the signal. The next fix here is better physical shielding around the sensor while also using pulsed LED sampling with ambient light subtraction, where the photodiode is sampled once with the LED off and once with it on, then the two readings are subtracted to remove outside light and isolate blood flow changes.

## Current Build
Right now the pulse sensor is working, the battery system is working, low-battery sleep mode has already been added, and the watch can display time and date. The display being used is the `1.3" 240x240 ST7789 TFT`, and the battery is a `3.7V 100mAh LiPo`, which is small enough to realistically fit inside a watch enclosure.

## What Still Needs Work
The rest of the watch still needs to be integrated into the full build. A `9DOF ICM-20948` will be used for compass heading, wrist-lift wake, hand-drop sleep, and motion features like step counting and distance tracking. GPS still needs to be added, and timekeeping will be backed by an RTC module with a coin cell battery. The biggest hardware step left is designing a compact PCB with mostly surface-mount parts so everything can fit into a proper watch body.

## Conclusion
The core direction is already set. The pulse sensor works, the power system is behaving properly, and the display layout is already heading in the right minimal data-heavy direction. What is left now is tightening the sensor design, adding the remaining motion and GPS features, and moving everything onto a compact PCB so it becomes a complete watch instead of a working prototype.
