# Stage 0: Research

**Context:** This file is part of the plugin-planning skill.
**Invoked by:** Main orchestrator in `SKILL.md` when starting planning workflow
**Purpose:** Understand what we're building before writing code through research and DSP architecture specification

---

**Goal:** Create DSP architecture specification (architecture.md)

**Duration:** 5-10 minutes

**Model Configuration:**
- Extended thinking: ENABLED
- Budget: 10000 tokens

---

## Prerequisites

1. Check for creative brief:
```bash
if [ ! -f "plugins/${PLUGIN_NAME}/.ideas/creative-brief.md" ]; then
    echo "✗ creative-brief.md not found"
    echo "Run /dream ${PLUGIN_NAME} first"
    exit 1
fi
```

2. Check plugin status (must not be past Stage 1):
```bash
STATUS=$(grep -A 2 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "Status:" | awk '{print $2}')
if [[ "$STATUS" =~ Stage\ [2-6] ]]; then
    echo "✗ Plugin already past planning stage"
    echo "Use /continue or /improve instead"
    exit 1
fi
```

---

## Research Process

### 1. Read Creative Brief

```bash
cat plugins/${PLUGIN_NAME}/.ideas/creative-brief.md
```

Extract key information:
- Plugin type (effect, instrument, utility)
- Core audio functionality
- Target use case
- Key features
- Sonic character

### 2. Identify Technical Approach

Determine:
- **Input/Output:** Mono, stereo, sidechain, multi-channel?
- **Processing Domain:** Time-domain, frequency-domain (FFT), granular, sample-based?
- **Real-time Requirements:** Low latency critical? Lookahead acceptable?
- **State Management:** Stateless or stateful processing?

### 3. Research JUCE DSP Modules

**Search for relevant juce::dsp classes:**

Use WebSearch to find JUCE documentation for:
- Audio processing (juce::dsp namespace)
- DSP utilities (filters, delays, reverb, etc.)
- Sample rate conversion
- FFT/frequency analysis

**Common JUCE DSP components:**

| Component Type | JUCE Classes |
|---------------|--------------|
| Gain/Volume | `juce::dsp::Gain` |
| Filters | `juce::dsp::IIR::Filter`, `juce::dsp::StateVariableFilter` |
| Delay | `juce::dsp::DelayLine` |
| Reverb | `juce::dsp::Reverb` |
| Dynamics | `juce::dsp::Compressor`, `juce::dsp::Limiter`, `juce::dsp::NoiseGate` |
| Distortion | `juce::dsp::WaveShaper` |
| Modulation | `juce::dsp::Oscillator`, `juce::dsp::LookupTableTransform` |
| Mixing | `juce::dsp::DryWetMixer` |
| FFT | `juce::dsp::FFT`, `juce::dsp::WindowingFunction` |
| Convolution | `juce::dsp::Convolution` |

**Document findings:**
- Which JUCE classes match the plugin needs?
- Are any custom algorithms required?
- What's missing from JUCE (need custom implementation)?

### 4. Research Professional Plugins

**Search for industry examples:**

Find 3-5 similar plugins from:
- FabFilter (modern, clean)
- Waves (industry standard)
- UAD (hardware emulation)
- Valhalla (reverb/modulation)
- iZotope (intelligent processing)
- Soundtoys (creative effects)

**Document for each:**
- Plugin name and manufacturer
- Core features
- Sonic characteristics
- Typical parameter ranges observed
- Unique approaches or algorithms

**Example:**
```markdown
1. **Valhalla VintageVerb**
   - Classic plate reverb algorithms
   - Decay range: 0.1-20s typical
   - Heavy damping at short decay times
   - Warmth control for analog character

2. **FabFilter Pro-R**
   - Modern algorithmic reverb
   - Real-time spectrum visualization
   - Decay affects both roomSize and damping
   - Pre-delay range: 0-500ms
```

### 5. Research Parameter Ranges

For each parameter type in the creative brief:

**Gain/Volume:**
- Range: -60dB to +20dB typical
- Default: 0dB (unity)
- Skew: Linear dB or exponential amplitude

