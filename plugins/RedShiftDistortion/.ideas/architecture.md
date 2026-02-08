# DSP Architecture: RedShiftDistortion

**CRITICAL CONTRACT:** This specification is immutable during Stages 1-4 implementation. Stage 3 (DSP) implements this exact architecture.

**Last Updated:** 2026-02-08 (Redesigned from parallel to series)
**Purpose:** DSP specification defining processing components, signal flow, and JUCE module usage

---

## Core Processing Chain

**SERIES ARCHITECTURE - Signal flows through stages in order:**

```
Input (Stereo)
  ↓
Delay Line (tempo-synced or free time)
  ← delayTime parameter
  ← tempoSync parameter
  ↓
(TODO: Doppler Pitch/Time Stretch)
  ← dopplerShift parameter
  ← pitchEnable parameter
  ↓
Tube Saturation (tanh waveshaping)
  ← saturation parameter
  ↓
Master Output Gain
  ← masterOutput parameter
  ↓
Output (Stereo)
```

**Key Principle:** ONE signal path. Each stage processes the output of the previous stage.

---

## Core Components

### 1. Delay Line

- **JUCE Class:** `juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>`
- **Purpose:** Provide tempo-synced or free-time delay
- **Parameters:** delayTime (0-2000ms), tempoSync (bool)
- **Implementation:**
  - Tempo-synced mode: Map delayTime value to musical divisions (1/16, 1/8, 1/4, 1/2, 1 bar)
  - Free time mode: Use delayTime directly in milliseconds
  - Max buffer: 2 seconds at 192kHz
  - Linear interpolation (simple, low CPU)

**Tempo Sync Implementation:**
```cpp
// Get host BPM (default 120 if unavailable)
auto bpm = getPlayHead()->getPosition()->getBpm().value_or(120.0);

// Convert to quarter note duration
double quarterNoteMs = (60000.0 / bpm);

// Map parameter ranges to divisions:
// 0-250ms → 1/16 note (0.25 beats)
// 250-500ms → 1/8 note (0.5 beats)
// 500-1000ms → 1/4 note (1.0 beats)
// 1000-1500ms → 1/2 note (2.0 beats)
// 1500-2000ms → 1 bar (4.0 beats)
```

### 2. Doppler Pitch/Time Stretch (TODO - Placeholder)

- **Purpose:** Create doppler effect (pitch shifting or time stretching)
- **Parameters:** dopplerShift (-50% to +50%), pitchEnable (bool)
- **Status:** NOT YET IMPLEMENTED
- **Planned Approach:** Granular synthesis or simple pitch-preserving time stretch
- **Current Behavior:** Pass-through (no processing)

**When implemented:**
- If pitchEnable = true: Shift pitch by ±12 semitones based on dopplerShift
- If pitchEnable = false: Time-stretch without pitch change
- Implementation complexity: HIGH (defer to future iteration)

### 3. Tube Saturation

- **Implementation:** Custom `std::tanh()` waveshaping
- **Purpose:** Add warm harmonic distortion
- **Parameter:** saturation (-12dB to +24dB)
- **Processing:**
  ```cpp
  float gain = std::pow(10.0f, saturationDb / 20.0f);
  output = std::tanh(gain * input);
  ```
- **Characteristics:**
  - Generates 2nd and 3rd order harmonics (musical warmth)
  - Soft-clipping (smooth saturation curve)
  - No oversampling needed for moderate settings

### 4. Master Output

- **Implementation:** Simple gain multiplication
- **Parameter:** masterOutput (-60dB to +12dB)
- **Processing:**
  ```cpp
  float gain = std::pow(10.0f, masterOutputDb / 20.0f);
  output *= gain;
  ```

---

## Parameter Specification

| Parameter | Type | Range | Default | Usage |
|-----------|------|-------|---------|-------|
| saturation | Float | -12.0 to 24.0 dB | 0.0 dB | Tube saturation drive |
| dopplerShift | Float | -50.0 to 50.0 % | 0.0 % | Doppler intensity (TODO) |
| pitchEnable | Bool | On/Off | true | Pitch vs time-stretch mode (TODO) |
| delayTime | Float | 0.0 to 2000.0 ms | 250.0 ms | Delay time |
| tempoSync | Bool | On/Off | false | Tempo-sync vs free time |
| masterOutput | Float | -60.0 to 12.0 dB | 0.0 dB | Final output level |

**Total Parameters:** 6

**Removed from parallel architecture:**
- ❌ delayLevel (no separate delay path)
- ❌ distortionLevel (no separate distortion path)
- ❌ feedback, hiCut, loCut, reverseDelay (scope creep, not in spec)

