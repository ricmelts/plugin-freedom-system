# Design-Sync Refactor Analysis

## Executive Summary

**Current State:**
The design-sync skill operates as a standalone validation gate that compares finalized mockups against creative briefs to detect "drift" (parameter mismatches, missing features, style divergence). It runs at two points: (1) automatically after ui-mockup Phase 5.5 when user chooses "Finalize", and (2) mandatorily before Stage 2 in plugin-workflow. When drift is detected, users must manually reconcile by updating either the brief or the mockup.

**Desired State:**
Eliminate design-sync entirely. When users finalize a mockup in ui-mockup, auto-update the creative-brief.md to reflect the final UI design, treating the mockup as the source of truth for UI decisions. This removes friction from the natural iteration process where designs evolve through v1, v2, v3... as users refine their vision.

**Key Findings:**
- Design-sync is deeply integrated at two critical gates: ui-mockup Phase 5.6 and plugin-workflow Stage 1→2 transition
- Creative brief has clearly separable sections: conceptual content (preserve) vs UI-specific content (update from mockup)
- The drift detection logic can be repurposed as a "smart update" that merges mockup reality into brief structure
- Safe removal requires updating 4 skills, removing 2 files, and modifying decision menus
- No backward compatibility concerns—system only applies to new plugins going forward

---

## Current Architecture

### Design-Sync Skill Components

**File Structure:**
- `.claude/skills/design-sync/SKILL.md` - Main skill definition (579 lines)
- `.claude/skills/design-sync/references/` - Supporting documentation
- `.claude/skills/design-sync/assets/` - Templates (evolution-template.md)

**Validation Flow (7-step sequence):**

1. **Step 0: Check Cache** - Skip validation if content unchanged since last run
2. **Step 1: Load Contracts** - Read creative-brief.md, parameter-spec.md, v[N]-ui.yaml
3. **Step 2: Quantitative Checks** - Extract parameter counts, compare (brief vs spec vs mockup)
4. **Step 3: Semantic Validation** - Use extended thinking to analyze visual style, feature completeness, scope changes
5. **Step 4: Categorize Drift** - Classify as: No Drift / Acceptable Evolution / Attention Required / Critical
6. **Step 5: Present Findings** - Show decision menu based on drift category
7. **Step 6: Execute User Choice** - Update brief, update mockup, or override
8. **Step 7: Cache Result and Route Back** - Cache validation, return to ui-mockup Phase 5.5

**Drift Categories and Thresholds:**

| Category | Conditions | User Options |
|----------|-----------|--------------|
| No Drift | ±1 param, all features present, style aligned, high confidence | Continue |
| Acceptable Evolution | +2-4 params, visual polish, justified additions, core intact | Update brief / Continue / Review |
| Attention Required | Missing features, style mismatch, ±5 params, medium confidence | Update brief / Update mockup / Override / Review |
| Critical Drift | Contradicts core concept, 2x params, opposite style, low confidence | Update brief / Update mockup (NO override) |

**What It Validates:**

**Quantitative (Step 2):**
- Parameter count match (creative brief mentions vs parameter-spec.md count vs mockup control count)
- Feature presence (grep for "preset", "bypass", "meter", "visualization", "tabs")
- Scope changes (additions, reductions)

**Semantic (Step 3 - extended thinking):**
- Visual style alignment (brief aesthetic intent vs mockup design system: colors, typography, layout)
- Feature completeness (brief promises vs mockup delivery)
- Use case support (brief scenarios vs mockup capabilities)
- Scope assessment (evolution vs creep, simplification vs missing)

**How It Detects Drift:**

1. **Parse creative brief** - Extract parameter mentions (quoted or CAPS), feature keywords
2. **Parse parameter-spec.md** - Count total parameters, extract names
3. **Parse mockup YAML** - Count controls in layout, detect UI features (preset_browser, meter, etc.)
4. **Compare counts** - Apply thresholds: 0-1 diff = match, 2-4 = acceptable, 5+ = drift, any decrease = drift
5. **Extended thinking analysis** - LLM compares brief aesthetic quotes with YAML design system
6. **Confidence check** - HIGH/MEDIUM/LOW based on clarity of alignment assessment

**Dependencies:**
- Preconditions: creative-brief.md, parameter-spec.md, v[N]-ui.yaml must exist
- Tool requirements: Python 3.8+, jq (for JSON/YAML processing)
- Extended thinking tool: `mcp__sequential-thinking__sequentialthinking` (8000 token budget)
- Validation cache utilities: `.claude/utils/validation-cache.sh`

---

## UI-Mockup Integration Points

### Phase 5.6: Automatic Design Validation Gate

**Current Flow:**
```
User selects "Finalize" in Phase 5.5 menu
    ↓
ui-mockup invokes design-sync via Skill tool
    ↓
design-sync runs 7-step validation
    ↓
IF drift detected:
    design-sync presents findings + resolution menu
    User chooses: update brief / update mockup / override / review
    IF update mockup: Return to ui-mockup Phase 5.5 (iterate)
    IF update brief: Continue to Phase 6-10 (implementation files)
    IF override: Log to .validator-overrides.yaml, continue
    ↓
IF no drift:
    design-sync routes back to ui-mockup Phase 5.5 decision menu
    ui-mockup proceeds to Phase 6-10 (generate 5 implementation files)
```

