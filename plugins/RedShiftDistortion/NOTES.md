# RedShiftDistortion Notes

## Status
- **Current Status:** 📦 Installed
- **Version:** 1.0.0
- **Type:** Audio Effect (Distortion/Delay)

## Lifecycle Timeline

- **2026-02-09:** Installed to system folders (VST3 + AU)
- **2026-02-08:** Stage 3 complete - GUI implementation finished
- **2026-02-08:** Architecture finalized (granular doppler shift design)
- **2026-02-05:** Plugin ideation - Creative brief created

## Known Issues

- None

## Additional Notes

**Installation Locations:**
- VST3: `~/Library/Audio/Plug-Ins/VST3/RedShiftDistortion.vst3`
- AU: `~/Library/Audio/Plug-Ins/Components/RedShiftDistortion.component`

**Formats:** VST3, AU, Standalone

**Description:**
Series tape delay processor with cumulative granular doppler shift and progressive saturation buildup. Creates the sonic equivalent of astronomical red shift through true unidirectional pitch shifting in the feedback loop.

**Key Features:**
- Cumulative granular doppler shift (±12 semitones per repeat)
- Independent stereo width modulation (±260ms L/R differential)
- Tube saturation in feedback loop
- Feedback filters (lo-cut + hi-cut)
- 13 parameters (7 main controls, 4 bypass switches, 2 advanced settings)
- WebView UI (950×700px, industrial metal aesthetic)

**Parameters:**
1. Stereo Width (-100% to +100%)
2. Feedback (0% to 95%)
3. Lo-Cut Filter (20Hz to 20kHz)
4. Hi-Cut Filter (20Hz to 20kHz)
5. Doppler Shift (-50% to +50%, ±12 semitones cumulative)
6. Saturation (-12dB to +24dB)
7. Master Output (-60dB to +12dB)
8-11. Bypass switches (Stereo Width, Delay, Doppler, Saturation)
12. Grain Size (25-200ms)
13. Grain Overlap (2x or 4x)

**DSP Architecture:**
Input → Stereo Width → Tape Delay (Feedback: Granular Doppler → Saturation → Filters) → Master Output
