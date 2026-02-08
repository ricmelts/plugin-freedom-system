# RedShiftDistortion - Creative Brief

## Overview

**Type:** Audio Effect
**Core Concept:** Series tape delay with separated stereo width modulation and cumulative granular doppler shift, creating the sonic equivalent of astronomical red shift
**Status:** 💡 Ideated (Architecture Finalized 2026-02-08)
**Created:** 2026-02-05
**Revised:** 2026-02-08 (Final - Stereo Width + Cumulative Doppler)

## Vision

RedShiftDistortion is a series-chain tape delay processor that combines independent stereo width modulation with cumulative granular doppler shift and progressive saturation buildup. Inspired by the astronomical phenomenon of red shift—where light from receding objects shifts toward the red spectrum—this plugin translates that concept into the audio domain through **true unidirectional pitch shifting**.

The key innovation is **cumulative doppler shift in the feedback loop**: each delay repeat shifts the pitch further, creating an escalating frequency shift that mimics approaching (blue shift, pitch rises) or receding (red shift, pitch falls) sound sources. This is achieved through granular synthesis pitch shifting, not simple delay time modulation.

Stereo width modulation is handled separately via differential L/R delay times, allowing users to independently control spatial positioning and frequency shifting—two conceptually distinct effects.

The holographic, 3D UI with reactive VU metering provides tactile visual feedback that matches the dimensional quality of the sound.

## Parameters

### Main Controls

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Stereo Width** | -100% to +100% | 0% | Spatial positioning via L/R delay differential (±260ms) |
| **Feedback** | 0-95% | 0% | Delay feedback amount (higher = more repeats) |
| **Filter Low (Hi-Pass)** | 20-20000 Hz | 100 Hz | Highpass filter in feedback path (removes low frequencies) |
| **Filter High (Lo-Pass)** | 20-20000 Hz | 8000 Hz | Lowpass filter in feedback path (removes high frequencies) |
| **Doppler Shift** | -50% to +50% | 0% | Pitch shift per repeat (±12 semitones, **CUMULATIVE**) |
| **Saturation** | -12dB to +24dB | 0dB | Tube drive amount (**CUMULATIVE** in feedback loop) |
| **Master Output** | -60dB to +12dB | 0dB | Final output level |

### Bypass Controls (Troubleshooting)

| Parameter | Default | Description |
|-----------|---------|-------------|
| **Bypass Stereo Width** | Off | Bypass stereo width modulation (mono output) |
| **Bypass Delay** | Off | Bypass entire delay + feedback loop |
| **Bypass Doppler** | Off | Bypass pitch shifting (keeps delay + saturation) |
| **Bypass Saturation** | Off | Bypass saturation stage (keeps delay + doppler) |

### Advanced Settings (Granular Quality Control)

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Grain Size** | 25-200 ms | 100 ms | Granular grain buffer size (quality vs CPU trade-off) |
| **Grain Overlap** | 2x or 4x | 4x | Simultaneous grains (2x = lower CPU, 4x = smoother) |

**Total Parameters:** 15 (7 main + 4 bypass + 2 advanced)

## Key Concepts

### Separated Stereo Width and Doppler Shift

**Stereo Width** = Spatial positioning (left/right separation)
- Negative values = narrow stereo (sounds distant)
- Zero = mono (no spatial width)
- Positive values = wide stereo (sounds present/close)

**Doppler Shift** = Frequency shift (pitch rising/falling)
- Negative values = red shift (pitch descends with each repeat, receding sound)
- Zero = no pitch shift
- Positive values = blue shift (pitch rises with each repeat, approaching sound)

These are **independent effects**—you can have stereo width without doppler, or doppler without stereo width.

### Cumulative Doppler Effect

The doppler shift is **CUMULATIVE** in the feedback loop:
- Each delay repeat applies another pitch shift
- Example with dopplerShift = +50% (+12 semitones = +1 octave):
  - Original signal: 440 Hz
  - Repeat 1: 880 Hz (+1 octave)
  - Repeat 2: 1760 Hz (+2 octaves)
  - Repeat 3: 3520 Hz (+3 octaves)
  - Eventually shifts out of audible range (natural decay)

This creates true **unidirectional pitch movement** (pitch consistently rises or falls), not oscillating wow & flutter.

## UI Concept

