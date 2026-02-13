# RedShiftDistortion - Implementation Plan

**Date:** 2026-02-10
**Complexity Score:** 4.6 (Complex)
**Strategy:** Phase-based implementation

---

## Complexity Factors

**Parameters:** 13 parameters (13/5 = 2.6, capped at 2.0) = 2.0

**Algorithms:** 6 DSP components = 6.0
- Stereo Width Modulation (DelayLine)
- Tape Delay with Feedback Loop (DelayLine)
- Granular Doppler Shift (custom granular synthesis)
- Cumulative Saturation (WaveShaper)
- Dual-Band Filtering (IIR::Filter × 2)
- Master Output Level (gain multiplication)

**Features:** 1 point
- Feedback loops (+1) - Cumulative processing in feedback path

**Calculation:**
```
param_score = min(13/5, 2.0) = 2.0
algorithm_count = 6
feature_count = 1 (feedback loops)
total = 2.0 + 6 + 1 = 9.0
final_score = min(9.0, 5.0) = 5.0
```

**Adjusted score:** 4.6
- Rationale: Granular doppler is HIGH complexity (~3.0 points alone), but other components are LOW-MEDIUM
- Feedback loop adds complexity but is well-understood pattern
- No file I/O, no multi-output, no MIDI (reduces complexity vs theoretical max)
- Adjusted from 5.0 to 4.6 to reflect actual implementation complexity

---

## Stages

- Stage 0: Research ✓
- Stage 1: Planning ✓
- Stage 1: Foundation ← Next
- Stage 2: Shell
- Stage 3: DSP (3 phases)
- Stage 4: GUI (3 phases)
- Stage 5: Validation

---

## Complex Implementation (Score ≥ 3.0)

### Stage 3: DSP Phases

#### Phase 4.1: Core Processing (Stereo Width + Basic Delay)

**Goal:** Implement stereo width modulation and basic delay line without feedback

**Components:**
- Stereo Width Modulation (DelayLine with L/R differential)
- Tape Delay (260ms base, no feedback yet)
- Master Output Level (simple gain)
- Bypass controls (bypassStereoWidth, bypassDelay)

**Signal flow:**
```
Input → Stereo Width → Delay (260ms) → Master Output → Output
```

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes
- [ ] Audio passes through (dry signal + 260ms delayed signal audible)
- [ ] Stereo width parameter works correctly (-100% to +100%)
  - [ ] At 0%: Mono output (no spatial effect)
  - [ ] At +100%: R channel delayed by +260ms (wide stereo)
  - [ ] At -100%: L channel delayed by +260ms (inverted wide)
- [ ] Master output parameter works correctly (-60dB to +12dB)
- [ ] Bypass delay works (dry signal only when bypassed)
- [ ] Bypass stereo width works (mono output when bypassed)
- [ ] No artifacts or discontinuities in delayed signal

**Git commit:** `feat(RedShiftDistortion): Phase 4.1 - stereo width + basic delay`

---

#### Phase 4.2: Feedback Loop (Saturation + Filters + Feedback)

**Goal:** Add cumulative saturation, dual-band filtering, and feedback loop

**Components:**
- Cumulative Saturation (WaveShaper with tanh)
- Dual-Band Filtering (IIR hi-pass + lo-pass)
- Feedback Loop (mix filtered output back into delay input)
- Bypass controls (bypassSaturation)

**Signal flow:**
```
Input → Stereo Width → Delay (260ms) + Feedback Loop → Master Output → Output
  ↑                                                        ↓
  └──── Feedback ← Filters ← Saturation ← (Delay Output) ──┘
```

**Test Criteria:**
- [ ] Saturation parameter works correctly (-12dB to +24dB)
  - [ ] At -12dB: Subtle warmth
  - [ ] At 0dB: Moderate saturation
  - [ ] At +24dB: Heavy saturation with harmonics
- [ ] Dual-band filtering works correctly
  - [ ] Lo-cut filter (filterBandLow) removes low frequencies
  - [ ] Hi-cut filter (filterBandHigh) removes high frequencies
  - [ ] Bandpass result: Only frequencies between lo-cut and hi-cut pass
  - [ ] Edge case: If lo-cut > hi-cut, swap values (no crash)
