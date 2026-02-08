# RedShiftDistortion - Implementation Plan

**Date:** 2026-02-05
**Revised:** 2026-02-08 (Final - Granular Doppler + Cumulative Feedback)
**Complexity Score:** 5.0 (Maximum Complexity)
**Strategy:** Phase-based implementation with isolated granular engine testing

---

## Complexity Factors

### Calculation Breakdown

- **Parameters:** 15 parameters (15/5 = 3.0 points, capped at 2.0) = **2.0**
  - stereoWidth, feedback, filterBandLow, filterBandHigh, dopplerShift, saturation, masterOutput, bypassStereoWidth, bypassDelay, bypassDoppler, bypassSaturation, grainSize, grainOverlap

- **Algorithms:** 4 DSP components = **4.0**
  - Stereo Width (differential L/R delays)
  - Tape Delay (feedback loop with filtering)
  - Granular Pitch Shifter (overlap-add synthesis with variable-rate playback) ← HIGHEST COMPLEXITY
  - Tube Saturation (tanh waveshaping)

- **Features:** 2 complexity features = **2.0**
  - Granular synthesis (+1) - Grain buffer management, overlap-add, windowing, pitch-shifted playback
  - Cumulative feedback effects (+1) - Doppler + saturation compound with each repeat

- **Total:** 2.0 + 4.0 + 2.0 = **8.0** → **Capped at 5.0**

**Complexity Tier:** 5 (Maximum - granular pitch shifting with cumulative feedback)

**Complexity rationale:**
- Granular pitch shifting is algorithmically complex (grain management, overlap-add, variable-rate playback)
- Cumulative feedback behavior adds risk (pitch shifts compound exponentially)
- No existing JUCE granular class (custom implementation required)
- Moderate to high CPU usage (~37-57% single core depending on grain settings)

---

## Stages

- ✓ Stage 0: Research & Planning (Complete - Finalized 2026-02-08)
- → Stage 1: Foundation + Shell (Next - Build system + APVTS parameters)
- ⏳ Stage 2: DSP (5 phases - Stereo Width → Granular Engine → Integration → Saturation → Polish)
- ⏳ Stage 3: GUI (Update UI for 15 parameters with advanced settings panel)
- ⏳ Stage 4: Validation (Presets, pluginval, changelog)

---

## Maximum Implementation (Score = 5.0)

### Stage 2: DSP Implementation (5 Phases)

#### Phase 2.1: Stereo Width + Basic Delay (No Feedback)

**Goal:** Establish series signal flow with stereo width modulation and basic delay

**Components:**
- Stereo width modulation (differential L/R delay times with exponential smoothing)
- Basic delay line (no feedback yet, just pass-through)
- Master output gain
- Bypass controls (bypassStereoWidth, bypassDelay placeholders)

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes
- [ ] Audio passes through series chain (stereo width → delay → output)
- [ ] stereoWidth parameter creates spatial positioning (±260ms L/R differential)
  - Negative values = narrow stereo
  - Zero = mono
  - Positive values = wide stereo
- [ ] bypassStereoWidth bypasses stereo width modulation (mono output)
- [ ] bypassDelay bypasses delay line entirely
- [ ] masterOutput scales final output level (-60dB to +12dB)
- [ ] No clicks, pops, or artifacts at extreme parameter values
- [ ] Both stereo channels process correctly
- [ ] Exponential smoothing prevents clicks on stereoWidth changes

**Duration:** 1 day

---

#### Phase 2.2: Granular Pitch Shifter (Isolated Testing)

**Goal:** Implement granular synthesis engine in isolation before feedback integration

**Components:**
- Grain buffer management (circular buffer, pre-allocated)
- Hann window generation (pre-calculated lookup table)
- Grain spawning and overlap tracking (4-grain or 2-grain overlap)
- Variable-rate grain playback (pitch-shifted resampling)
- Overlap-add synthesis (sum of windowed grains)
- Advanced parameters (grainSize, grainOverlap)

**Test Criteria:**
- [ ] Granular engine compiles without errors
- [ ] dopplerShift parameter shifts pitch accurately (±12 semitones / ±1 octave)
  - -50% doppler → pitch down one octave (0.5x pitch ratio)
  - 0% doppler → no pitch shift (1.0x pitch ratio)
  - +50% doppler → pitch up one octave (2.0x pitch ratio)