**Layout:**
- Top section: Main controls (stereo width, doppler shift, saturation, feedback, filters, master output)
- Middle section: Bypass toggles (stereo width, delay, doppler, saturation)
- Bottom section: Advanced settings panel (grain size, grain overlap) - collapsible
- Right side: VU meter with holographic aesthetic

**Visual Style:**
- Holographic/3D elements with depth and dimensionality
- Reactive VU meter that responds to output signal
- Tactile knob feel with physical feedback visualization
- Color scheme suggests space/astronomy (deep blues, purples, reds shifting based on doppler direction)
- Advanced settings panel can be collapsed to hide grain controls

**Key Elements:**
- Large central doppler shift knob (primary creative control)
- Stereo width knob (separate from doppler)
- Saturation knob
- Feedback control with filter knobs
- VU meter with holographic glow
- Bypass toggles for each stage
- Collapsible advanced panel for grain quality settings

## Use Cases

- **Vocal processing:** Add cumulative pitch shifting for creative risers/fallers with stereo width
- **Synth textures:** Create escalating doppler effects that build with feedback
- **Guitar effects:** Combine stereo width + doppler for spatial and frequency motion
- **FX design:** Generate sci-fi sounds with extreme cumulative pitch shifts (approaching/receding spacecraft)
- **Mixing depth:** Use stereo width for spatial positioning, doppler for frequency movement
- **Tape echo emulation:** Authentic feedback loop saturation with frequency-shifting repeats
- **Riser/faller effects:** Cumulative blue shift creates pitch-rising risers, red shift creates pitch-falling fallers

## Inspirations

- Astronomical red shift phenomenon (Doppler effect in light wavelengths)
- Physical doppler effect in audio (approaching/receding sound sources)
- Classic tape delay feedback loops (RE-201 Space Echo, Echoplex)
- Tube amplifier saturation and harmonic enhancement
- Granular synthesis pitch shifting (FabFilter Timeless, Eventide H3000)
- Modern cumulative feedback effects (Eventide pitch shifters with feedback)

## Technical Notes

### DSP Architecture (Finalized - Series Processing with Cumulative Doppler)

**Signal Flow:**
Input → Stereo Width → Tape Delay (with Feedback: Granular Doppler → Saturation → Filters) → Master Output

**Stereo Width:**
- Differential L/R delay times (±260ms max)
- Static spatial positioning (no LFO modulation)
- Independent of doppler shift

**Granular Doppler Shift:**
- Granular synthesis with pitch-shifted grain playback
- 4-grain overlap (or 2-grain for lower CPU)
- Hann windowing for smooth grain boundaries
- User-adjustable grain size (25-200ms) and overlap (2x or 4x)
- **CUMULATIVE:** Each feedback repeat applies another pitch shift
- Creates true unidirectional doppler (pitch rises/falls consistently)

**Tape Delay Feedback Loop:**
- Contains granular doppler → saturation → filters (in order)
- Cumulative effects: pitch shift + saturation both compound with each repeat
- Hi-cut + lo-cut filters simulate tape frequency response

**Saturation Placement:**
- INSIDE feedback loop, AFTER granular doppler
- Creates cumulative tape warmth (each repeat saturates more)
- Saturates the pitch-shifted signal

### Complexity Notes
- Complexity score: 5.0 (Maximum complexity)
- Granular pitch shifting is most complex component (70% of project risk)
- Cumulative feedback behavior requires careful stability testing
- Estimated CPU: 37-57% single core at 48kHz (depends on grain settings)
- Latency: ~10-20ms (granular grain size, reported for host compensation)

### Design Decisions

1. **Separated stereo width and doppler:** Conceptually distinct effects (spatial vs frequency)
2. **Cumulative doppler in feedback:** Creates true escalating pitch shift (not oscillating wow/flutter)
3. **Granular synthesis:** Lower CPU than phase vocoder, acceptable quality for creative effect
4. **No LFO modulation:** Removed for cleaner, more focused design (doppler provides pitch movement)
5. **Advanced settings:** Grain size and overlap exposed for quality tuning

## Next Steps

- [x] Create finalized architecture document (completed 2026-02-08)
- [x] Update implementation plan for granular doppler (completed 2026-02-08)
- [ ] Create UI mockup matching new parameter set (15 parameters with advanced section)
- [ ] Implement DSP according to finalized architecture (`/implement RedShiftDistortion`)
