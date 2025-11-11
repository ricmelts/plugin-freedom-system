---
name: plugin-planning
description: Interactive research and planning (Stages 0-1) for JUCE plugin development
allowed-tools:
  - Read # For contracts and references
  - Write # For architecture.md, plan.md
  - Edit # For state updates (PLUGINS.md, .continue-here.md)
  - Bash # For git commits, precondition checks
  - WebSearch # For professional plugin research
  - Grep # For searching existing implementations
  - Glob # For finding reference files
preconditions:
  - creative-brief.md must exist in plugins/[Name]/.ideas/
  - Plugin must NOT already be past Stage 1
---

# plugin-planning Skill

**Purpose:** Handle Stages 0-1 (Research and Planning) through interactive contract creation without subagents. This skill creates the foundation contracts (architecture.md, plan.md) that guide implementation.

**Invoked by:** `/plan` command (to be created) or as first step of `/implement` workflow

---

## Entry Point

**Check preconditions first:**

1. Verify creative brief exists:
```bash
if [ ! -f "plugins/${PLUGIN_NAME}/.ideas/creative-brief.md" ]; then
    echo "✗ creative-brief.md not found"
    exit 1
fi
```

2. Check plugin status in PLUGINS.md:
```bash
STATUS=$(grep -A 2 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "Status:" | awk '{print $2}')
```

- If status is 🚧 Stage N where N >= 2: **BLOCK** - Plugin already past planning
- If status is 💡 Ideated or not found: OK to proceed

3. Check for existing contracts:
```bash
# Check what already exists
test -f "plugins/${PLUGIN_NAME}/.ideas/architecture.md" && echo "✓ architecture.md exists"
test -f "plugins/${PLUGIN_NAME}/.ideas/plan.md" && echo "✓ plan.md exists"
```

**Resume logic:**
- If architecture.md exists but plan.md doesn't: Skip to Stage 1
- If both exist: Ask user if they want to regenerate or proceed to implementation
- If neither exists: Start at Stage 0

---

## Stage 0: Research

**Goal:** Create DSP architecture specification (architecture.md)

**Duration:** 5-10 minutes

**Process:**

1. Read creative brief:
```bash
cat plugins/${PLUGIN_NAME}/.ideas/creative-brief.md
```

2. Identify plugin technical approach:
   - Audio effect, MIDI effect, synthesizer, or utility?
   - Input/output configuration (mono, stereo, sidechain)
   - Processing type (time-domain, frequency-domain, granular)

3. Research JUCE DSP modules:
   - Search for relevant juce::dsp classes for the identified plugin type
   - Use WebSearch for JUCE documentation and examples
   - Document specific classes (e.g., juce::dsp::Gain, juce::dsp::IIR::Filter)

4. Research professional plugin examples:
   - Search web for industry leaders (FabFilter, Waves, UAD, etc.)
   - Document 3-5 similar plugins
   - Note sonic characteristics and typical parameter ranges

5. Research parameter ranges:
   - Industry-standard ranges for plugin type
   - Typical defaults (reference professional plugins)
   - Skew factors for nonlinear ranges

6. Check design sync (if mockup exists):
   - Look for `plugins/${PLUGIN_NAME}/.ideas/mockups/v*-ui.yaml`
   - If exists: Compare mockup parameters with creative brief
   - If conflicts found: Invoke design-sync skill to resolve
   - Document sync results

**Output:**

Create `plugins/${PLUGIN_NAME}/.ideas/architecture.md` using the template from `assets/architecture-template.md`.

**Required sections:**
1. Title: `# DSP Architecture: [PluginName]`
2. Contract header (immutability statement)
3. `## Core Components` - Each DSP component with structured format
4. `## Processing Chain` - ASCII diagram showing signal flow
5. `## Parameter Mapping` - Table mapping parameter IDs to components
6. `## Algorithm Details` - Implementation approach for each algorithm
7. `## Special Considerations` - Thread safety, performance, denormals, sample rate
8. `## Research References` - Professional plugins, JUCE docs, technical resources

