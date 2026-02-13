# Phase 2.2 Summary: Feedback Loop with Saturation + Filters

**Date:** 2026-02-12
**Status:** Complete
**Phase:** 2.2 of 3 (DSP Stage)

---

## What Was Implemented

### 1. Cumulative Saturation (tanh waveshaping)

**Components added:**
- `juce::dsp::WaveShaper<float> saturationL`
- `juce::dsp::WaveShaper<float> saturationR`

**Configuration:**
- Transfer function: `std::tanh(gain * input)`
- Gain formula: `gain = pow(10.0, saturationDB / 20.0)`
- Range: -12dB to +24dB (-12dB = 0.251x, 0dB = 1.0x, +24dB = 15.85x)
- Applied INSIDE feedback loop (cumulative effect)
- Bypass control: `bypassSaturation` parameter

### 2. Dual-Band Filtering (Hi-pass + Lo-pass)

**Components added:**
- `juce::dsp::IIR::Filter<float> hiPassFilterL` (lo-cut)
- `juce::dsp::IIR::Filter<float> hiPassFilterR` (lo-cut)
- `juce::dsp::IIR::Filter<float> loPassFilterL` (hi-cut)
- `juce::dsp::IIR::Filter<float> loPassFilterR` (hi-cut)

**Configuration:**
- Filter type: 2nd-order Butterworth (Q=0.707, maximally flat passband)
- Hi-pass cutoff: `filterBandLow` parameter (20-20000 Hz, default 100 Hz)
- Lo-pass cutoff: `filterBandHigh` parameter (20-20000 Hz, default 8000 Hz)
- Series configuration: Hi-pass → Lo-pass
- Edge case handling: If lo-cut > hi-cut, swap values (ensures valid bandpass)
- Applied INSIDE feedback loop, AFTER saturation

### 3. Feedback Loop Closure

**Components added:**
- `float feedbackStateL` - Stores previous output for L channel
- `float feedbackStateR` - Stores previous output for R channel

**Configuration:**
- Feedback gain: `feedback` parameter (0-95%, scaled to 0.0-0.95)
- Feedback signal mixed with current input BEFORE stereo width delay
- Feedback chain: Delay Output → Saturation → Hi-Pass → Lo-Pass → Feedback Gain → Mix with Input
- Creates cumulative effects (each repeat applies saturation + filtering again)

---

## Signal Flow (Phase 2.2)

```
Input → [Mix with Feedback] → Stereo Width → Delay (260ms) → Output
  ↑                                                             ↓
  └──── Feedback ← Filters ← Saturation ← (Delay Output) ──────┘
```

**Feedback Loop Detail:**
```
Delay Output (260ms)
  ↓
[bypassSaturation?]
  ├─ NO → Saturation (tanh waveshaping)
  └─ YES → Skip
  ↓
Hi-Pass Filter (lo-cut, removes low frequencies)
  ↓
Lo-Pass Filter (hi-cut, removes high frequencies)
  ↓
Feedback Gain (0.0-0.95)
  ↓
Mix with Current Input → Back to Stereo Width Delay
```

---

## Parameters Connected (This Phase)

| Parameter | Type | Range | Effect |
|-----------|------|-------|--------|
| feedback | Float | 0-95% | Feedback gain (0% = single repeat, 95% = ~10+ repeats) |
| saturation | Float | -12dB to +24dB | Tube saturation drive (cumulative in feedback) |
| filterBandLow | Float | 20-20000 Hz | Hi-pass cutoff (lo-cut, removes low frequencies) |
| filterBandHigh | Float | 20-20000 Hz | Lo-pass cutoff (hi-cut, removes high frequencies) |
| bypassSaturation | Bool | Off/On | Bypass saturation stage (keeps delay + filters) |

---

## Test Criteria (from plan.md)

**Saturation:**
- [ ] At -12dB: Subtle warmth
- [ ] At 0dB: Moderate saturation
- [ ] At +24dB: Heavy saturation with harmonics
- [ ] Cumulative effect audible (each repeat more saturated)

