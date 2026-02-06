# RedShiftDistortion - Implementation Plan

**Date:** 2026-02-05
**Complexity Score:** 5.0 (Complex - Maximum Complexity)
**Strategy:** Phase-based implementation with incremental testing

---

## Complexity Factors

### Calculation Breakdown

- **Parameters:** 8 parameters (8/5 = 1.6 points, capped at 2.0) = **1.6**
  - saturation, dopplerShift, pitchEnable, delayTime, tempoSync, delayLevel, distortionLevel, masterOutput

- **Algorithms:** 4 DSP components = **4.0**
  - Tube Saturation (tanh waveshaping)
  - Delay Line (juce::dsp::DelayLine with tempo sync)
  - Granular Pitch Shifter (custom 4-grain implementation)
  - Parallel Signal Routing (independent processing paths)

- **Features:** 2 complexity features = **2.0**
  - Granular synthesis (+1) - Complex grain buffer management, overlap-add, windowing
  - Modulation system (+1) - LFO-driven delay time modulation for doppler simulation

- **Total:** 1.6 + 4.0 + 2.0 = **7.6** → **Capped at 5.0**

**Complexity Tier:** 5 (Highest - granular synthesis with parallel routing)

---

## Stages

- ✓ Stage 0: Research & Planning (Complete)
- → Stage 1: Foundation + Shell (Next - Build system + APVTS parameters)
- ⏳ Stage 2: DSP (3 phases - Core → Modulation → Advanced)
- ⏳ Stage 3: GUI (3 phases - Layout → Binding → Advanced)
- ⏳ Stage 4: Validation (Presets, pluginval, changelog)

---

## Complex Implementation (Score = 5.0)

### Stage 2: DSP Implementation (3 Phases)

#### Phase 2.1: Core Processing & Parallel Routing

**Goal:** Establish basic signal flow with parallel paths and simple distortion/delay

**Components:**
- Parallel signal router (split input to two paths)
- Tube saturation engine (tanh waveshaping)
- Basic delay line (fixed time, no tempo sync yet)
- Output mixer (3-stage level control: delay, distortion, master)
- Signal flow validation (both paths process independently)

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes
- [ ] Audio passes through both parallel paths
- [ ] saturation parameter affects distortion path (audible warmth at +12dB)
- [ ] delayTime parameter affects delay path (basic fixed delay 0-16000ms)
- [ ] delayLevel scales delay path volume (-60dB to 0dB)
- [ ] distortionLevel scales distortion path volume (-60dB to 0dB)
- [ ] masterOutput scales final mixed output (-60dB to +12dB)
- [ ] No clicks, pops, or artifacts at extreme parameter values
- [ ] Both stereo channels process correctly

**Duration:** 1-2 days

---

#### Phase 2.2: Tempo Sync & Delay Modulation

**Goal:** Add host tempo synchronization and doppler simulation via delay time modulation

**Components:**
- Tempo sync system (BPM query from host via AudioPlayHead)
- Note division quantization (1/16 to 8 bars)
- Delay time modulator (LFO-driven delay time variation)
- tempoSync parameter toggle (musical vs free time)
- Smooth parameter transitions (prevent clicks on tempo/sync changes)

**Test Criteria:**
- [ ] tempoSync parameter switches between musical (bars) and free time (ms) modes
- [ ] Delay time quantizes to nearest note division when tempo sync ON
- [ ] Host BPM changes update delay time correctly (test tempo automation)
- [ ] Fallback to 120 BPM when host doesn't provide tempo
- [ ] dopplerShift parameter modulates delay time (0-10% variation based on ±50% range)
- [ ] LFO modulation creates audible pitch artifacts (doppler effect)
- [ ] No clicks when switching tempoSync on/off
- [ ] Delay time updates smoothly during tempo changes

**Duration:** 1 day

---

#### Phase 2.3: Granular Pitch Shifting (Advanced Feature)

**Goal:** Implement granular synthesis engine for pitch shifting and time stretching

**Components:**
- Granular synthesis engine (4-grain overlap, Hann window)
- Grain buffer management (100ms grain size at 48kHz)
- Pitch-shifted grain playback (variable-rate resampling)
- Time stretching mode (pitchEnable parameter toggle)
- Overlap-add synthesis (4 simultaneous grains)
- Integration with delay path (pitch shift applied to delayed signal)