**State management:**

1. Create/update `.continue-here.md`:
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
- Plugin type defined
- Professional examples researched
- DSP feasibility verified
- Parameter ranges researched

## Next Steps

1. Stage 1: Planning (calculate complexity, create implementation plan)
2. Review research findings
3. Pause here

## Files Created
- plugins/[PluginName]/.ideas/architecture.md
```

2. Update PLUGINS.md status to `🚧 Stage 0` and add timeline entry

3. Git commit:
```bash
git add plugins/${PLUGIN_NAME}/.ideas/architecture.md plugins/${PLUGIN_NAME}/.continue-here.md PLUGINS.md
git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 0 - research complete

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

**Decision menu (numbered format):**

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

Wait for user input. Handle:
- Number (1-6): Execute corresponding option
- "continue" keyword: Execute option 1
- "pause" keyword: Execute option 5
- "review" keyword: Execute option 2
- "other": Ask "What would you like to do?" then re-present menu

---

## Stage 1: Planning

**Goal:** Calculate complexity and create implementation plan (plan.md)

**Duration:** 2-5 minutes

**Preconditions:**

Check for required contracts:
```bash
test -f "plugins/${PLUGIN_NAME}/.ideas/parameter-spec.md" && echo "✓ parameter-spec.md" || echo "✗ parameter-spec.md MISSING"
test -f "plugins/${PLUGIN_NAME}/.ideas/architecture.md" && echo "✓ architecture.md" || echo "✗ architecture.md MISSING"
```

**If parameter-spec.md OR architecture.md is missing:**

STOP IMMEDIATELY and BLOCK with this message:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ BLOCKED: Cannot proceed to Stage 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Missing implementation contracts:

Required contracts:
✓ creative-brief.md - exists
[✓/✗] parameter-spec.md - [exists/MISSING (required)]
[✓/✗] architecture.md - [exists/MISSING (required)]

WHY BLOCKED:
Stage 1 planning requires complete specifications to prevent implementation
drift. These contracts are the single source of truth.

HOW TO UNBLOCK:
1. parameter-spec.md: Complete ui-mockup two-phase workflow
   - Run: /dream [PluginName]
   - Choose: "Create UI mockup"
   - Design UI and finalize (Phase 4.5)
   - Finalization generates parameter-spec.md

2. architecture.md: Create DSP architecture specification
   - Run Stage 0 (Research) to generate architecture.md
   - Document DSP components and processing chain
   - Map parameters to DSP components
   - Save to plugins/[PluginName]/.ideas/architecture.md

Once both contracts exist, Stage 1 will proceed.
```

Exit skill and wait for user to create contracts.

**Process (contracts confirmed present):**

1. Read all contracts:
```bash
cat plugins/${PLUGIN_NAME}/.ideas/parameter-spec.md
cat plugins/${PLUGIN_NAME}/.ideas/architecture.md
```

2. Calculate complexity score:

**Formula:**
```
score = min(param_count / 5, 2.0) + algorithm_count + feature_count
Cap at 5.0
```

**Extract metrics:**

From parameter-spec.md:
- Count parameters (each parameter definition = 1)
- param_score = min(param_count / 5, 2.0)

From architecture.md:
- Count distinct DSP algorithms/components
- algorithm_count = number of juce::dsp classes or custom algorithms

Features to identify (from architecture.md):
- Feedback loops present? (+1)
- FFT/frequency domain processing? (+1)
- Multiband processing? (+1)
- Modulation systems (LFO, envelope)? (+1)
- External MIDI control? (+1)
- feature_count = sum of above

**Display breakdown:**
```
Complexity Calculation:
- Parameters: [N] parameters ([N/5] points, capped at 2.0) = [X.X]
- Algorithms: [N] DSP components = [N]
- Features: [List features] = [N]
Total: [X.X] / 5.0
```

3. Determine implementation strategy:
   - **Simple (score ≤ 2.0):** Single-pass implementation
   - **Complex (score ≥ 3.0):** Phase-based implementation with staged commits

4. For complex plugins, create phases:

**Stage 4 (DSP) phases:**
- Phase 4.1: Core processing (essential audio path)
- Phase 4.2: Parameter modulation (APVTS integration)
- Phase 4.3: Advanced features (if applicable)

**Stage 5 (GUI) phases:**
- Phase 5.1: Layout and basic controls
- Phase 5.2: Advanced UI elements
- Phase 5.3: Polish and styling (if applicable)

Each phase needs description, test criteria, estimated duration.

**Output:**

Create `plugins/${PLUGIN_NAME}/.ideas/plan.md` using the template from `assets/plan-template.md`.

**State management:**

1. Update `.continue-here.md`:
```yaml
---
plugin: [PluginName]
stage: 1
status: complete
last_updated: [YYYY-MM-DD HH:MM:SS]
complexity_score: [X.X]
phased_implementation: [true/false]
---

