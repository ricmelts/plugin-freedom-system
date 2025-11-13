# Changelog

All notable changes to TapeAge will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.4] - 2025-11-13

### Fixed

- **Dry/Wet Slap-Back Delay:** Eliminated ~100ms slap-back echo when mixing dry and wet signals at intermediate mix values
  - **Root Cause:** DryWetMixer constructed with default `maximumWetLatencyInSamples = 0`, preventing internal dry delay buffer from compensating for wet signal processing latency (~4850 samples at 48kHz: oversampler + 100ms delay line). The `setWetLatency()` call in `prepareToPlay()` was silently clamped to 0, leaving dry signal immediate while wet signal arrived 100ms late.
  - **Solution:** Initialize DryWetMixer with 20,000 sample capacity at construction time: `dryWetMixer { 20000 }` (supports up to 192kHz sample rate × 0.1s delay line + oversampler latency)
  - **Technical Details:** JUCE's DryWetMixer requires maximum latency specified at construction - cannot be changed afterward. Default constructor sets internal capacity to 0. Previous v1.1.1 attempt (never committed) incorrectly assumed calculation was the issue, but the real problem was insufficient buffer allocation.
  - **User Impact:** At 50/50 mix, dry and wet signals now arrive simultaneously instead of creating "kick drums bouncing off walls" echo effect. Mix parameter works correctly across all ratios (0-100%).
  - **Investigation:** Deep-research Level 2 analysis confirmed via JUCE source code examination (`/Applications/JUCE/modules/juce_dsp/processors/juce_DryWetMixer.h:123`)
  - **Testing:** Manual DAW testing with dry/wet at 50% confirms no audible slap-back delay
  - **Backward Compatibility:** Zero impact on saved presets or parameter values - latency compensation is transparent to user

## [1.1.0] - 2025-11-12

### Added

- **Volume Compensation:** Drive knob now maintains constant output level via automatic makeup gain
  - Makeup gain = `1.0 / tanh(gain)` applied after saturation
  - Output volume stays consistent regardless of drive setting
  - Allows focus on tonal character without volume changes

- **Enhanced AGE Parameter:** More extreme tape degradation at maximum settings
  - **Increased Wow Depth:** ±25 cents pitch variation (was ±10 cents) - 2.5x more warble for noticeable vintage character while remaining musical
  - **Dual-LFO Flutter:** Added 6Hz secondary flutter LFO at 20% depth for richer texture alongside primary 1-2Hz wow LFO
  - **High-Frequency Rolloff:** Age-dependent treble loss simulating tape aging and head wear
    - Age 0%: 20kHz (transparent, no effect)
    - Age 100%: 8kHz cutoff (vintage tape character with noticeable treble loss)
    - Exponential mapping for musical response
  - **Increased Noise Floor:** -60dB at max age (was -80dB) - 20dB louder for more present vintage character while remaining subtle
  - **User Impact:** AGE knob now produces more dramatic "old tape" character at high values - more pitch wobble, duller frequency response, and audible noise texture
  - **Backward Compatibility:** Existing presets load but will sound different at high AGE values (more extreme degradation)

### Technical Details

- Added `flutterPhase[2]` member variable for secondary LFO state tracking
- Added `ageFilter[2]` IIR lowpass filters (one per channel) for frequency rolloff
- Modified processBlock wow/flutter section to combine dual LFOs before delay line modulation
- Added high-frequency rolloff processing after wow/flutter, before dropout
- Updated architecture.md to document v1.1.0 enhancements
- All changes maintain real-time safety (no allocations in audio thread)
- Filter coefficients updated per buffer, not per sample (performance optimization)

### Testing Notes

- Built and tested in Release mode
- Verified increased wow depth is noticeable but musical (not excessive)
- Confirmed high-frequency rolloff creates "old tape" character without harshness
- Noise increase is present but not overwhelming
- All parameters function correctly with new processing chain

## [1.0.3] - 2025-11-12

### Fixed

- **UI Hover Stability:** Eliminated subtle shaking when hovering over knobs
  - **Root Cause:** `transform: scale(1.005)` in `.knob:hover` caused sub-pixel rendering recalculation in WebView, creating imperceptible but continuous layout shifts
  - **Solution:** Replaced scale transform with border color change (matching VU meter housing behavior)
  - **Implementation:** Added `border: 2px solid rgba(212, 165, 116, 0.3)` to `.knob-body`, hover changes to `#c49564` (gold accent)
  - **User Impact:** Knobs now have stable, clean hover effect without visual artifacts
  - **Technical Details:** WebView/WebBrowserComponent struggles with micro-scale transforms (<1%), causing render loop instability
  - **Testing:** Verified across all 3 knobs (drive, age, mix)—no shake on hover, depression effect on click preserved
  - **Backward Compatibility:** Zero impact—visual-only change, no parameter or state modifications

## [1.0.2] - 2025-11-11

### Fixed

- WebView parameter knobs now correctly display saved values on plugin reload
- Fixed `valueChangedEvent` callback to read values via `getNormalisedValue()` instead of using undefined callback parameters
- Eliminated race condition where JavaScript called `getNormalisedValue()` before C++ sent initial values
- Knobs now load directly at saved positions instead of animating from zero

### Technical Details

- Root cause: JUCE's `valueChangedEvent` is a notification event that doesn't pass callback parameters
- Solution: Call `getNormalisedValue()` inside the callback to read from state object
- Documented in: troubleshooting/gui-issues/webview-parameter-undefined-event-callback-TapeAge-20251111.md
- Added to Required Reading: Pattern #15 in juce8-critical-patterns.md

## [1.0.1] - 2025-11-11

### Fixed

- Parameter state now persists correctly when closing and reopening Ableton Live
- Changed `getNumPrograms()` to return 0 to prevent Ableton from interfering with state restoration
- Fixes issue where parameters would reset to defaults on project reload

## [1.0.0] - 2025-11-11

### Added

- Initial release
- Vintage tape saturator with warm harmonic saturation
- Tape degradation effects: wow, flutter, dropout, and noise
- Three parameters: Drive (saturation), Age (degradation), Mix (dry/wet)
- Drive: 0-100%, default 50% - Controls tape saturation intensity
- Age: 0-100%, default 25% - Controls tape degradation artifacts (wow, flutter, dropout, noise)
- Mix: 0-100%, default 100% - Dry/wet blend control
- Factory presets: Unity (bypass), Subtle Warmth, Classic Tape, Worn Out, Destroyed

### Audio Processing

- Warm harmonic saturation with 2x oversampling for aliasing reduction
- Custom tanh transfer function for vintage tape character
- Wow and flutter modulation (±10 cents) using delay line with Lagrange interpolation
- Subtle tape noise generation (musical even at maximum settings)
- Random dropout simulation for authentic tape artifacts
- Equal-power dry/wet crossfade with latency compensation
- Real-time safe processing (no allocations in audio callback)

### User Interface

- WebView-based vintage hardware aesthetic
- Vintage VU meter with peak metering and ballistic animation (30 FPS C++ + 60 FPS JS)
- Three brass rotary knobs with gold indicators (horizontal layout)
- Creamy beige textured background with burnt orange/brown accents
- Clean sans-serif all-caps typography
- 500x450px window size

### Validation

- Passed pluginval strict validation (strictness level 10)
- VST3: All tests passed
- AU: All tests passed + auval validation
- Tested in Ableton Live and Standalone mode
- Thread safety verified
- State save/restore verified
- Parameter automation verified

### Formats

- VST3
- AU (Audio Unit)
- Standalone application