**Test Criteria:**
- [ ] dopplerShift parameter shifts pitch up/down (±1 octave / ±12 semitones)
- [ ] Positive doppler (+50%) → pitch down ~1 octave (red shift)
- [ ] Negative doppler (-50%) → pitch up ~1 octave (blue shift)
- [ ] pitchEnable ON → pitch + time change together (natural doppler)
- [ ] pitchEnable OFF → time stretch only (speed changes, no pitch shift)
- [ ] Grain boundaries smooth (no clicks or graininess at moderate settings)
- [ ] Acceptable artifacts at extreme settings (±50% doppler)
- [ ] CPU usage within budget (~30-40% single core at 48kHz)
- [ ] No memory leaks or buffer overflows
- [ ] Stereo processing maintains channel independence

**Duration:** 3-5 days (highest complexity phase)

**Risk Mitigation:**
- Start with 2-grain overlap if CPU too high (trade quality for performance)
- Tune grain size (50-100ms range) to minimize artifacts
- Profile CPU early and optimize if >50% single core
- Reference granular synthesis examples (JUCE forum, Curtis Roads literature)

---

### Stage 3: GUI Implementation (3 Phases)

#### Phase 3.1: Layout and Basic Controls

**Goal:** Integrate WebView mockup HTML and bind primary parameters

**Components:**
- Copy v[N]-ui.html mockup to Source/ui/public/index.html
- Update PluginEditor.h/cpp with WebView setup
- Configure CMakeLists.txt for WebView resources (NEEDS_WEB_BROWSER TRUE)
- Create resource provider (getResource() with explicit URL mapping)
- Bind primary parameters via WebSliderRelay system:
  - saturation knob
  - dopplerShift knob
  - delayTime knob/selector
  - delayLevel, distortionLevel, masterOutput knobs
  - tempoSync toggle
  - pitchEnable toggle

**Test Criteria:**
- [ ] WebView window opens with correct size (match mockup dimensions)
- [ ] All knobs render with holographic/3D aesthetic
- [ ] Background styling matches mockup design
- [ ] Layout matches creative brief vision (central saturation, prominent doppler)
- [ ] VU meter placeholder visible (static, no animation yet)
- [ ] All controls visible and clickable

**Duration:** 1-2 days

---

#### Phase 3.2: Parameter Binding and Interaction

**Goal:** Two-way parameter communication (UI ↔ DSP)

**Components:**
- JavaScript → C++ relay calls (knob drag updates DSP parameters)
- C++ → JavaScript parameter updates (host automation updates UI)
- Value formatting and display (dB for levels, % for doppler, note divisions for tempo sync)
- Real-time parameter updates during playback
- Relative knob drag (frame-delta pattern, NOT absolute positioning)
- Parameter smoothing (prevent audible zipper noise)

**Test Criteria:**
- [ ] Knob drag changes DSP parameters (audible effect)
- [ ] Host automation updates UI knobs (visual feedback)
- [ ] Preset changes update all UI elements simultaneously
- [ ] Parameter values display correctly (formatted strings)
- [ ] No lag or visual glitches during rapid parameter changes
- [ ] Knobs use relative drag (not absolute cursor positioning)
- [ ] Toggle switches reflect state correctly (tempoSync, pitchEnable)
- [ ] No frozen knobs (verify WebSliderParameterAttachment has 3rd parameter: nullptr)

**Duration:** 1-2 days

**Critical Patterns:**
- Use `getSliderState()` for continuous parameters (saturation, doppler, levels)
- Use `getToggleState()` for boolean parameters (tempoSync, pitchEnable)
- Verify `type="module"` on script tags (ES6 module loading)
- Use relative drag pattern (lastY, NOT startY) for knobs
- WebSliderParameterAttachment constructor requires 3 params in JUCE 8: (parameter, relay, nullptr)

---

#### Phase 3.3: VU Meter Animation (Advanced UI)

**Goal:** Implement holographic VU meter with ballistic motion

**Components:**
- VU meter level calculation (dual-path metering: delay + distortion)
- C++ → JavaScript level updates (60Hz update rate)
- JavaScript requestAnimationFrame loop (ballistic motion: fast attack, slow decay)
- Visual styling (holographic glow, color zones: green/yellow/red)
- Needle rotation based on dB level (map -60dB to 0dB → -45° to +45°)

**Test Criteria:**
- [ ] VU meter needle responds to audio level (both delay and distortion paths)
- [ ] Needle has ballistic motion (fast attack ~0.4 speed, slow decay ~0.15 speed)
- [ ] Needle rotation smooth (no jitter or stutter)
- [ ] Color zones change based on level (green <75%, yellow 75-90%, red >90%)
- [ ] Holographic glow effect renders correctly
- [ ] VU meter updates at ~60fps (no UI thread starvation)
- [ ] Level calculation doesn't affect DSP performance (<5% CPU)