**Filter Cutoff:**
- Range: 20Hz to 20kHz
- Default: 1kHz (center) or off
- Skew: Exponential (log frequency scale)

**Time-based (Delay, Reverb):**
- Range: 0ms to 5000ms (delay), 0.1s to 20s (reverb decay)
- Default: Context-dependent
- Skew: Linear or exponential depending on range

**Modulation Rate:**
- Range: 0.01Hz to 20Hz
- Default: 1Hz (slow) or 5Hz (fast)
- Skew: Exponential (wide range)

**Mix/Blend:**
- Range: 0% to 100%
- Default: 50% or context-dependent
- Skew: Linear

**Reference existing plugins in system:**
```bash
# Find similar parameter ranges in existing plugins
grep -r "addParameter" plugins/*/Source/*.cpp | grep -i "[parameter-type]"
```

### 6. Design Sync Check (If Mockup Exists)

Check for existing UI mockup:
```bash
ls -la plugins/${PLUGIN_NAME}/.ideas/mockups/v*-ui.yaml 2>/dev/null
```

**If mockup exists:**

1. Read mockup file to extract parameters
2. Read creative brief to extract expected parameters
3. Compare parameter lists

**If conflicts found:**
- Parameter in mockup but not in brief
- Parameter in brief but not in mockup
- Different parameter types or ranges

**Invoke design-sync skill:**
```
Use Skill tool to invoke: design-sync
Pass plugin name and detected conflicts
```

**Document sync results:**
```markdown
## Design Sync Results

**Status:** [✓ Synchronized | ⚠ Conflicts resolved | ✗ Manual review needed]

**Changes made:**
- [List any parameter additions/removals]
- [List any range adjustments]
- [List any type changes]

**Contracts updated:**
- [creative-brief.md | parameter-spec.md | both]
```

---

## Create DSP Architecture Document

**Use template:** `assets/architecture-template.md`

**File location:** `plugins/${PLUGIN_NAME}/.ideas/architecture.md`

### Required Sections

#### 1. Header
```markdown
# DSP Architecture: [PluginName]

**CRITICAL CONTRACT:** This specification is immutable during Stages 2-5 implementation.

**Generated by:** Stage 0 Research
**Referenced by:** Stage 1 (Planning), Stage 4 (DSP Implementation)
**Purpose:** DSP specification defining processing components, signal flow, and JUCE module usage
```

#### 2. Core Components

For each DSP component identified:

```markdown
### [Component Name]
- **JUCE Class:** `juce::dsp::ClassName` or "Custom implementation (description)"
- **Purpose:** [What this component does]
- **Parameters Affected:** [List parameter IDs]
- **Configuration:**
  - [Initialization settings]
  - [Parameter mappings and ranges]
  - [Special handling notes]
```

**Mapping research to components:**
- JUCE classes identified → List each as a component
- Custom algorithms needed → Describe implementation approach
- Parameter research → Document ranges in Configuration section
- Professional plugins → Reference in Research References section

#### 3. Processing Chain

Create ASCII diagram showing:
- Signal flow from input to output
- Where each component fits
- Parameter control points
- Parallel paths or feedback loops

```
Example:
Input
  ↓
Dry/Wet Mixer (capture) ← MIX
  ↓
[Main Processing Chain]
  ↓
Output Gain ← VOLUME
  ↓
Dry/Wet Mixer (blend) ← MIX
  ↓
Output
```

#### 4. Parameter Mapping

Create table mapping every parameter to DSP components:

| Parameter ID | Type | Range | DSP Component | Usage |
|-------------|------|-------|---------------|-------|
| ... | ... | ... | ... | ... |

**Source:** Extract from creative brief + parameter-spec.md (if exists)

#### 5. Algorithm Details

For each component, describe implementation:
- Mathematical formulas
- Coefficient calculations
- Interpolation methods
- Edge case handling

**Use research findings:**
- JUCE documentation for built-in components
- Professional plugin behavior for custom algorithms
- Technical resources for DSP theory

#### 6. Special Considerations

Document:

**Thread Safety:**
- How parameters are accessed (atomic? locks?)
- Buffer ownership
- State updates