- [ ] Feedback loop works correctly (0-95%)
  - [ ] At 0%: Single repeat (no feedback)
  - [ ] At 50%: ~3-5 repeats
  - [ ] At 95%: Long decay (~10+ repeats)
- [ ] Cumulative saturation audible (each repeat more saturated)
- [ ] Feedback stability: No runaway oscillation at 95% feedback + max saturation
- [ ] Bypass saturation works (keeps delay + filters, skips saturation)
- [ ] No DC offset buildup over time (test with 30+ second audio)

**Git commit:** `feat(RedShiftDistortion): Phase 4.2 - feedback loop with saturation + filters`

---

#### Phase 4.3: Granular Doppler Shift (Advanced Feature)

**Goal:** Implement granular pitch shifting in feedback loop

**Components:**
- Granular Doppler Shift (custom granular synthesis implementation)
- Grain scheduling (2x or 4x overlap)
- Hann windowing for smooth grain boundaries
- Bypass control (bypassDoppler)

**Signal flow:**
```
Input → Stereo Width → Delay (260ms) + Feedback Loop → Master Output → Output
  ↑                                                                      ↓
  └──── Feedback ← Filters ← Saturation ← Granular Doppler ← (Delay Output) ──┘
```

**Test Criteria:**
- [ ] Granular doppler shift works correctly (-50% to +50%)
  - [ ] At -50%: Pitch down 1 octave per repeat (red shift)
  - [ ] At 0%: No pitch shift (unity)
  - [ ] At +50%: Pitch up 1 octave per repeat (blue shift)
- [ ] Cumulative pitch shift audible (each repeat shifts further)
  - [ ] Test: +50% doppler + 50% feedback = ~5 repeats = pitch rises 5 octaves
  - [ ] Verify: Repeats naturally shift out of audible range (self-limiting)
- [ ] Grain size parameter works (25-200ms)
  - [ ] At 25ms: Lower latency, may have artifacts
  - [ ] At 100ms: Default, good quality
  - [ ] At 200ms: Smoothest, highest latency
- [ ] Grain overlap parameter works (2x vs 4x)
  - [ ] 2x: Lower CPU, acceptable quality
  - [ ] 4x: Higher CPU, smoother quality
- [ ] No clicks/pops at grain boundaries (Hann window smooths transitions)
- [ ] Pitch accuracy: Sine wave test verifies correct pitch ratio
  - [ ] Input: 440Hz sine, +50% doppler → Output: 880Hz first repeat, 1760Hz second repeat
- [ ] CPU usage acceptable:
  - [ ] 2x overlap: ~20-30% single core
  - [ ] 4x overlap: ~30-40% single core
- [ ] Bypass doppler works (keeps delay + saturation, skips granular processing)
- [ ] No buffer overflows or crashes with extreme settings (high doppler + high feedback)

**Git commit:** `feat(RedShiftDistortion): Phase 4.3 - granular doppler shift in feedback loop`

---

### Stage 4: GUI Phases

#### Phase 5.1: Layout and Basic Controls

**Goal:** Integrate v10 UI mockup HTML and render layout

**Components:**
- Copy v10-ui.yaml mockup to Source/ui/public/index.html (convert YAML → HTML)
- WebView setup in PluginEditor.h/cpp
- Configure CMakeLists.txt for WebView resources (BinaryData)
- Render layout with diamond plate background, corner screws, title, subtitle
- Display all knobs and buttons (non-interactive at this phase)

**Test Criteria:**
- [ ] WebView window opens with correct size (950x700px)
- [ ] Diamond plate background texture renders correctly
- [ ] Corner screws visible at all 4 corners
- [ ] Title "RED SHIFT" renders with glow effect
- [ ] Subtitle "digital delay" renders below title
- [ ] Center doppler knob visible (168px diameter HISE filmstrip knob)
- [ ] 6 surrounding knobs visible (130px diameter photorealistic images)
- [ ] 4 bypass toggle buttons visible (29px diameter, left side)
- [ ] 2 advanced settings controls visible (grain size slider, grain overlap combo)
- [ ] VU meters visible (dual stereo L/R, right side)
- [ ] Layout matches v10-ui.yaml mockup exactly