**Duration:** 1-2 days

---

### Implementation Flow Summary

```
✓ Stage 0: Research & Planning (Complete)
  - architecture.md created (DSP specification)
  - plan.md created (implementation strategy)

→ Stage 1: Foundation + Shell
  - Build system (CMakeLists.txt, JUCE configuration)
  - APVTS parameter definitions (8 parameters)
  - Placeholder PluginProcessor/PluginEditor

⏳ Stage 2: DSP (3 phases, ~5-8 days)
  - Phase 2.1: Core Processing & Parallel Routing (1-2 days)
  - Phase 2.2: Tempo Sync & Delay Modulation (1 day)
  - Phase 2.3: Granular Pitch Shifting (3-5 days) ← HIGHEST RISK

⏳ Stage 3: GUI (3 phases, ~3-6 days)
  - Phase 3.1: Layout and Basic Controls (1-2 days)
  - Phase 3.2: Parameter Binding and Interaction (1-2 days)
  - Phase 3.3: VU Meter Animation (1-2 days)

⏳ Stage 4: Validation (~1 day)
  - Preset creation (10 presets showcasing doppler effect ranges)
  - pluginval testing (VST3, AU, Standalone)
  - Changelog generation
  - Build verification
```

---

## Implementation Notes

### Thread Safety

- All parameter reads use atomic `APVTS::getRawParameterValue()->load()` (real-time safe)
- Granular grain buffers pre-allocated in prepareToPlay() (no audio thread allocations)
- Delay line buffer sized for maximum delay at 192kHz (no reallocations)
- No shared state between stereo channels (per-channel grain tracking)
- Host tempo query via getPlayHead() is host-provided and real-time safe
- VU meter level calculation uses lock-free atomic<float> for thread-safe updates

### Performance

**Estimated CPU usage per component:**
- Granular pitch shifter: ~25-30% single core (most expensive)
  - 4-grain overlap with 100ms grain size
  - Variable-rate grain playback (interpolation)
- Delay line: ~5% (Lagrange3rd interpolation)
- Tube saturation: ~3% (per-sample tanh)
- Delay time modulator: ~2% (sine LFO calculation)
- Parallel mixing: ~2% (addition + gain)
- **Total estimated: ~37-42% single core @ 48kHz**

**Optimization opportunities if CPU exceeds budget:**
- Reduce grain overlap to 2x (trade quality for 10-15% CPU savings)
- Use linear interpolation instead of Lagrange3rd (trade smoothness for 2-3% CPU savings)
- Apply saturation only if saturation > -10dB (bypass threshold saves ~2% CPU)
- Update doppler modulation every 64 samples (not every sample, saves ~1% CPU)

### Latency