- [ ] Pitch shift accuracy verified with tone generator (440 Hz → 880 Hz at +50% doppler)
- [ ] Grain boundaries smooth (no clicks or pops)
- [ ] grainSize parameter adjusts grain buffer size (25-200ms)
- [ ] grainOverlap parameter switches between 2x and 4x overlap
- [ ] 4x overlap smoother than 2x overlap (subjective quality test)
- [ ] 2x overlap uses ~50% CPU of 4x overlap
- [ ] No artifacts at moderate grain settings (100ms, 4x overlap)
- [ ] Acceptable artifacts at extreme settings (25ms grain size may have slight graininess)
- [ ] CPU usage within budget (~20-40% single core depending on settings)
- [ ] No memory leaks or buffer overflows
- [ ] Stereo processing works correctly (independent L/R grain tracking)

**Duration:** 3-5 days ⚠️ HIGHEST RISK

**Risk Mitigation:**
- Implement grain engine in separate test harness first
- Unit test pitch accuracy with known frequencies
- Profile CPU usage early (optimize if >50% single core)
- Reference Curtis Roads granular synthesis literature
- Start with 2-grain overlap (simpler), scale to 4-grain if CPU allows
- Make grain settings user-adjustable (allows post-release tuning)

---

#### Phase 2.3: Integrate Granular into Feedback Loop

**Goal:** Add granular pitch shifter to feedback path and test cumulative behavior

**Components:**
- Feedback loop structure (delayed signal fed back to input)
- Granular pitch shifter in feedback path
- Feedback gain control (0-95%)
- bypassDoppler control

**Test Criteria:**
- [ ] Feedback loop creates repeating delays
- [ ] Granular pitch shifter processes each feedback repeat
- [ ] **CUMULATIVE behavior verified:**
  - Repeat 1: +1 octave (at +50% doppler)
  - Repeat 2: +2 octaves
  - Repeat 3: +3 octaves
  - Eventually shifts out of audible range (natural decay)
- [ ] Negative doppler creates descending pitch shifts (red shift)
- [ ] Positive doppler creates ascending pitch shifts (blue shift)
- [ ] bypassDoppler bypasses pitch shifting (keeps delay, skips granular processing)
- [ ] Feedback loop is stable (no runaway oscillation at 95% feedback)
- [ ] High feedback + high doppler creates rapid pitch escalation (expected behavior)
- [ ] CPU usage acceptable with feedback (~30-50% single core)

**Duration:** 1 day