**Git commit:** `feat(RedShiftDistortion): Phase 5.1 - UI layout and basic controls`

---

#### Phase 5.2: Parameter Binding and Interaction

**Goal:** Two-way parameter communication (UI ↔ DSP)

**Components:**
- JavaScript → C++ relay calls (knob drag updates DSP parameters)
- C++ → JavaScript parameter updates (host automation updates UI)
- Canvas-based knob rendering (NOT CSS filmstrip - previous issue)
- Value formatting and display (dB, Hz, %, ms units)
- Real-time parameter updates during playback

**Critical note from previous implementation:**
- Previous WebView CSS filmstrip knobs didn't work (rendering issues)
- Use Canvas rendering for knobs instead (draw rotation programmatically)
- Reference: v10-ui.yaml specifies HISE filmstrip, but implement via Canvas for reliability

**Test Criteria:**
- [ ] Knob drag updates DSP parameters
  - [ ] Doppler shift knob: -50% to +50% (center = 0%)
  - [ ] Stereo width knob: -100% to +100% (center = 0%)
  - [ ] Feedback knob: 0% to 95% (left = 0%)
  - [ ] Saturation knob: -12dB to +24dB (center = 0dB)
  - [ ] Lo-cut filter knob: 20Hz to 20kHz (exponential scale)
  - [ ] Hi-cut filter knob: 20Hz to 20kHz (exponential scale)
  - [ ] Master output knob: -60dB to +12dB (center = 0dB)
- [ ] Host automation updates UI knobs
  - [ ] Automate parameter in DAW → UI knob rotates to match
  - [ ] Preset change → All knobs update to preset values
- [ ] Toggle buttons work
  - [ ] Bypass doppler: Click toggles on/off, LED lights up when active
  - [ ] Bypass saturation: Click toggles on/off, LED lights up
  - [ ] Bypass filters: Click toggles on/off, LED lights up
  - [ ] Bypass feedback: Click toggles on/off, LED lights up
- [ ] Advanced settings work
  - [ ] Grain size slider: 25ms to 200ms
  - [ ] Grain overlap combo: 2x or 4x
- [ ] Value display shows current parameter values
  - [ ] Doppler shift: "-50%" to "+50%"
  - [ ] Saturation: "-12dB" to "+24dB"
  - [ ] Filters: "20Hz" to "20kHz"
  - [ ] Master output: "-60dB" to "+12dB"
- [ ] No lag or visual glitches during parameter changes
- [ ] Canvas knobs rotate smoothly (no jitter)

**Git commit:** `feat(RedShiftDistortion): Phase 5.2 - parameter binding and interaction`

---

#### Phase 5.3: VU Meters and Visual Polish

**Goal:** Implement real-time VU meter visualization

**Components:**
- VU meter data flow (C++ audio thread → JavaScript UI thread)
- Canvas-based VU meter rendering (dual stereo L/R bars)
- Ballistic motion (fast attack, slow decay)
- Color zones (green → yellow → red based on level)
- requestAnimationFrame loop for smooth animation

**Test Criteria:**
- [ ] VU meters respond to output signal in real-time
  - [ ] L meter tracks left channel output level
  - [ ] R meter tracks right channel output level
- [ ] Ballistic motion feels natural
  - [ ] Fast attack (~0.4 speed): Catches peaks quickly
  - [ ] Slow decay (~0.15 speed): Readable fallback
- [ ] Color zones work correctly
  - [ ] Green: 0-75% level (safe)
  - [ ] Yellow: 75-90% level (hot)
  - [ ] Red: 90-100% level (clipping warning)