**Performance:**
- Estimated CPU per component
- Hot paths
- Optimization opportunities

**Denormal Protection:**
- Strategy (ScopedNoDenormals, flush-to-zero, etc.)
- Which components need it

**Sample Rate Handling:**
- Sample-rate-dependent calculations
- prepareToPlay requirements
- Coefficient updates on rate change

**Latency:**
- Sources of latency (delays, lookahead, etc.)
- Total latency calculation
- Host compensation via getLatencySamples()

#### 7. Research References

Document all research:

**Professional Plugins:**
- List each plugin researched
- Key observations
- Parameter ranges noted

**JUCE Documentation:**
- Classes researched
- Key findings
- Usage patterns

**Technical Resources:**
- Books, papers, tutorials
- Algorithms studied
- Reference implementations

---

## State Management

### 1. Create Handoff File

**File:** `plugins/${PLUGIN_NAME}/.continue-here.md`

**Content:**
```yaml
---
plugin: [PluginName]
stage: 0
status: complete
last_updated: [YYYY-MM-DD HH:MM:SS]
---

# Resume Point

## Current State: Stage 0 - Research Complete

DSP architecture documented. Ready to proceed to planning.

## Completed So Far

**Stage 0:** ✓ Complete
- Plugin type defined: [Type]
- Professional examples researched: [Count]
- JUCE modules identified: [List]
- DSP feasibility verified
- Parameter ranges researched

## Next Steps

1. Stage 1: Planning (calculate complexity, create implementation plan)
2. Review architecture.md findings
3. Pause here

## Files Created
- plugins/[PluginName]/.ideas/architecture.md
```

### 2. Update PLUGINS.md

**Check if entry exists:**
```bash
if ! grep -q "^### ${PLUGIN_NAME}$" PLUGINS.md; then
    # Create new entry
    echo "Creating new PLUGINS.md entry"
else
    # Update existing entry
    echo "Updating existing PLUGINS.md entry"
fi
```

**New entry format:**
```markdown
### [PluginName]

**Status:** 🚧 Stage 0
**Type:** [Audio Effect | MIDI Instrument | Synth | Utility]
**Created:** [YYYY-MM-DD]

[Brief description from creative-brief.md]

**Lifecycle Timeline:**
- **[YYYY-MM-DD]:** Creative brief created
- **[YYYY-MM-DD] (Stage 0):** Research completed - DSP architecture documented

**Last Updated:** [YYYY-MM-DD]
```

**Update existing entry:**
```markdown
# Change status
**Status:** 💡 Ideated → **Status:** 🚧 Stage 0

# Add timeline entry
- **[YYYY-MM-DD] (Stage 0):** Research completed - DSP architecture documented

# Update last updated
**Last Updated:** [YYYY-MM-DD]
```

### 3. Git Commit

```bash
git add \
  plugins/${PLUGIN_NAME}/.ideas/architecture.md \
  plugins/${PLUGIN_NAME}/.continue-here.md \
  PLUGINS.md

git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 0 - research complete

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

**Display commit hash:**
```bash
git log -1 --format='✓ Committed: %h - Stage 0 complete'
```

---

## Decision Menu

**Present numbered list (NOT AskUserQuestion):**

```
✓ Stage 0 complete: DSP architecture documented

What's next?
1. Continue to Stage 1 - Planning (recommended)
2. Review architecture.md findings
3. Improve creative brief based on research
4. Run deeper JUCE investigation (deep-research skill) ← Discover troubleshooting
5. Pause here
6. Other

Choose (1-6): _
```

**Handle user input:**

| Input | Action |
|-------|--------|
| 1 or "continue" | Proceed to Stage 1 |
| 2 or "review" | Display architecture.md, re-present menu |
| 3 | Open creative brief for editing, re-present menu |
| 4 | Invoke deep-research skill, return to menu |
| 5 or "pause" | Exit skill with handoff file ready |
| 6 or "other" | Ask "What would you like to do?", re-present menu |

---

**Return to:** Main plugin-planning orchestration in `SKILL.md`