- Granular pitch shifter: ~10-20ms (grain size + overlap delay)
- Delay line: 0-16000ms user-controlled (not counted as latency)
- Tube saturation: 0ms (instantaneous waveshaping)
- **Total plugin latency: ~10-20ms**
- Report via `setLatencySamples()` for host compensation
- Note: User-controlled delay time is NOT latency (it's an effect parameter)

### Denormal Protection

- Use `juce::ScopedNoDenormals` in processBlock() (prevent CPU spikes)
- All JUCE DSP components handle denormals internally
- Granular LFO phase wrapping prevents denormals (explicit modulo at 2π)
- Delay line interpolation handles denormals (JUCE DelayLine internals)

### Known Challenges

**Granular Synthesis Artifacts:**
- **Challenge:** Graininess or metallic sound if grain size/overlap incorrect
- **Mitigation:** Tune grain size (50-100ms range) and overlap (2x-4x) during Phase 2.3
- **Reference:** Curtis Roads "Real-Time Granular Synthesis" for optimal parameters

**Tempo Sync Edge Cases:**
- **Challenge:** Some hosts don't provide BPM, or BPM changes mid-buffer
- **Mitigation:** Fallback to 120 BPM, smooth delay time changes over 10ms
- **Reference:** AngelGrain tempo sync implementation (lines 170-190)

**Parallel Path Mixing:**
- **Challenge:** Summing two independent paths may cause clipping at extreme levels
- **Mitigation:** Test with delayLevel=0dB + distortionLevel=0dB + loud input, add soft clipping if needed
- **Reference:** FlutterVerb dry/wet mixer (similar parallel architecture)

**WebView Parameter Binding:**
- **Challenge:** JUCE 8 requires 3 parameters for WebSliderParameterAttachment (not 2)
- **Mitigation:** Always use (parameter, relay, nullptr) constructor
- **Reference:** juce8-critical-patterns.md #12

**VU Meter Thread Safety:**
- **Challenge:** Audio thread updates VU level, UI thread reads it
- **Mitigation:** Use std::atomic<float> for level value (lock-free)
- **Reference:** FlutterVerb VU meter implementation

---

## References

### Contract Files
- Creative brief: `plugins/RedShiftDistortion/.ideas/creative-brief.md`
- Parameter spec: `plugins/RedShiftDistortion/.ideas/parameter-spec-draft.md` (draft version)
- DSP architecture: `plugins/RedShiftDistortion/.ideas/architecture.md`
- UI mockup: `plugins/RedShiftDistortion/.ideas/mockups/v[N]-ui.yaml` (to be created)

### Reference Plugins
- **AngelGrain** - Tempo sync implementation, grain playback reference
- **FlutterVerb** - VU meter animation, parallel dry/wet mixing
- **TapeAge** - Tube saturation waveshaping, WebView parameter binding
- **GainKnob** - Basic WebView setup, relative knob drag pattern

### JUCE Critical Patterns
- juce8-critical-patterns.md #11: WebView member initialization (use std::unique_ptr)
- juce8-critical-patterns.md #12: WebSliderParameterAttachment 3-param constructor
- juce8-critical-patterns.md #15: valueChangedEvent callback (no parameters passed)
- juce8-critical-patterns.md #16: Relative knob drag (frame-delta, not absolute)
- juce8-critical-patterns.md #20: VU meter requestAnimationFrame loop

---

## Success Criteria

**Stage 1 (Foundation + Shell):**
- [ ] Project builds without errors (VST3, AU, Standalone)
- [ ] Plugin loads in DAW
- [ ] All 8 parameters defined in APVTS
- [ ] Parameter ranges correct (dB for levels, % for doppler, bars/ms for delay)

**Stage 2 (DSP):**
- [ ] Parallel routing works (distortion + delay paths independent)
- [ ] Tube saturation adds warmth and harmonics
- [ ] Delay line responds to delayTime parameter (0-16000ms or 1/16-8 bars)
- [ ] Tempo sync switches between musical and free time modes
- [ ] Doppler effect audible (pitch shift via delay time modulation)
- [ ] Granular pitch shifter shifts pitch ±1 octave smoothly
- [ ] pitchEnable toggles between pitch+time and time-only modes
- [ ] CPU usage within budget (~40% single core at 48kHz)
- [ ] No clicks, pops, or artifacts

**Stage 3 (GUI):**
- [ ] WebView UI loads with holographic/3D aesthetic
- [ ] All knobs interactive (drag updates parameters)
- [ ] Host automation updates UI knobs
- [ ] VU meter animates with ballistic motion
- [ ] Parameter values display correctly (formatted strings)
- [ ] No frozen knobs or binding issues

**Stage 4 (Validation):**
- [ ] 10 presets created (showcasing doppler effect ranges)
- [ ] pluginval passes (VST3, AU, Standalone)
- [ ] Build script succeeds
- [ ] Changelog generated
- [ ] Plugin installed to system folders

---

## Risk Assessment

**Highest Risk: Granular Pitch Shifter (Phase 2.3)**
- 70% of project risk
- Most algorithmically complex component
- No JUCE class (custom implementation required)
- Artifact risk (graininess if tuned incorrectly)
- CPU cost significant (~25-30% single core)

**Mitigation:**
- Phase 2.1 and 2.2 establish working foundation before tackling granular engine
- Granular implementation isolated (can test independently)
- Fallback to 2-grain overlap if 4-grain too CPU-intensive
- Reference existing granular implementations (JUCE forum examples, Curtis Roads literature)

**Medium Risk: Tempo Sync (Phase 2.2)**
- 20% of project risk
- Host BPM query may fail (need fallback)
- Tempo changes mid-buffer require smooth transitions

**Mitigation:**
- AngelGrain reference implementation (proven pattern)
- Default to 120 BPM if host doesn't provide tempo
- Smooth delay time changes over 10ms (prevent clicks)

**Low Risk: All Other Components**
- 10% of project risk
- Tube saturation is simple (tanh waveshaping)
- Parallel routing is straightforward
- WebView UI is standard pattern

---

## Duration Estimate

**Total estimated time:** 9-15 days

- Stage 1 (Foundation + Shell): 1 day
- Stage 2 (DSP - 3 phases): 5-8 days
- Stage 3 (GUI - 3 phases): 3-6 days
- Stage 4 (Validation): 1 day

**Critical path:** Phase 2.3 (Granular Pitch Shifting) is longest and highest risk. All other stages can proceed once foundation is in place.
