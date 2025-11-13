# Scatter - Creative Brief

## Overview

**Type:** Effect
**Core Concept:** Granular reversed delay with beautiful stuttering grains, randomized pitch (quantized to musical scales), and randomized stereo placement
**Status:** 💡 Ideated
**Created:** 2025-11-12

## Vision

Scatter is a granular delay effect that transforms incoming audio into evolving, textural ambience through reversed playback, pitch-shifting grains, and randomized spatial placement. Each grain is a fragment of the delayed signal played backwards, with its pitch quantized to a musical scale and its position randomized across the stereo field. The result is a shimmering, stuttering delay that creates ambient soundscapes while maintaining musical coherence through scale quantization.

The plugin draws inspiration from granular processors like GrainScanner and Portal, ambient effects like Cosmos and Shimmer, and reverse delays like Backmask and H-Delay. It's designed for textural ambience—creating evolving soundscapes and atmospheric beds that transform source material into abstract, musical textures.

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Delay Time | 100ms-2s (synced) | 500ms (1/4 note) | Buffer length for grain capture, tempo-synced to note values |
| Grain Size | 5-500ms | 100ms | Length of individual grains |
| Grain Density | 0-100% | 50% | Overlap amount between grains (0%=sparse, 100%=dense clouds) |
| Pitch Random | 0-100% | 30% | Amount of pitch randomization applied to grains (±7 semitones max) |
| Scale | Chromatic/Major/Minor/etc | Chromatic | Musical scale for quantizing randomized pitches |
| Root Note | C-B | C | Root note for scale quantization |
| Pan Random | 0-100% | 75% | Amount of stereo randomization (0%=mono, 100%=full random spread) |
| Feedback | 0-100% | 30% | Traditional delay feedback amount |
| Mix | 0-100% | 50% | Dry/wet signal blend |

## UI Concept

**Layout:** Single-page interface with logical parameter grouping
**Visual Style:** Particle-based visualization showing grains scattering across time and space
**Key Elements:**
- Central grain cloud visualization (shows grain position in time/stereo/pitch space)
- Tempo sync indicator for delay time
- Scale/root note selector (dropdown or keyboard visualization)
- Visual feedback for grain density and randomization amounts

## Use Cases

- Creating evolving soundscapes and atmospheric beds from melodic loops or pads
- Transforming percussion into stuttering, pitched textures
- Adding ambient depth to vocals by creating quantized harmonic delays
- Building textural layers that evolve over time through feedback and randomization
- Sound design for film/games requiring abstract, musical atmospheres

## Inspirations

- **Granular:** GrainScanner, Portal, Granite, Iris (grain processing techniques)
- **Ambient:** Cosmos, Shimmer, CloudSeed (textural delay effects)
- **Reverse:** Backmask, H-Delay reverse mode, EchoBoy (reverse delay algorithms)

## Technical Notes

**DSP Considerations:**
- Circular delay buffer with tempo-sync support (linked to DAW tempo)
- Grain scheduler with overlap-based density control
- Per-grain reverse playback with windowing (Hann/Hamming to avoid clicks)
- Pitch-shifting algorithm for grains (time-domain stretching or frequency-domain shifting)
- Scale quantization system (±7 semitone range mapped to selected scale/root)
- Per-grain random pan position generator
- Feedback loop with traditional delay feedback architecture
- Each grain plays with random forward/reverse selection

**Implementation Strategy:**
- Grain engine with voice management (polyphonic grain playback)
- Windowing to prevent clicks on grain start/end
- Efficient pitch-shifting (spectral or time-domain)
- Real-time tempo sync (recalculate delay time on tempo changes)

## Next Steps

- [ ] Create UI mockup (`/dream Scatter` → option 3)
- [ ] Start implementation (`/implement Scatter`)