# Resume Point

## Current State: Stage 1 - Planning Complete

Implementation plan created. Ready to proceed to foundation (Stage 2).

## Completed So Far

**Stage 0:** ✓ Complete
**Stage 1:** ✓ Complete
- Complexity score: [X.X]
- Strategy: [Single-pass | Phased implementation]
- Plan documented

## Next Steps

1. Stage 2: Foundation (create build system)
2. Review plan details
3. Pause here

## Files Created
- plugins/[PluginName]/.ideas/architecture.md
- plugins/[PluginName]/.ideas/plan.md
```

2. Update PLUGINS.md status to `🚧 Stage 1` and add timeline entry

3. Git commit:
```bash
git add plugins/${PLUGIN_NAME}/.ideas/plan.md plugins/${PLUGIN_NAME}/.continue-here.md PLUGINS.md
git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 1 - planning complete

Complexity: [X.X]
Strategy: [Single-pass | Phased implementation]

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

**Decision menu (numbered format):**

```
✓ Stage 1 complete: Implementation plan created (Complexity [X.X], [single-pass/phased])

What's next?
1. Continue to Stage 2 - Foundation (via /implement) (recommended)
2. Review plan.md details
3. Adjust complexity assessment
4. Review contracts (parameter-spec, architecture) ← Discover design-sync
5. Pause here
6. Other

Choose (1-6): _
```

Wait for user input. Handle:
- Number (1-6): Execute corresponding option
- "continue" keyword: Execute option 1
- "pause" keyword: Execute option 5
- "review" keyword: Execute option 2
- "other": Ask "What would you like to do?" then re-present menu

---

## Handoff to Implementation

**When user chooses to proceed to Stage 2:**

Create final handoff file that plugin-workflow skill expects:

```yaml
---
plugin: [PluginName]
stage: 1
status: complete
last_updated: [YYYY-MM-DD HH:MM:SS]
complexity_score: [X.X]
phased_implementation: [true/false]
next_stage: 2
ready_for_implementation: true
---

# Ready for Implementation

Planning complete. All contracts created:
- ✓ creative-brief.md
- ✓ parameter-spec.md
- ✓ architecture.md
- ✓ plan.md

Run `/implement [PluginName]` to begin Stage 2 (Foundation).
```

Display to user:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ Planning Complete
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Plugin: [PluginName]
Complexity: [X.X] ([Simple/Complex])
Strategy: [Single-pass | Phased implementation]

Contracts created:
✓ creative-brief.md
✓ parameter-spec.md
✓ architecture.md
✓ plan.md

Ready to build. Run: /implement [PluginName]
```

---

## Reference Files

Detailed stage implementations are in:
- `references/stage-0-research.md` - Research stage details
- `references/stage-1-planning.md` - Planning stage details

Templates are in:
- `assets/architecture-template.md` - DSP architecture contract template
- `assets/plan-template.md` - Implementation plan template