**Risk Mitigation:**
- Test cumulative behavior with tone generator (measure pitch of each repeat)
- Document expected cumulative escalation (not a bug, it's a feature)
- Test stability at extreme settings (feedback=95%, doppler=+50%)

---

#### Phase 2.4: Saturation + Filters in Feedback

**Goal:** Add saturation and dual-band filtering to feedback path

**Components:**
- Tube saturation (tanh waveshaping AFTER granular pitch shifter)
- Hi-cut filter (lowpass) in feedback path
- Lo-cut filter (highpass) in feedback path
- bypassSaturation control

**Test Criteria:**
- [ ] saturation parameter affects pitch-shifted delayed signal (cumulative warmth)
- [ ] Saturation builds with each repeat (cumulative behavior)
- [ ] filterBandHigh (hi-cut lowpass) removes high frequencies from feedback
- [ ] filterBandLow (lo-cut highpass) removes low frequencies from feedback
- [ ] Filters create authentic tape echo frequency response (rolloff with repeats)
- [ ] bypassSaturation bypasses saturation stage (linear passthrough)
- [ ] High feedback + high saturation + high doppler doesn't cause harsh artifacts
- [ ] No DC offset from saturation (verify with spectrum analyzer)
- [ ] Cumulative saturation + pitch shift creates expected tonal character
- [ ] CPU usage within budget (~37-57% single core total)

**Duration:** 1 day

**Risk Mitigation:**
- Test with extreme settings (feedback=95%, saturation=+24dB, doppler=+50%)
- Verify no runaway oscillation or DC offset
- Add soft clipper after saturation if needed (safety limiter)
- Reference RE-201 Space Echo for expected cumulative saturation behavior

---

#### Phase 2.5: Polish and Optimization

**Goal:** Tune grain parameters, optimize CPU, reduce artifacts

**Components:**
- Grain parameter tuning (find optimal defaults for quality/CPU balance)
- CPU profiling and optimization
- Artifact reduction (tune grain size and overlap)
- Bypass optimization (skip unnecessary processing when bypassed)

**Test Criteria:**
- [ ] Default grain settings (100ms, 4x overlap) sound smooth
- [ ] 2x overlap mode provides acceptable quality with lower CPU
- [ ] CPU usage profiled and optimized (<50% single core at 48kHz)
- [ ] Bypasses skip unnecessary processing (CPU savings when bypassed)
- [ ] Artifacts minimized at default settings
- [ ] Extreme grain settings documented (25ms may have graininess, expected)
- [ ] No memory leaks after extended use
- [ ] Plugin stable with rapid parameter automation

**Duration:** 0.5-1 day

---

### Stage 3: GUI Implementation

#### GUI Update: 15 Parameters with Advanced Settings Panel

**Goal:** Update WebView UI to match new 15-parameter set with collapsible advanced panel

**Components:**
- Main controls section (stereoWidth, feedback, filters, dopplerShift, saturation, masterOutput)
- Bypass controls section (bypassStereoWidth, bypassDelay, bypassDoppler, bypassSaturation)
- Advanced settings panel (grainSize, grainOverlap) - collapsible
- Update PluginEditor.h/cpp bindings for 15 parameters
- Update index.html for new parameter set
- Verify WebSliderRelay and WebToggleButtonRelay for all parameters
- Value formatting (dB for saturation/masterOutput, % for stereoWidth/dopplerShift/feedback, Hz for filters, ms for grainSize)

**Test Criteria:**
- [ ] WebView window opens with correct size
- [ ] All 15 parameter controls render correctly
- [ ] Main controls section displays 7 main parameters
- [ ] Bypass controls section displays 4 bypass toggles
- [ ] Advanced settings panel displays 2 grain quality controls
- [ ] Advanced panel can be collapsed/expanded
- [ ] Knob drag changes DSP parameters (audible effect)
- [ ] Host automation updates UI knobs (visual feedback)
- [ ] Preset changes update all UI elements simultaneously
- [ ] Parameter values display correctly (formatted strings with units)
- [ ] Toggle switches reflect state correctly (bypasses)
- [ ] grainOverlap displays as "2x" or "4x" (not 2.0 or 4.0)
- [ ] No frozen knobs or binding issues
- [ ] No references to deleted parameters (lfoRate, lfoDepth, lfoTempoSync)

**Duration:** 1 day

**Critical Patterns:**
- Use `getSliderState()` for continuous parameters (stereoWidth, feedback, filters, dopplerShift, saturation, grainSize, masterOutput)
- Use `getToggleState()` for boolean parameters (bypassStereoWidth, bypassDelay, bypassDoppler, bypassSaturation)
- grainOverlap is choice parameter (2x or 4x, may need custom handling)
- WebSliderParameterAttachment requires 3 params in JUCE 8: (parameter, relay, nullptr)
- Verify all deleted parameters removed from PluginEditor constructor

---

### Implementation Flow Summary

```
✓ Stage 0: Research & Planning (Complete - Finalized 2026-02-08)
  - architecture.md finalized (granular doppler + cumulative feedback)
  - creative-brief.md finalized (separated stereo width + doppler)
  - plan.md finalized (5-phase implementation)

→ Stage 1: Foundation + Shell
  - Update parameter definitions (15 parameters)
  - Build system (CMakeLists.txt, JUCE configuration)
  - APVTS parameter layout
  - Placeholder PluginProcessor/PluginEditor

⏳ Stage 2: DSP (5 phases, ~6-9 days)
  - Phase 2.1: Stereo Width + Basic Delay (1 day)
  - Phase 2.2: Granular Pitch Shifter (3-5 days) ← HIGHEST RISK
  - Phase 2.3: Integrate Granular into Feedback (1 day)
  - Phase 2.4: Saturation + Filters (1 day)
  - Phase 2.5: Polish and Optimization (0.5-1 day)

⏳ Stage 3: GUI (1 day)
  - Update UI for 15 parameters
  - Add advanced settings panel (collapsible)
  - Update parameter bindings

⏳ Stage 4: Validation (~1 day)
  - Preset creation (10 presets showcasing cumulative doppler + stereo width)
  - pluginval testing (VST3, AU, Standalone)
  - Changelog generation
  - Build verification
```

---

## Implementation Notes

### Thread Safety

- All parameter reads use atomic `APVTS::getRawParameterValue()->load()` (real-time safe)
- Delay line buffers pre-allocated in prepareToPlay (no allocations in audio thread)
- Granular grain buffers pre-allocated in prepareToPlay (no reallocations)
- Feedback buffer sized for maximum delay (300ms at 192kHz)
- No shared state between channels (per-channel grain tracking)
- Hann window pre-calculated in prepareToPlay (lookup table, no runtime calculation)

### Performance

**Estimated CPU usage per component:**
- Stereo width: ~5% single core (two delay lines with exponential smoothing)
- Tape delay: ~5% (delay line read/write + feedback mixing)
- Granular pitch shifter: ~20-40% (depends on grainSize and grainOverlap)
  - 4-grain overlap, 100ms grain: ~30% CPU
  - 2-grain overlap, 100ms grain: ~15% CPU
  - Larger grain sizes reduce CPU per grain
- Saturation: ~3% (per-sample tanh in feedback loop)
- Feedback filters: ~4% (dual IIR filters)
- **Total estimated: ~37-57% single core @ 48kHz** (depends on grain settings)

**Optimization opportunities:**
- Use 2x grain overlap instead of 4x (saves ~15% CPU)
- Larger grain size (150-200ms) reduces CPU per grain (trade latency for CPU)
- Bypass granular when dopplerShift = 0% (skip unnecessary processing)
- Update grain spawning less frequently (every N samples instead of every sample)

### Latency

- Stereo width: ~0-260ms (user-controlled, NOT counted as latency)
- Tape delay: 0-300ms user-controlled (NOT counted as latency)
- Granular pitch shifter: ~10-20ms inherent latency (grain size dependent)
- Saturation: 0ms (instantaneous waveshaping)
- **Total plugin latency: ~10-20ms** (granular grain size)
- Report via `setLatencySamples()` for host compensation

### Denormal Protection

- Use `juce::ScopedNoDenormals` in processBlock() (prevent CPU spikes)
- All JUCE DSP components handle denormals internally
- Granular grain buffer wrapping prevents denormals (explicit modulo wrapping)
- Delay line interpolation handles denormals (JUCE DelayLine internals)

### Known Challenges

**Granular Pitch Shifting Complexity:**
- **Challenge:** Most algorithmically complex component (grain management, overlap-add, variable-rate playback)
- **Mitigation:** Implement in isolation first, unit test pitch accuracy, reference existing implementations
- **Testing:** Verify pitch shift accuracy with tone generator (440 Hz → 880 Hz at +50% doppler)

**Cumulative Pitch Shift Escalation:**
- **Challenge:** High feedback + high doppler creates rapid pitch escalation (may shift out of audible range quickly)
- **Mitigation:** Document expected behavior (this is intentional, not a bug), test with tone generator
- **User control:** Users can reduce feedback or doppler for subtler effects

**Feedback Loop Stability:**
- **Challenge:** Cumulative doppler + saturation in feedback may cause unexpected behavior
- **Mitigation:** Clamp feedback to 0.95 max, test extreme settings, add soft clipper if needed
- **Testing:** Verify stability at feedback=95%, saturation=+24dB, doppler=+50%

**CPU Usage:**
- **Challenge:** Granular engine may use significant CPU (~20-40% depending on settings)
- **Mitigation:** Make grain settings user-adjustable, start with 2-grain overlap, profile early
- **Optimization:** Bypass optimizations, larger grain sizes, update grain spawning less frequently

**Artifact Management:**
- **Challenge:** Small grain sizes (25-50ms) may have slight graininess
- **Mitigation:** Default to 100ms grain size, document expected artifacts at extremes
- **User control:** grainSize and grainOverlap exposed for quality tuning

---

## References

### Contract Files
- Creative brief: `plugins/RedShiftDistortion/.ideas/creative-brief.md` (✓ FINALIZED)
- DSP architecture: `plugins/RedShiftDistortion/.ideas/architecture.md` (✓ FINALIZED)
- Continue-here: `plugins/RedShiftDistortion/.continue-here.md` (needs update)

### Reference Plugins
- **FabFilter Timeless 3** - Granular pitch shifting, time stretching
- **Eventide H3000** - Pitch shifting with feedback, cumulative effects
- **TapeAge** - WebView parameter binding, basic delay structure
- **GainKnob** - Basic WebView setup, relative knob drag pattern

### JUCE Critical Patterns
- juce8-critical-patterns.md #11: WebView member initialization (use std::unique_ptr)
- juce8-critical-patterns.md #12: WebSliderParameterAttachment 3-param constructor
- juce8-critical-patterns.md #15: valueChangedEvent callback (no parameters passed)
- juce8-critical-patterns.md #16: Relative knob drag (frame-delta, not absolute)

### Technical References
- Curtis Roads: "Real-Time Granular Synthesis with Texture Control"
- Granular synthesis parameters: 50-100ms grain size, 4x overlap for smooth output
- Pitch ratio formula: `ratio = 2^(semitones / 12)`
- Hann window formula: `window[i] = 0.5 * (1.0 - cos(2π * i / grainSize))`

---

## Success Criteria

**Stage 1 (Foundation + Shell):**
- [ ] Project builds without errors (VST3, AU, Standalone)
- [ ] Plugin loads in DAW
- [ ] All 15 parameters defined in APVTS
- [ ] Parameter ranges correct (dB, %, Hz, ms units)
- [ ] No references to deleted parameters (lfoRate, lfoDepth, lfoTempoSync)

**Stage 2 (DSP):**
- [ ] Series processing chain works (stereo width → delay → granular doppler → saturation → output)
- [ ] Stereo width creates spatial positioning via L/R delay differential
- [ ] Granular pitch shifter shifts pitch accurately (±12 semitones / ±1 octave)
- [ ] **Cumulative doppler behavior verified** (each repeat shifts pitch further)
- [ ] Feedback loop creates repeating delays with cumulative effects
- [ ] Saturation in feedback loop creates cumulative tape warmth
- [ ] Filters shape feedback frequency response (hi-cut + lo-cut)
- [ ] All bypass toggles work correctly (stereoWidth, delay, doppler, saturation)
- [ ] grainSize and grainOverlap parameters adjust quality vs CPU trade-off
- [ ] Feedback loop is stable (no runaway oscillation at 95% feedback)
- [ ] CPU usage within budget (~37-57% single core at 48kHz)
- [ ] No clicks, pops, or artifacts at default settings
- [ ] Acceptable artifacts at extreme settings (documented)

**Stage 3 (GUI):**
- [ ] WebView UI loads with updated parameter set (15 parameters)
- [ ] Main controls section displays correctly
- [ ] Bypass controls section displays correctly
- [ ] Advanced settings panel displays correctly (collapsible)
- [ ] All knobs interactive (drag updates parameters)
- [ ] Host automation updates UI knobs
- [ ] Parameter values display correctly (formatted strings with units)
- [ ] No frozen knobs or binding issues
- [ ] No references to deleted parameters in UI code

**Stage 4 (Validation):**
- [ ] 10 presets created (showcasing cumulative doppler + stereo width + saturation)
- [ ] pluginval passes (VST3, AU, Standalone)
- [ ] Build script succeeds
- [ ] Changelog generated
- [ ] Plugin installed to system folders

---

## Risk Assessment

**Highest Risk: Granular Pitch Shifter (Phase 2.2)**
- 70% of project risk
- Most algorithmically complex component
- No JUCE class (custom implementation required)
- Moderate to high CPU cost (~20-40% single core)
- Artifacts possible if grain size/overlap incorrect
- Cumulative behavior adds testing complexity

**Mitigation:**
- Implement in isolation first (unit test before feedback integration)
- Start with 2-grain overlap (simpler, lower CPU)
- Make grain settings user-adjustable (advanced panel)
- Reference Curtis Roads granular synthesis literature
- Profile CPU early and optimize if >50% single core
- Test pitch accuracy with tone generator

**Medium Risk: Cumulative Feedback Behavior (Phase 2.3)**
- 20% of project risk
- Cumulative pitch shift grows exponentially (each repeat multiplies pitch ratio)
- High feedback + high doppler may shift pitch out of audible range quickly
- Saturation in feedback may interact unpredictably

**Mitigation:**
- Document expected cumulative behavior (pitch escalation is intentional)
- Test with extreme settings (feedback=95%, doppler=+50%, saturation=+24dB)
- Clamp feedback gain to 0.95 maximum (5% safety margin)
- Add soft clipper after saturation if runaway occurs

**Low Risk: All Other Components**
- 10% of project risk
- Stereo width is simple (differential L/R delays with exponential smoothing)
- Saturation is simple (tanh waveshaping)
- Filters are standard JUCE IIR
- WebView UI is standard pattern

---

## Duration Estimate

**Total estimated time:** 6-9 days

- Stage 1 (Foundation + Shell): 1 day
- Stage 2 (DSP - 5 phases): 6-9 days
  - Phase 2.1: 1 day
  - Phase 2.2: 3-5 days ← CRITICAL PATH
  - Phase 2.3: 1 day
  - Phase 2.4: 1 day
  - Phase 2.5: 0.5-1 day
- Stage 3 (GUI): 1 day
- Stage 4 (Validation): 1 day

**Critical path:** Phase 2.2 (Granular Pitch Shifter) is longest and highest risk. All subsequent phases depend on granular engine being stable and performant.

**Risk buffer:** 3-day range (6-9 days) accounts for granular engine complexity and potential CPU optimization needs.