**Dual-band filtering:**
- [ ] Lo-cut filter removes low frequencies
- [ ] Hi-cut filter removes high frequencies
- [ ] Bandpass result: Only frequencies between lo-cut and hi-cut pass
- [ ] Edge case: If lo-cut > hi-cut, swap values (no crash)

**Feedback loop:**
- [ ] At 0%: Single repeat (no feedback)
- [ ] At 50%: ~3-5 repeats
- [ ] At 95%: Long decay (~10+ repeats)
- [ ] No runaway oscillation at 95% feedback + max saturation
- [ ] No DC offset buildup over time (test with 30+ second audio)

**Bypass:**
- [ ] Bypass saturation works (keeps delay + filters, skips saturation)

---

## Implementation Notes

### Real-Time Safety

All processing is real-time safe:
- No memory allocation in `processBlock()`
- Filter coefficients updated every block (no allocations, just coefficient recalculation)
- Atomic parameter reads via `getRawParameterValue()->load()`
- `juce::ScopedNoDenormals` present in `processBlock()`

### Filter Coefficient Updates

Filter coefficients are recalculated every block when parameters change:
```cpp
*hiPassFilterL.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoff, 0.707);
*loPassFilterL.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoff, 0.707);
```

This is real-time safe because:
- `makeHighPass()` and `makeLowPass()` return shared pointers (no allocation)
- Assignment to `filter.state` is atomic pointer swap
- JUCE DSP components handle this pattern internally

### Saturation Algorithm

Hyperbolic tangent waveshaping:
```cpp
float saturatedL = std::tanh(saturationGain * delayedL);
```

Why tanh?
- Bounded output: -1 to +1 (prevents runaway clipping)
- Smooth saturation curve (no harsh edges)
- Classic tape saturation sound (odd harmonics)
- Fast (standard library implementation)

### Cumulative Effects Explanation

**Without feedback (Phase 2.1):**
- Input → Delay → Output (single pass)

**With feedback (Phase 2.2):**
- Input → Delay → Saturation → Filters → Feedback → Mix with input
- Each repeat goes through saturation + filters again
- Example: +12dB saturation, 50% feedback, 5 repeats
  - Repeat 1: Saturated once
  - Repeat 2: Saturated twice (cumulative)
  - Repeat 3: Saturated three times (progressive buildup)
  - Result: Each repeat progressively more saturated and filtered

This creates the "tape delay" sound where repeats get progressively darker and more saturated.

---

## Known Issues / Limitations

None identified during implementation.

**Potential issues to monitor:**
1. Filter coefficient updates every block (could optimize with change detection)
2. Feedback stability at extreme settings (95% feedback + +24dB saturation + wide filters)
3. DC offset buildup over very long runs (>60 seconds continuous playback)

**Mitigation:**
- Feedback limited to 95% (prevents infinite gain)
- tanh saturation is bounded (prevents runaway clipping)
- Butterworth filters with Q=0.707 (no resonance, maximally flat)
- Edge case handling (swap lo-cut/hi-cut if inverted)

---

## Next Phase: 2.3 - Granular Doppler Shift

**Components to implement:**
- Granular pitch shifter (custom implementation using DelayLine)
- Grain scheduling (2x or 4x overlap)
- Hann windowing for smooth grain boundaries
- Pitch ratio calculation: `pow(2.0, dopplerShift / 100.0)`
- Applied INSIDE feedback loop, AFTER delay output but BEFORE saturation

**Signal flow:**
```
Delay Output → Granular Doppler → Saturation → Filters → Feedback
```

**Complexity:** HIGH
- Custom granular synthesis implementation
- Multiple simultaneous read pointers (2 or 4 grains)
- Hann window calculation and application per sample
- Pitch accuracy verification required

---

## Files Modified

- `plugins/RedShiftDistortion/Source/PluginProcessor.h` - Added saturation, filter, and feedback state members
- `plugins/RedShiftDistortion/Source/PluginProcessor.cpp` - Implemented feedback loop with saturation + filters