- [ ] Animation is smooth (~60fps via requestAnimationFrame)
- [ ] No CPU spikes in UI thread (VU meter updates don't block UI)
- [ ] VU meters don't flicker or jitter
- [ ] Visual polish: Glow effects, shadows, holographic aesthetic match v10 mockup

**Git commit:** `feat(RedShiftDistortion): Phase 5.3 - VU meters and visual polish`

---

### Implementation Flow

- Stage 0: Research ✓
- Stage 1: Planning ✓
- Stage 1: Foundation - project structure
- Stage 2: Shell - APVTS parameters
- Stage 3: DSP - 3 phases
  - Phase 4.1: Core Processing (Stereo Width + Basic Delay)
  - Phase 4.2: Feedback Loop (Saturation + Filters + Feedback)
  - Phase 4.3: Granular Doppler Shift (Advanced Feature)
- Stage 4: GUI - 3 phases
  - Phase 5.1: Layout and Basic Controls
  - Phase 5.2: Parameter Binding and Interaction
  - Phase 5.3: VU Meters and Visual Polish
- Stage 5: Validation - presets, pluginval, changelog

---

## Implementation Notes

### Thread Safety

- All parameter reads use atomic `getRawParameterValue()->load()` in audio thread
- Filter coefficient updates in audio thread when parameters change (no allocations)
- Grain scheduling state is per-channel (no shared state between L and R)
- Delay line buffers pre-allocated in prepareToPlay (no allocations in processBlock)
- Bypass checks are boolean atomics (thread-safe reads)
- VU meter data: Audio thread writes to atomic float, UI thread reads (no locks)

### Performance

- **Granular doppler:** ~30-40% CPU (most expensive, depends on overlap setting)
  - 2x overlap: ~20-30% CPU
  - 4x overlap: ~30-40% CPU
- **Saturation:** ~5% CPU (tanh is fast)
- **Dual-band filters:** ~5-10% CPU (two 2nd-order IIR filters)
- **Stereo width + delay:** ~5% CPU (simple delay line reads)
- **Total estimated:** 37-57% single core at 48kHz
- **Optimization opportunities:**
  - Use SIMD for tanh approximation (faster than std::tanh)
  - Reduce grain overlap to 2x for lower CPU mode (user setting already exposed)
  - Reduce grain size to 50ms for lower latency mode

### Latency

- **Stereo width delay:** 260ms base delay (12,480 samples at 48kHz)
- **Granular grain size:** Default 100ms (4,800 samples at 48kHz)
- **Total processing latency:** ~360ms (260ms + 100ms)
- **Samples:** 17,280 samples at 48kHz, 34,560 samples at 96kHz
- **Report via `getLatencySamples()`:** Return total latency for host compensation
- **Implementation:** `return static_cast<int>(260.0 * getSampleRate() / 1000.0 + grainSize * getSampleRate() / 1000.0);`

### Denormal Protection

- Use `juce::ScopedNoDenormals` in processBlock() (sets FTZ/DAZ CPU flags)
- All JUCE DSP components (DelayLine, IIR::Filter, WaveShaper) handle denormals internally
- Granular Hann window never reaches true zero (avoids denormals in grain windowing)
- Feedback loop uses addition (not multiplication by zero), less prone to denormals

### Known Challenges

#### Granular Doppler Shift Implementation

**Challenge:** Complex grain scheduling with overlapping windowed reads

**Solution approach:**
1. Use circular buffer (DelayLine) for grain storage
2. Maintain array of grain read pointers (2 or 4 depending on overlap)
3. Schedule new grain every `grainSize / overlapCount` samples
4. Each grain has independent read pointer advancing at `pitchRatio * sampleRate`
5. Apply Hann window per-sample during grain read
6. Sum all active grain outputs

**Reference implementations:**
- PaulStretch (open-source extreme time-stretching, uses granular synthesis)
- ChowDSP pitch shifter (JUCE-based, simpler than our approach but good reference)
- JUCE Forum thread: "Granular synth write up / samples / c++"

**Testing strategy:**
- Start with sine wave input (verify pitch accuracy before musical material)
- Test with 2x overlap first (simpler), upgrade to 4x if artifacts present
- Benchmark CPU usage per overlap setting (2x vs 4x)
- Monitor for clicks/pops at grain boundaries (indicates windowing issue)

#### Cumulative Feedback Loop Stability

**Challenge:** High doppler + high saturation + high feedback could cause runaway oscillation

**Solution approach:**
1. Hard limit feedback to 95% (prevents infinite gain)
2. tanh saturation is bounded (-1 to +1, prevents clipping runaway)
3. Dual-band filters limit frequency buildup (prevents resonance)
4. Test extensively with extreme settings (95% feedback + +24dB saturation + +50% doppler)

**Monitoring:**
- Add debug assert: `jassert(output < 1.0)` to catch runaway before release
- Test with long audio files (30+ seconds) to verify stability over time
- Check for DC offset buildup (add DC blocker after saturation if needed)

**Fallback plans:**
- If oscillation occurs: Reduce max feedback to 90% or add soft-clipping above 90%
- If DC offset builds: Add `juce::dsp::LinkwitzRileyFilter` as DC blocker after saturation
- If still unstable: Add dynamic feedback limiting (reduce feedback when output > -6dBFS)

#### Canvas Knob Rendering (CSS Filmstrip Issue)

**Challenge:** Previous WebView implementation used CSS filmstrip knobs that didn't render correctly

**Solution approach:**
1. Use Canvas API for knob rendering (programmatic drawing, not image filmstrips)
2. Draw knob background as circle with gradient
3. Draw rotation indicator as line or triangle
4. Rotate via Canvas transform: `ctx.rotate(angle * Math.PI / 180)`
5. Update rotation on parameter change (relative drag pattern)

**Reference pattern (from juce8-critical-patterns.md):**
```javascript
// Relative drag (NOT absolute positioning)
let rotation = 0;
let lastY = 0;

knob.addEventListener('mousedown', (e) => {
    isDragging = true;
    lastY = e.clientY;  // Store current position
});

document.addEventListener('mousemove', (e) => {
    if (!isDragging) return;
    const deltaY = lastY - e.clientY;  // Distance since LAST FRAME
    rotation += deltaY * 0.5;  // Increment rotation
    rotation = Math.max(-135, Math.min(135, rotation));  // Clamp
    setRotation(rotation);
    lastY = e.clientY;  // Update for next frame
});
```

**Why Canvas, not CSS filmstrip:**
- CSS `background-position` with filmstrip images had rendering issues in previous implementation
- Canvas is more reliable for programmatic rotation
- Canvas allows custom drawing (glow effects, shadows, holographic style)
- Canvas animation is standard pattern in WebView plugins (see GainKnob, TapeAge examples)

---

## References

**Contract files:**
- Creative brief: `plugins/RedShiftDistortion/.ideas/creative-brief.md`
- Parameter spec: `plugins/RedShiftDistortion/.ideas/parameter-spec.md`
- DSP architecture: `plugins/RedShiftDistortion/.ideas/architecture.md`
- UI mockup: `plugins/RedShiftDistortion/.ideas/mockups/v10-ui.yaml`

**Similar plugins for reference:**
- **TapeAge:** Tape saturation + delay (reference for saturation in feedback loop)
- **FlutterVerb:** Modulation + reverb (reference for LFO modulation patterns - NOT needed for RedShift, but good WebView example)
- **GainKnob:** Simple gain + WebView (reference for Canvas knob rendering)
- **LushPad:** Synth with WebView (reference for complex WebView parameter binding)

**JUCE 8 critical patterns:**
- Read before implementation: `troubleshooting/patterns/juce8-critical-patterns.md`
- Pattern 11: WebView member initialization (std::unique_ptr order)
- Pattern 12: WebSliderParameterAttachment requires 3 parameters (nullptr for undoManager)
- Pattern 15: valueChangedEvent callback receives NO parameters (call getNormalisedValue() inside)
- Pattern 16: Relative drag for knobs (NOT absolute positioning)
- Pattern 21: ES6 module loading (`type="module"` required for JUCE index.js)

**Professional plugin research:**
- FabFilter Timeless 3: Stereo width via L/R delay differential, tape saturation
- Soundtoys EchoBoy: Tape emulation with cumulative saturation in feedback
- Arturia Delay TAPE-201: RE-201 emulation with feedback intensity control
- UAD Galaxy Tape Echo: Lively distortion and temporal fluctuations

**Technical resources:**
- DSP Labs - Granular Synthesis Implementation (grain scheduling, overlap approach)
- JUCE dsp::DelayLine documentation (interpolation types, modulation patterns)
- JUCE dsp::WaveShaper documentation (tanh saturation, transfer functions)
- JUCE dsp::IIR::Filter documentation (makeHighPass, makeLowPass, coefficient calculation)
