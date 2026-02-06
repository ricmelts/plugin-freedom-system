# RedShiftDistortion - Creative Brief

## Overview

**Type:** Audio Effect
**Core Concept:** Parallel delay and tube distortion processor with comprehensive doppler shift effects, creating the sonic equivalent of astronomical red shift
**Status:** 💡 Ideated
**Created:** 2026-02-05

## Vision

RedShiftDistortion is a hybrid parallel processor that combines the warmth of tube saturation with sophisticated delay and doppler effects. Inspired by the astronomical phenomenon of red shift—where light from receding objects shifts toward the red spectrum—this plugin translates that concept into the audio domain through pitch bending, time stretching, and delay time modulation.

The parallel architecture allows the tube distortion and delay paths to coexist independently, creating rich, evolving textures. The bidirectional doppler effect (-50% to +50%) enables both "red shift" (receding, downward pitch) and "blue shift" (approaching, upward pitch) characteristics, giving users unprecedented creative control over space and motion in their sound.

The holographic, 3D UI with reactive VU metering provides tactile visual feedback that matches the dimensional quality of the sound.

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Saturation | -12dB to +24dB | 0dB | Tube-style drive amount for harmonic saturation |
| Doppler Shift | -50% to +50% | 0% | Bidirectional pitch/time shift intensity (negative = red shift, positive = blue shift) |
| Pitch Enable | On/Off | On | Toggle pitch shifting on/off (keeps time stretch when off) |
| Delay Time | 1/16 to 8 bars (tempo-sync) or 0-16000ms (free) | 1/4 | Delay time with tempo sync or free time mode |
| Tempo Sync | On/Off | On | Lock delay to tempo or use free time |
| Delay Level | 0-100% or -∞ to 0dB | 50% | Volume of delay path in parallel mix |
| Distortion Level | 0-100% or -∞ to 0dB | 50% | Volume of distortion path in parallel mix |
| Master Output | -∞ to +12dB | 0dB | Final output level after parallel mixing |

## UI Concept

**Layout:** Central focus on saturation and doppler shift controls, with 3-stage mixer section below. VU meter prominently displayed with holographic aesthetic.

**Visual Style:**
- Holographic/3D elements with depth and dimensionality
- Reactive VU meter that responds to both distortion and delay signals
- Tactile knob feel with physical feedback visualization
- Color scheme suggests space/astronomy (deep blues, purples, reds shifting based on doppler direction)

**Key Elements:**
- Large central saturation knob
- Prominent doppler shift control with bidirectional indication
- VU meter with holographic glow
- 3-level mixer section (Delay/Distortion/Master)
- Pitch toggle switch
- Tempo sync toggle
- Delay time control with visual sync indication

## Use Cases

- **Vocal processing:** Add dimension and movement to lead vocals with parallel saturation and shifting delays
- **Synth textures:** Create evolving, spatial pads and leads with bidirectional doppler effects
- **Guitar effects:** Combine tube warmth with experimental delay textures for ambient and experimental guitar
- **FX design:** Generate sci-fi sounds, motion effects, and dimensional transitions using extreme doppler settings
- **Mixing depth:** Use subtle settings to place elements in 3D space (red shift = pushing back, blue shift = pulling forward)

## Inspirations

- Astronomical red shift phenomenon (light from receding objects)
- Doppler effect in audio (pitch changes from moving sound sources)
- Classic tape delay with wow/flutter characteristics
- Tube amplifier saturation and harmonic enhancement
- Modern granular/time-stretching processors

## Technical Notes

### DSP Architecture
- **Parallel processing:** Distortion and delay paths operate independently before final mixing
- **Doppler implementation:** Triple-threat approach
  1. Delay time modulation (creates natural pitch artifacts like tape machines)
  2. Independent pitch shifting (clean pitch adjustment)
  3. Time stretching (speed changes without pitch when pitch toggle off)
- **Tube saturation:** Warm harmonic distortion modeling with adjustable drive
- **Tempo sync:** BPM-aware delay timing with musical subdivisions (1/16 to 8 bars)
- **Free time mode:** Millisecond-based delay (0-16000ms) when tempo sync disabled

### Complexity Notes
- Pitch shifting + time stretching may require sophisticated DSP (granular synthesis, phase vocoder, or similar)
- Real-time tempo sync requires host BPM detection
- VU meter needs dual-path metering (separate or combined distortion/delay levels)

## Next Steps

- [ ] Create UI mockup (`/dream RedShiftDistortion` → option 3)
- [ ] Start implementation (`/implement RedShiftDistortion`)
- [ ] Research pitch shifting/time stretching algorithms for doppler effect