---

## Processing Flow (Pseudocode)

```cpp
void processBlock(AudioBuffer& buffer, MidiBuffer& midi)
{
    // Read parameters
    float delayTimeMs = getTempoSyncedDelayTime(delayTimeParam, tempoSyncEnabled);
    float delayTimeSamples = (delayTimeMs / 1000.0f) * sampleRate;
    mainDelayLine.setDelay(delayTimeSamples);

    float saturationGain = pow(10.0f, saturationDb / 20.0f);
    float masterGain = pow(10.0f, masterOutputDb / 20.0f);

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // STAGE 1: Delay
        float leftIn = buffer.getSample(0, sample);
        float rightIn = buffer.getSample(1, sample);

        mainDelayLine.pushSample(0, leftIn);
        mainDelayLine.pushSample(1, rightIn);

        float leftOut = mainDelayLine.popSample(0);
        float rightOut = mainDelayLine.popSample(1);

        // STAGE 2: Doppler (TODO - currently pass-through)
        // if (dopplerShift != 0.0f) { ... }

        // STAGE 3: Saturation
        leftOut = tanh(saturationGain * leftOut);
        rightOut = tanh(saturationGain * rightOut);

        // STAGE 4: Master Output
        buffer.setSample(0, sample, leftOut * masterGain);
        buffer.setSample(1, sample, rightOut * masterGain);
    }
}
```

---

## State Persistence

**Saved Parameters (APVTS automatic):**
- saturation
- dopplerShift
- pitchEnable
- delayTime
- tempoSync
- masterOutput

**Methods:**
- `getStateInformation()` - Serialize APVTS to XML
- `setStateInformation()` - Deserialize APVTS from XML

No custom state needed - all state is parameter-based.

---

## Implementation Notes

### Thread Safety
- All parameter reads use `getRawParameterValue()->load()` (atomic)
- Delay line prepared in `prepareToPlay()` (no allocations in audio thread)
- Host tempo query via `getPlayHead()` is real-time safe

### Performance
- **Delay line:** ~3-5% CPU (linear interpolation)
- **Saturation:** ~2% CPU (per-sample tanh)
- **Doppler (when implemented):** ~20-30% CPU estimate
- **Total (without doppler):** ~5-7% CPU - very lightweight

### Denormal Protection
- Use `juce::ScopedNoDenormals` in `processBlock()`
- JUCE DelayLine handles denormals internally

### Sample Rate Handling
- Delay line sized for 2 seconds at max sample rate
- Tempo sync milliseconds calculated per-buffer (BPM can change)
- All processing reinit in `prepareToPlay()` on sample rate change

---

## Architecture Decisions

### Why Series (Not Parallel)?

**Decision:** Single signal path - Delay → (Doppler) → Saturation → Output

**Rationale:**
- **Simpler implementation:** No buffer management for parallel paths
- **Lower CPU:** Single path vs dual-path processing
- **Clearer sonic identity:** Saturated delay (not delay + saturation blend)
- **Easier to understand:** Parameters affect one signal, not mix of two

**Trade-off:** Less mixing flexibility than parallel, but cleaner design

### Why Defer Doppler Implementation?

**Decision:** Leave doppler as TODO placeholder

**Rationale:**
- Granular synthesis is complex (~70% of original project risk)
- Current series architecture works without it
- Can be added later without changing signal flow
- Allows testing core delay + saturation first

**When to implement:**
- After core functionality stable
- If user requests doppler-specific effects
- Consider simpler pitch-shifting alternatives first

---

## Component Requirements

### JUCE Modules Needed:
- `juce_audio_processors` - AudioProcessor base class, APVTS
- `juce_dsp` - DelayLine, ProcessSpec

### DSP Classes Used:
- `juce::dsp::DelayLine<float, Linear>` - Main delay
- `juce::AudioProcessorValueTreeState` - Parameter management
- `std::tanh()` - Saturation waveshaping

### No External Dependencies
- Pure JUCE implementation
- No third-party DSP libraries

---

## Testing Strategy

1. **Delay Test:** Verify tempo sync and free time modes work correctly
2. **Saturation Test:** Check harmonic generation at various drive levels
3. **Master Output Test:** Verify gain scaling (especially at extremes)
4. **Parameter Recall Test:** Save/load preset, verify all values restored

---

## Future Enhancements (Out of Scope)

- ⏳ Granular doppler pitch shifting
- ⏳ Feedback loop with filtering
- ⏳ LFO modulation of delay time
- ⏳ Stereo width control

Current scope: **Delay → Saturation → Output** (simple, stable, usable)

---

**END OF SPECIFICATION**
