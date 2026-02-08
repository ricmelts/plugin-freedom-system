# Parameter Specification: RedShiftDistortion

**Status:** Final
**Generated:** 2026-02-08 (from architecture.md)
**Source:** Stage 0 Architecture (Parameter Mapping table)

This specification defines all APVTS parameters for RedShiftDistortion.

---

## Main Controls

### stereoWidth
- **Type:** Float
- **Range:** -100.0 to 100.0 %
- **Default:** 0.0 %
- **DSP Purpose:** L/R delay differential for spatial positioning (±260ms max)
- **UI Label:** "Stereo Width"
- **Units:** %

### feedback
- **Type:** Float
- **Range:** 0.0 to 95.0 %
- **Default:** 0.0 %
- **DSP Purpose:** Feedback gain (0.0 to 0.95) for tape delay loop
- **UI Label:** "Feedback"
- **Units:** %

### filterBandLow
- **Type:** Float
- **Range:** 20.0 to 20000.0 Hz
- **Default:** 100.0 Hz
- **DSP Purpose:** Highpass cutoff in feedback loop (removes low frequencies)
- **UI Label:** "Lo-Cut"
- **Units:** Hz

### filterBandHigh
- **Type:** Float
- **Range:** 20.0 to 20000.0 Hz
- **Default:** 8000.0 Hz
- **DSP Purpose:** Lowpass cutoff in feedback loop (removes high frequencies)
- **UI Label:** "Hi-Cut"
- **Units:** Hz

### dopplerShift
- **Type:** Float
- **Range:** -50.0 to 50.0 %
- **Default:** 0.0 %
- **DSP Purpose:** Pitch shift per repeat (±12 semitones, CUMULATIVE in feedback loop)
- **UI Label:** "Doppler Shift"
- **Units:** %

### saturation
- **Type:** Float
- **Range:** -12.0 to 24.0 dB
- **Default:** 0.0 dB
- **DSP Purpose:** Tube saturation drive (0.25x to 15.85x gain, CUMULATIVE in feedback)
- **UI Label:** "Saturation"
- **Units:** dB

### masterOutput
- **Type:** Float
- **Range:** -60.0 to 12.0 dB
- **Default:** 0.0 dB
- **DSP Purpose:** Final output level control after all processing
- **UI Label:** "Master Output"
- **Units:** dB

---

## Bypass Controls

### bypassStereoWidth
- **Type:** Bool
- **Default:** false (Off)
- **DSP Purpose:** Bypass stereo width modulation (sets stereoControl to 0.0)
- **UI Label:** "Bypass Stereo Width"

### bypassDelay
- **Type:** Bool
- **Default:** false (Off)
- **DSP Purpose:** Bypass entire tape delay + feedback loop
- **UI Label:** "Bypass Delay"

### bypassDoppler
- **Type:** Bool
- **Default:** false (Off)
- **DSP Purpose:** Bypass granular pitch shifting (keeps delay + saturation)
- **UI Label:** "Bypass Doppler"

### bypassSaturation
- **Type:** Bool
- **Default:** false (Off)
- **DSP Purpose:** Bypass tube saturation stage
- **UI Label:** "Bypass Saturation"

---

## Advanced Settings

### grainSize
- **Type:** Float
- **Range:** 25.0 to 200.0 ms
- **Default:** 100.0 ms
- **DSP Purpose:** Grain buffer size for granular pitch shifting (quality vs CPU)
- **UI Label:** "Grain Size"
- **Units:** ms

### grainOverlap
- **Type:** Choice
- **Options:** "2x" (value: 0), "4x" (value: 1)
- **Default:** "4x" (value: 1)
- **DSP Purpose:** Simultaneous grain count (2x = lower CPU, 4x = smoother)
- **UI Label:** "Grain Overlap"

---

## Total Parameter Count

**Total:** 13 parameters
- 7 main controls (float)
- 4 bypass controls (bool)
- 2 advanced settings (1 float, 1 choice)

---

## Parameter Dependencies

- **stereoWidth** creates spatial positioning (independent of dopplerShift)
- **dopplerShift** creates pitch movement (CUMULATIVE in feedback loop)
- **feedback** amplifies cumulative doppler + saturation effects
- **grainSize** vs **grainOverlap** tradeoff:
  - Larger grainSize = smoother sound, more latency, less CPU per grain
  - Higher grainOverlap = smoother sound, more CPU (more simultaneous grains)
  - Recommended: grainSize=100ms, grainOverlap=4x for best quality

---

## Notes

- All parameters managed via APVTS (AudioProcessorValueTreeState)
- Automatic XML serialization for presets
- No custom state management needed
- Thread-safe atomic parameter reads in audio thread