**Key Code References:**

**ui-mockup SKILL.md lines 675-711:**
```markdown
## Phase 5.6: Automatic Design Validation (Finalize Gate)

**Trigger:** User selected "Finalize" option in Phase 5.5

**Protocol:**
1. Invoke design-sync skill automatically
2. Handle validation results:
   - If validation passes (no drift): Proceed to Phase 6-10
   - If drift detected: design-sync presents resolution options
     - If user updates mockup: Return to Phase 5.5 with updated design
     - If user updates brief: Continue to Phase 6-10 with updated contracts
     - If user overrides: Log decision, continue to Phase 6-10
3. Skip validation only if: No creative-brief.md exists (standalone mode)
```

**ui-mockup Phase 5.5 Decision Menu (lines 620-671):**
```
What would you like to do?

1. Iterate - Refine design, adjust layout
2. Finalize - Validate alignment and complete mockup  ← triggers Phase 5.6
3. Save as template - Add to aesthetic library for reuse
4. Other
```

**Where Creative Brief Update Logic Should Integrate:**

**Option 1: Replace Phase 5.6 entirely**
- Remove design-sync invocation
- Add new Phase 5.6: "Update creative brief from finalized mockup"
- Auto-update brief's UI sections with mockup data
- Present confirmation: "Brief updated to reflect final design. Continue to implementation files?"

**Option 2: Modify finalization flow before Phase 6**
- After user chooses "Finalize" (Phase 5.5 option 2)
- Before generating implementation files (Phase 6-10)
- Insert brief update step with single confirmation gate

**Recommendation: Option 1** - Cleaner conceptual model, replaces validation with transformation

---

### Data Available at Finalization Time

When ui-mockup reaches Phase 5.6 (after user chooses "Finalize"), these artifacts exist:

**Files Generated:**
- `v[N]-ui.yaml` - Complete design specification (layout, colors, typography, controls, metadata)
- `v[N]-ui-test.html` - Browser-testable mockup (interactive, parameter messages)
- `parameter-spec.md` - Parameter definitions (generated in Phase 4 if v1, or already exists if v2+)

**Metadata in YAML:**
```yaml
# Example from v2-ui.yaml
metadata:
  plugin_name: "Drum808"
  version: 2
  created: "2025-11-13"

design_system:
  colors:
    background: "#1a1a1a"
    panel: "#2a2a2a"
    accent: "#ff6b35"
  typography:
    primary: "Arial, sans-serif"
    size_base: 14px
  aesthetic: "Modern utilitarian with 808 heritage"

layout:
  type: "horizontal-channels"
  dimensions:
    width: 1000
    height: 500
  controls:
    - id: kick_level
      type: slider
      position: [channel1, row1]
    # ... 23 more controls

features:
  preset_browser: true
  trigger_indicators: true
  master_meter: true
  individual_outputs: true
```

**Extractable Information:**

| Data Category | Source | Extraction Method |
|---------------|--------|-------------------|
| Parameter count | parameter-spec.md | Count `### {param_name}` sections |
| Parameter names | parameter-spec.md | Extract from section headers |
| Visual style | v[N]-ui.yaml | Read `design_system.aesthetic` + color palette |
| Layout type | v[N]-ui.yaml | Read `layout.type` |
| Window dimensions | v[N]-ui.yaml | Read `layout.dimensions` |
| Special features | v[N]-ui.yaml | Read `features.*` keys |
| Color scheme | v[N]-ui.yaml | Read `design_system.colors` |
| Typography | v[N]-ui.yaml | Read `design_system.typography` |
| Control types | v[N]-ui.yaml | Parse `layout.controls[].type` |

**Example Extraction Code:**
```bash
# Extract parameter count
PARAM_COUNT=$(grep -c '^###' plugins/Drum808/.ideas/parameter-spec.md)

# Extract visual aesthetic
AESTHETIC=$(yq '.design_system.aesthetic' plugins/Drum808/.ideas/mockups/v2-ui.yaml)

# Extract dimensions
WIDTH=$(yq '.layout.dimensions.width' plugins/Drum808/.ideas/mockups/v2-ui.yaml)
HEIGHT=$(yq '.layout.dimensions.height' plugins/Drum808/.ideas/mockups/v2-ui.yaml)

# Extract features
FEATURES=$(yq '.features | keys' plugins/Drum808/.ideas/mockups/v2-ui.yaml)
```

---

## Creative Brief Structure

### Section Breakdown

Based on Drum808 example (`plugins/Drum808/.ideas/creative-brief.md`):

**Conceptual Sections (PRESERVE - user's original vision):**
- **Overview** → Type, core concept, status, created date
- **Vision** → Why this plugin exists, what problem it solves, target audience
- **Use Cases** → Specific scenarios where plugin is used
- **Inspirations** → Reference plugins, hardware, artistic influences
- **Technical Notes** → DSP architecture considerations, performance notes
- **Next Steps** → Workflow checklist (auto-generated, safe to overwrite)

**UI-Specific Sections (UPDATE from mockup):**
- **Parameters** → Parameter definitions, ranges, defaults, DSP descriptions
- **UI Concept** → Layout, visual style, key elements, dimensions

**Example of Section to Preserve:**
```markdown
## Vision

Drum808 is an authentic emulation of the legendary Roland TR-808 drum machine,
focusing on circuit-accurate synthesis for six classic voices: kick, low tom,
mid tom, clap, closed hat, and open hat. Unlike hardware-style step sequencers,
this plugin integrates seamlessly with modern DAW workflows via MIDI triggering,
allowing you to program patterns in your sequencer while retaining the warm,
analog character of the original 808.

[... technical synthesis details ...]
```

**Example of Section to Update:**
```markdown
## UI Concept

**Layout:** Six vertical channel strips (one per drum), each containing four
knobs (Level, Tone, Decay/Snap, Tuning) arranged vertically. Global section
at top with master output level. Visual trigger indicators for each voice.

**Visual Style:** Authentic 808 aesthetic—red/orange LED-style trigger
indicators, brushed metal panel background, sans-serif labeling. Functional
and utilitarian, not skeuomorphic recreation of hardware.

**Key Elements:**
- Per-drum trigger LEDs (light up on MIDI note-on)
- Individual output routing indicators
- Master level meter
- Preset browser (user presets only)

**Dimensions:** Approximately 1000×500px (horizontal layout, six channels)
```

### Update Strategy

**Smart Merge Algorithm:**

1. **Read current creative-brief.md**
2. **Extract conceptual sections** (Vision, Use Cases, Inspirations, Technical Notes)
3. **Parse finalized mockup YAML** (v[N]-ui.yaml)
4. **Parse parameter-spec.md** (if exists)
5. **Generate new UI Concept section** from mockup metadata:
   ```markdown
   ## UI Concept

   **Layout:** {layout.type from YAML} with {control count} controls arranged {describe from layout.controls}
   **Visual Style:** {design_system.aesthetic from YAML}
   **Color Scheme:** {design_system.colors from YAML}
   **Dimensions:** {layout.dimensions from YAML}
   **Key Elements:**
   {generate bullet list from features.* keys in YAML}
   ```
6. **Generate new Parameters section** from parameter-spec.md:
   - Copy entire "## Parameters" section verbatim
   - Maintains ranges, defaults, DSP descriptions user wrote
7. **Reconstruct creative-brief.md:**
   ```markdown
   # {PluginName} - Creative Brief

   ## Overview
   {preserve from original}

   ## Vision
   {preserve from original}

   ## Parameters
   {UPDATED from parameter-spec.md}

   ## UI Concept
   {UPDATED from mockup YAML}

   ## Use Cases
   {preserve from original}

   ## Inspirations
   {preserve from original}

   ## Technical Notes
   {preserve from original}

   ## Next Steps
   {auto-generated workflow checklist}
   ```

**Preservation Logic:**

**PRESERVE (exact copy from original brief):**
- All text in "## Vision" section
- All text in "## Use Cases" section
- All text in "## Inspirations" section
- All text in "## Technical Notes" section
- "## Overview" section metadata (type, core concept, status, created date)

**UPDATE (generate from mockup):**
- "## Parameters" section → Copy from parameter-spec.md
- "## UI Concept" section → Generate from v[N]-ui.yaml metadata
- "## Next Steps" section → Auto-generate workflow checklist

**Template Compliance:**

The creative brief template (if it exists at `.claude/skills/plugin-ideation/assets/creative-brief.md`) defines the canonical section order and format. The update logic should:

1. Read template to determine section order
2. Preserve conceptual sections from original brief
3. Update UI sections from mockup
4. Maintain template structure (headings, formatting, section order)

**No template found in glob search**, so section order is inferred from existing briefs. Recommended canonical order:
```
1. Title (# PluginName - Creative Brief)
2. Overview
3. Vision
4. Parameters
5. UI Concept
6. Use Cases
7. Inspirations
8. Technical Notes
9. Next Steps
```

---

## Workflow Changes Required

### 1. plugin-workflow Stage 1→2 Transition

**Current Implementation (lines 388-456 in plugin-workflow SKILL.md):**

```markdown
<design_sync_gate enforcement_level="MANDATORY">
  **Purpose:** Prevent design drift before implementation begins.

  **When:** BEFORE dispatching Stage 2 (foundation-shell-agent), IF mockup exists.

  **Conditions:**
  - IF plugins/[PluginName]/.ideas/mockups/ directory exists
  - AND parameter-spec.md exists (mockup finalized)
  - THEN design-sync validation is REQUIRED

  **Implementation:**
  Before dispatching Stage 2:

  1. Check for mockup:
     - Look in plugins/[PluginName]/.ideas/mockups/
     - Find latest version (highest v[N] prefix)
     - If any mockup files exist: Invoke design-sync skill

  2. If mockup exists:
     - Run design-sync validation automatically
     - Present findings with decision menu
     - BLOCK Stage 2 until one of:
       a) No drift detected (continue)
       b) Acceptable evolution (user confirms)
       c) Drift resolved (user updates brief or mockup)
       d) User explicitly overrides (logged)

  3. If no mockup:
     - Skip design-sync (no visual design to validate)
     - Proceed to Stage 2 directly
```

**Required Changes:**

**Remove entire `<design_sync_gate>` block (lines 389-456)**

**Replace with:**
```markdown
<creative_brief_sync enforcement_level="AUTOMATIC">
  **Purpose:** Ensure creative brief reflects finalized mockup before implementation.

  **When:** BEFORE dispatching Stage 2 (foundation-shell-agent), IF mockup exists.

  **Implementation:**
  Before dispatching Stage 2:

  1. Check for finalized mockup:
     IF plugins/[PluginName]/.ideas/mockups/v*-ui.yaml exists
     AND plugins/[PluginName]/.ideas/parameter-spec.md exists:
       Proceed to step 2
     ELSE:
       Skip sync, proceed to Stage 2

  2. Verify brief is current:
     - Read .continue-here.md for "brief_updated_from_mockup" flag
     - IF flag == true AND mockup version matches:
       Skip sync (already done during finalization)
     - ELSE:
       Present info message:
       "Notice: Mockup finalized but brief not yet updated.
        This should have happened during mockup finalization.
        Updating now to ensure alignment..."
       Proceed to step 3

  3. Update creative brief from mockup:
     - Read current creative-brief.md
     - Extract conceptual sections (Vision, Use Cases, Inspirations, Technical Notes)
     - Parse v[latest]-ui.yaml for UI metadata
     - Parse parameter-spec.md for parameter definitions
     - Generate new Parameters and UI Concept sections
     - Reconstruct brief maintaining section order
     - Write updated creative-brief.md
     - Commit: "docs(PluginName): sync creative brief with finalized mockup v[N]"

  4. Confirm and continue:
     Display: "✓ Creative brief updated from mockup v[N]
              Contracts aligned. Proceeding to Stage 2..."
     Continue to foundation-shell-agent dispatch

  **No user interaction required** - automatic sync replaces validation gate.
</creative_brief_sync>
```

**Rationale:**
- Removes blocking validation
- Replaces with automatic sync
- Maintains contract alignment guarantee
- No user decision needed (mockup is source of truth)

### 2. ui-mockup Phase 5.6

**Current Implementation (lines 675-711):**

```markdown
## Phase 5.6: Automatic Design Validation (Finalize Gate)

**Trigger:** User selected "Finalize" option in Phase 5.5

**Protocol:**
1. Invoke design-sync skill automatically
2. Handle validation results:
   - If validation passes (no drift): Proceed to Phase 6-10
   - If drift detected: design-sync presents resolution options
3. Skip validation only if: No creative-brief.md exists (standalone mode)
```

**Required Changes:**

**Replace entire Phase 5.6 with:**

```markdown
## Phase 5.6: Update Creative Brief from Finalized Mockup

**Purpose:** Automatically sync creative-brief.md with finalized mockup design.

**Trigger:** User selected "Finalize" option in Phase 5.5

**Protocol:**

1. **Check if creative-brief.md exists:**
   ```bash
   if [ ! -f "plugins/${PLUGIN_NAME}/.ideas/creative-brief.md" ]; then
     echo "Standalone mode: No creative brief to update"
     # Skip to Phase 6-10
   fi
   ```

2. **Read mockup metadata:**
   - v[N]-ui.yaml (finalized design)
   - parameter-spec.md (finalized parameters)
   - Extract: layout type, dimensions, visual style, color scheme, features

3. **Preserve conceptual content:**
   - Read current creative-brief.md
   - Extract sections: Overview, Vision, Use Cases, Inspirations, Technical Notes
   - Store in memory for reconstruction

4. **Generate UI sections:**
   - Create new "## Parameters" section from parameter-spec.md
   - Create new "## UI Concept" section from mockup YAML:
     ```markdown
     **Layout:** {layout type from YAML} - {describe arrangement}
     **Visual Style:** {design_system.aesthetic}
     **Color Scheme:** {colors from YAML}
     **Dimensions:** {width}×{height}px
     **Key Elements:**
     - {feature 1 from YAML}
     - {feature 2 from YAML}
     ```

5. **Reconstruct creative-brief.md:**
   - Maintain canonical section order
   - Combine preserved sections + updated UI sections
   - Write to creative-brief.md

6. **Update workflow state:**
   - Mark in .continue-here.md:
     ```yaml
     brief_updated_from_mockup: true
     mockup_version_synced: {N}
     brief_update_timestamp: {ISO-8601}
     ```

7. **Commit changes:**
   ```bash
   git add plugins/${PLUGIN_NAME}/.ideas/creative-brief.md
   git add plugins/${PLUGIN_NAME}/.continue-here.md
   git commit -m "docs(${PLUGIN_NAME}): sync creative brief with finalized mockup v${VERSION}"
   ```

8. **Present confirmation:**
   ```
   ✓ Creative brief updated from mockup v{N}

   Updated sections:
   - Parameters (24 parameters from parameter-spec.md)
   - UI Concept (layout, visual style, dimensions)

   Preserved sections:
   - Vision (original concept intact)
   - Use Cases (unchanged)
   - Inspirations (unchanged)
   - Technical Notes (unchanged)

   Proceeding to implementation file generation...
   ```

9. **Continue to Phase 6-10:** Generate 5 implementation files

**Error Handling:**
- If brief parse fails: Present error, offer manual update option
- If YAML parse fails: Fallback to minimal update (dimensions only)
- If git commit fails: Warn but continue (state recoverable)

**No user interaction required** - automatic update with confirmation display only.
```

**Phase 5.5 Decision Menu Update:**

No changes needed to menu text, but option 2 routing changes:

**Before:**
```
Option 2: Finalize
  → Invoke design-sync (Phase 5.6)
  → design-sync presents findings + menu
  → User resolves drift
  → Return to Phase 5.5 OR continue to Phase 6-10
```

**After:**
```
Option 2: Finalize
  → Auto-update creative brief (Phase 5.6)
  → Display confirmation
  → Continue to Phase 6-10 (no gates)
```

### 3. ui-mockup References

**File:** `.claude/skills/ui-mockup/references/decision-menus.md`

**Lines 62-66 (Phase 5.5 Option 2 routing):**

**Current:**
```markdown
**Option 2: Finalize design**
- Action: Proceed to Phase 5.6 (automatic validation gate)
- This is the ONLY option that proceeds to Phase B (files 3-7)
- Verification steps:
  1. Check WebView constraints validation (Phase 5.3 already executed)
  2. Invoke design-sync skill automatically (Phase 5.6)
  3. Verify user explicitly confirmed finalization
  4. Mark design as finalized in YAML file
  5. Proceed to Phase 6-10 (generate 5 implementation files)
- If any verification fails: show errors, return to Phase 5.5 menu
```

**Update to:**
```markdown
**Option 2: Finalize design**
- Action: Proceed to Phase 5.6 (automatic brief update)
- This is the ONLY option that proceeds to Phase B (files 3-7)
- Verification steps:
  1. Check WebView constraints validation (Phase 5.3 already executed)
  2. Update creative-brief.md from mockup (Phase 5.6 - automatic)
  3. Verify user explicitly confirmed finalization
  4. Mark design as finalized in YAML file
  5. Proceed to Phase 6-10 (generate 5 implementation files)
- If any step fails: show errors, offer retry or manual fix
```

### 4. Other References to design-sync

**Files to check:**
- `.claude/agents/ui-design-agent.md` - May reference validation
- `.claude/agents/ui-finalization-agent.md` - May reference validation
- `.claude/agents/research-planning-agent.md` - May reference Stage 0→1 flow
- `CLAUDE.md` - System overview mentions design-sync
- `analyses/*.md` - Various analysis docs may reference it

**Required Updates:**

**CLAUDE.md (lines in Proactive Validation section):**

Search for "Stage 0→2 Transition (design-sync gate)" and update:

**Before:**
```markdown
**Stage 0→2 Transition (design-sync gate):**
- MANDATORY validation of mockup ↔ creative brief alignment
- Catches design drift before Stage 2 generates boilerplate
- Blocks implementation if contracts misaligned
```

**After:**
```markdown
**Stage 0→2 Transition (brief sync):**
- Automatic update of creative brief from finalized mockup
- Ensures contracts reflect final UI design before Stage 2
- No manual reconciliation needed (mockup is source of truth)
```

**Update anywhere "design-sync" or "/sync-design" appears:**
- Replace validation language with sync/update language
- Remove drift/alignment terminology
- Emphasize automatic nature

---

## Removal Strategy

### Safe Removal Sequence

**Step 1: Update Integration Points First (prevent broken references)**

1. Modify `ui-mockup/SKILL.md` Phase 5.6
2. Modify `ui-mockup/references/decision-menus.md` option 2 routing
3. Modify `plugin-workflow/SKILL.md` Stage 1→2 transition
4. Modify `CLAUDE.md` proactive validation section

**Step 2: Remove design-sync Skill**

```bash
# Backup first
cp -r .claude/skills/design-sync .claude/skills/design-sync.REMOVED-$(date +%Y%m%d)

# Remove skill directory
rm -rf .claude/skills/design-sync
```

**Skill deletion checklist:**
- [x] All integration points updated (Step 1)
- [x] No other skills invoke design-sync via Skill tool
- [x] No hooks reference design-sync (verified: no .md hooks exist)
- [x] Cached validation results in system can be ignored (TTL expires automatically)

**Step 3: Remove /sync-design Command**

```bash
# Backup first
cp .claude/commands/sync-design.md .claude/commands/sync-design.md.REMOVED-$(date +%Y%m%d)

# Remove command
rm .claude/commands/sync-design.md
```

**Step 4: Clean Up Documentation**

**Files to update:**
- `CLAUDE.md` - Remove design-sync from skills list, update validation section
- `.claude/docs/integration-contracts.md` - Remove design-sync contract if present
- Any README or overview docs mentioning design-sync

**Grep for all references:**
```bash
grep -r "design-sync\|sync-design" .claude/ --exclude-dir=skills/design-sync.REMOVED* \
  | grep -v ".git" | grep -v "node_modules"
```

**Step 5: Update PLUGINS.md and Workflow Files**

No changes needed—workflow state files (.continue-here.md) don't reference design-sync.

**Step 6: Commit Removal**

```bash
git add -A
git commit -m "refactor: remove design-sync skill, replace with auto-update in ui-mockup

BREAKING CHANGE: /sync-design command removed
- design-sync skill removed entirely
- ui-mockup now auto-updates creative-brief.md when user finalizes
- plugin-workflow no longer validates drift, just syncs brief if needed
- Mockup is now source of truth for UI decisions (no manual reconciliation)

Rationale: UI iteration through v1, v2, v3 is normal workflow, not drift.
Treating design evolution as error created unnecessary friction. Auto-updating
brief from finalized mockup removes decision gates and streamlines flow.

Applies to: New plugins only (existing plugins already reconciled)"
```

### Backward Compatibility

**Good news: No concerns**

**Why:**
- System state is per-plugin (each plugin has own .continue-here.md)
- Existing plugins already reconciled (if they went through workflow)
- New plugins start fresh (never knew design-sync existed)
- Cached validation results have TTL (expire automatically after 24hrs)
- No persistent state tracking design-sync runs

**For existing plugins:**
- Already completed workflow: No impact (won't re-run design-sync)
- Mid-workflow (paused at Stage 2+): Will skip sync check (brief already exists)
- Just finished mockup (waiting at Stage 1): First to use new auto-update flow

**Migration not needed** - removal is forward-compatible by design.

---

## Implementation Roadmap

### Phase 1: Preparation (30 minutes)

**1.1 Create Implementation Script**
- Write bash script to auto-update creative brief from mockup
- Location: `.claude/utils/sync-brief-from-mockup.sh`
- Inputs: plugin name, mockup version
- Outputs: updated creative-brief.md
- Test with Drum808 (mockup v2 exists, brief exists)

**1.2 Document Preserved Sections**
- Create section identifier map:
  ```json
  {
    "preserve": ["## Overview", "## Vision", "## Use Cases", "## Inspirations", "## Technical Notes"],
    "update": ["## Parameters", "## UI Concept"],
    "auto_generate": ["## Next Steps"]
  }
  ```
- Store in script or separate config

**1.3 Test Section Extraction**
- Parse Drum808 creative brief
- Verify extraction preserves formatting
- Verify reconstruction maintains structure

### Phase 2: Update ui-mockup (45 minutes)

**2.1 Rewrite Phase 5.6**
- Replace design-sync invocation with brief update logic
- Call sync-brief-from-mockup.sh script
- Add confirmation display
- Test error handling (what if brief parse fails?)

**2.2 Update Phase 5.5 Routing**
- Modify option 2 routing (Finalize)
- Remove "validate alignment" language
- Add "auto-update brief" language

**2.3 Update References**
- Modify `references/decision-menus.md` option 2 description
- Update any phase diagrams or workflow docs

**2.4 Test with Drum808**
- Run ui-mockup through finalization
- Verify brief updated correctly
- Check preserved sections intact
- Verify commit created

### Phase 3: Update plugin-workflow (30 minutes)

**3.1 Replace Stage 1→2 Gate**
- Remove `<design_sync_gate>` block
- Add `<creative_brief_sync>` block (automatic, no menu)
- Call sync-brief-from-mockup.sh if needed

**3.2 Add State Tracking**
- Update .continue-here.md format to include:
  ```yaml
  brief_updated_from_mockup: true
  mockup_version_synced: 2
  brief_update_timestamp: "2025-11-13T10:30:00Z"
  ```
- Check flag before running sync (skip if already done)

**3.3 Test Stage 2 Entry**
- Mock scenario: mockup finalized but brief not yet updated
- Verify automatic sync runs
- Verify Stage 2 proceeds without gates

### Phase 4: Remove design-sync (15 minutes)

**4.1 Verify No Dependencies**
- Grep for all references to design-sync
- Confirm all integration points updated
- Check no hooks reference it

**4.2 Remove Skill**
- Move to .REMOVED backup
- Remove from filesystem

**4.3 Remove Command**
- Move to .REMOVED backup
- Remove from filesystem

**4.4 Update Documentation**
- Remove from CLAUDE.md skills list
- Update proactive validation section
- Update workflow overview if applicable

### Phase 5: Testing (30 minutes)

**5.1 End-to-End Test: New Plugin**
- Create creative brief for test plugin
- Run ui-mockup through finalization
- Verify brief auto-updates
- Continue to Stage 2
- Verify no validation gates appear
- Confirm workflow proceeds smoothly

**5.2 Edge Case: Standalone Mockup**
- Run ui-mockup without creative brief
- Verify Phase 5.6 skips update (no brief exists)
- Confirm no errors

**5.3 Edge Case: Mid-Workflow Resume**
- Pause at Stage 1 (post-mockup, pre-Stage 2)
- Resume with /continue
- Verify automatic sync at Stage 2 entry
- Confirm state tracking works

**5.4 Rollback Test**
- Verify backups exist (.REMOVED directories)
- Document rollback procedure if issues found

### Phase 6: Commit and Deploy (15 minutes)

**6.1 Final Verification**
- All tests pass
- No broken references
- Documentation updated

**6.2 Commit with BREAKING CHANGE**
- Use conventional commit format
- Explain rationale in commit body
- Note forward-compatibility (no migration needed)

**6.3 Update TO-DOS.md if Applicable**
- Mark design-sync removal complete
- Add follow-up tasks if identified during implementation

### Total Estimated Time: 2.5 hours

**Breakdown:**
- Preparation: 30 min
- ui-mockup updates: 45 min
- plugin-workflow updates: 30 min
- Removal: 15 min
- Testing: 30 min
- Commit/deploy: 15 min

**Risk areas:**
- Brief parsing edge cases (malformed markdown, missing sections)
- YAML parsing failures (invalid mockup files)
- State tracking bugs (flag not set correctly)
- Incomplete removal (missed references)

**Mitigation:**
- Extensive error handling in sync script
- Fallback to minimal update if parsing fails
- Grep verification before removal
- Backup before deletion (.REMOVED copies)

---

## Verification Plan

### Pre-Deployment Checks

**1. Reference Verification**
```bash
# Search for all design-sync references
grep -r "design-sync\|sync-design" .claude/ \
  --exclude-dir=.git \
  --exclude-dir=node_modules \
  --exclude="*.REMOVED*" \
  | tee design-sync-references.txt

# Expected: Zero results (all references removed or updated)
```

**2. Integration Point Verification**

**ui-mockup Phase 5.6:**
- [ ] No longer invokes design-sync via Skill tool
- [ ] Calls sync-brief-from-mockup.sh instead
- [ ] Displays confirmation message
- [ ] Updates .continue-here.md state
- [ ] Commits changes

**plugin-workflow Stage 1→2:**
- [ ] No longer has `<design_sync_gate>` block
- [ ] Has `<creative_brief_sync>` block instead
- [ ] Checks .continue-here.md flag before syncing
- [ ] Automatic (no user menus)
- [ ] Proceeds to Stage 2 without blocking

**3. File Verification**
```bash
# Verify skill removed
test ! -d .claude/skills/design-sync && echo "✓ Skill removed" || echo "✗ Skill still exists"

# Verify command removed
test ! -f .claude/commands/sync-design.md && echo "✓ Command removed" || echo "✗ Command still exists"

# Verify backups created
test -d .claude/skills/design-sync.REMOVED-* && echo "✓ Skill backup exists" || echo "✗ No backup"
test -f .claude/commands/sync-design.md.REMOVED-* && echo "✓ Command backup exists" || echo "✗ No backup"
```

### Post-Deployment Testing

**Test 1: New Plugin from Scratch**

```bash
# 1. Create creative brief
/dream TestPlugin01
# ... ideation flow ...

# 2. Create UI mockup
/dream TestPlugin01 → option 3 (UI mockup)
# ... design iteration ...
# Choose "Finalize" in Phase 5.5

# 3. Verify Phase 5.6 behavior
# Expected:
# - "✓ Creative brief updated from mockup v1"
# - Confirmation display shows updated sections
# - No validation menus
# - Proceeds to Phase 6-10 automatically
```

**Verification steps:**
- [ ] Phase 5.6 runs without errors
- [ ] creative-brief.md Parameters section updated from parameter-spec.md
- [ ] creative-brief.md UI Concept section updated from mockup YAML
- [ ] Vision, Use Cases, Inspirations sections unchanged
- [ ] .continue-here.md has brief_updated_from_mockup: true
- [ ] Git commit created with sync message

**Test 2: Resume at Stage 2 Entry**

```bash
# 1. Manually edit mockup YAML to change dimensions
# (Simulate: user finalized mockup but didn't run implementation yet)

# 2. Resume workflow
/continue TestPlugin01

# 3. Verify Stage 1→2 behavior
# Expected:
# - Automatic sync runs (detects brief out of sync with mockup)
# - "✓ Creative brief updated from mockup v1"
# - No validation menus
# - Proceeds to Stage 2 dispatch
```

**Verification steps:**
- [ ] Automatic sync triggered
- [ ] creative-brief.md updated with new dimensions
- [ ] .continue-here.md flag set
- [ ] Stage 2 proceeds without blocking

**Test 3: Standalone Mockup (No Brief)**

```bash
# 1. Create mockup without creative brief
# (Simulate: user exploring UI ideas before committing to concept)

# Skip creative brief creation
# Run ui-mockup directly

# 2. Finalize mockup
# Choose "Finalize" in Phase 5.5

# 3. Verify Phase 5.6 behavior
# Expected:
# - "Standalone mode: No creative brief to update"
# - Skips sync step
# - Proceeds to Phase 6-10 normally
```

**Verification steps:**
- [ ] No errors thrown
- [ ] Phase 6-10 proceeds
- [ ] parameter-spec.md generated normally
- [ ] No creative-brief.md created (standalone mode)

**Test 4: Edge Case - Malformed Brief**

```bash
# 1. Manually corrupt creative-brief.md
# (Remove ## Vision section header, add invalid YAML frontmatter, etc.)

# 2. Finalize mockup
# Choose "Finalize" in Phase 5.5

# 3. Verify Phase 5.6 error handling
# Expected:
# - Parse error detected
# - Error menu presented:
#   1. Retry with fallback (minimal update)
#   2. Manual fix (pause workflow)
#   3. Skip update (risky, continue anyway)
```

**Verification steps:**
- [ ] Error caught gracefully
- [ ] Menu presented (not crash)
- [ ] Fallback option works (minimal update: dimensions only)
- [ ] Manual fix option pauses workflow cleanly

**Test 5: Edge Case - Missing YAML Metadata**

```bash
# 1. Manually edit mockup YAML to remove design_system section
# (Simulate: old mockup format or corrupted file)

# 2. Finalize mockup

# 3. Verify Phase 5.6 fallback
# Expected:
# - Missing metadata detected
# - Fallback update runs (minimal info: dimensions, parameter count)
# - Warning displayed but workflow continues
```

**Verification steps:**
- [ ] Fallback logic triggered
- [ ] creative-brief.md updated with available info
- [ ] Missing metadata noted in UI Concept section
- [ ] Workflow continues (not blocked)

### Success Criteria

**All tests pass when:**

1. **Functional Requirements:**
   - [ ] creative-brief.md auto-updates when mockup finalized
   - [ ] Conceptual sections (Vision, Use Cases, etc.) preserved verbatim
   - [ ] UI sections (Parameters, UI Concept) updated from mockup
   - [ ] No validation gates appear (no drift warnings)
   - [ ] Workflow proceeds smoothly from finalization to Stage 2

2. **Integration Requirements:**
   - [ ] ui-mockup Phase 5.6 works as documented
   - [ ] plugin-workflow Stage 1→2 sync works as documented
   - [ ] State tracking (.continue-here.md flags) works correctly
   - [ ] Git commits created at appropriate times

3. **Error Handling:**
   - [ ] Malformed briefs handled gracefully (fallback or menu)
   - [ ] Missing YAML metadata handled (minimal update)
   - [ ] Standalone mode (no brief) works without errors
   - [ ] All edge cases tested and verified

4. **Cleanup Verification:**
   - [ ] design-sync skill removed from filesystem
   - [ ] /sync-design command removed
   - [ ] No references to design-sync in active codebase
   - [ ] Backups created (.REMOVED directories)
   - [ ] Documentation updated (CLAUDE.md, integration docs)

5. **Rollback Capability:**
   - [ ] Backups exist and are valid
   - [ ] Rollback procedure documented (restore from .REMOVED)
   - [ ] Quick rollback tested (restore skill + command, revert workflow changes)

**Final Sign-Off:**
- [ ] All tests pass
- [ ] Code review complete (self-review or pair)
- [ ] Documentation accurate
- [ ] Commit message clear (BREAKING CHANGE noted)
- [ ] No known issues or regressions

---

## Summary

**Current System:**
- design-sync validates mockup ↔ brief alignment
- Detects drift (parameter mismatches, style divergence)
- Requires manual reconciliation (update brief or mockup)
- Creates friction during normal design iteration

**New System:**
- ui-mockup auto-updates brief when user finalizes
- Preserves conceptual content (Vision, Use Cases)
- Updates UI content (Parameters, UI Concept) from mockup
- Treats mockup as source of truth (no validation needed)
- Removes all decision gates related to drift

**Impact:**
- Streamlines workflow (one less gate)
- Removes "drift" concept (iteration is normal, not exceptional)
- Maintains contract alignment (brief always reflects final design)
- No backward compatibility issues (forward-only change)

**Implementation Effort:**
- 2.5 hours estimated (preparation through deployment)
- Low risk (clear integration points, well-tested removal strategy)
- High value (removes friction, improves UX)

**Next Steps:**
1. Review this analysis
2. Approve approach
3. Implement Phase 1 (preparation script)
4. Test with Drum808
5. Proceed through roadmap phases
6. Deploy and verify
