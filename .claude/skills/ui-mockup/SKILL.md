---
name: ui-mockup
description: Generate production-ready WebView UI mockups in two phases - design iteration (2 files) then implementation scaffolding (5 files after approval)
allowed-tools:
  - Read
  - Write
preconditions:
  - None (can work standalone or with creative brief)
---

# ui-mockup Skill

**Purpose:** Generate production-ready WebView UIs in two phases. The HTML generated IS the plugin UI, not a throwaway prototype.

## Workflow Overview

**TWO-PHASE WORKFLOW:**

### Phase A: Design Iteration (Fast)
Generate 2 design files for rapid iteration:
1. **v[N]-ui.yaml** - Machine-readable design specification
2. **v[N]-ui-test.html** - Browser-testable mockup (no JUCE required)

**STOP HERE** - Present decision menu for user to iterate or finalize.

### Phase B: Implementation Scaffolding (After Finalization)
Generate 5 implementation files only after user approves design:
3. **v[N]-ui.html** - Production HTML (copy-paste to plugin)
4. **v[N]-PluginEditor.h** - C++ header boilerplate
5. **v[N]-PluginEditor.cpp** - C++ implementation boilerplate
6. **v[N]-CMakeLists.txt** - WebView build configuration
7. **v[N]-integration-checklist.md** - Implementation steps

**Why two phases?** HTML mockups are cheap to iterate. C++ boilerplate is pointless if design isn't locked. This saves time by avoiding premature scaffolding generation.

All files saved to: `plugins/[PluginName]/.ideas/mockups/`

## Phase 0: Check for Aesthetic Library

**Before starting design, check if saved aesthetics exist.**

```bash
if [ -f .claude/aesthetics/manifest.json ]; then
    AESTHETIC_COUNT=$(jq '.aesthetics | length' .claude/aesthetics/manifest.json)
    if [ $AESTHETIC_COUNT -gt 0 ]; then
        echo "Found $AESTHETIC_COUNT saved aesthetics"
    fi
fi
```

**If aesthetics exist, present decision menu:**

```
Found $AESTHETIC_COUNT saved aesthetics in library.

How would you like to start the UI design?
1. Start from aesthetic template - Apply saved visual system
2. Start from scratch - Create custom design
3. List all aesthetics - Browse library before deciding

Choose (1-3): _
```

**Option handling:**

- **Option 1: Start from aesthetic template**
  - Read manifest: `.claude/aesthetics/manifest.json`
  - Display available aesthetics with metadata:
    ```
    Available aesthetics:

    1. Vintage Hardware (vintage-hardware-001)
       Vibe: Vintage analog
       Colors: Orange/cream/brown with paper texture
       From: TapeAge mockup v2

    2. Modern Minimal Blue (modern-minimal-002)
       Vibe: Clean, modern
       Colors: Blue/gray/white with subtle shadows
       From: EQ4Band mockup v1

    4. None (start from scratch)

    Choose aesthetic: _
    ```
  - If user selects aesthetic: Invoke ui-template-library skill with "apply" operation
  - Skip to Phase 4 with generated mockup from aesthetic

- **Option 2: Start from scratch**
  - Continue to Phase 1 (load context)

- **Option 3: List all aesthetics**
  - Invoke ui-template-library skill with "list" operation
  - Show preview paths
  - Return to option menu

**If no aesthetics exist:**
- Skip Phase 0
- Continue directly to Phase 1

**See:** `references/aesthetic-integration.md` for complete integration details

---

## Phase 1: Load Context from Creative Brief

**CRITICAL: Always read creative-brief.md before starting.**

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/creative-brief.md"
```

**Extract UI context from creative-brief.md:**

- **UI Concept section:** Layout preferences, visual style mentions
- **Parameters:** Count and types (determines control layout)
- **Plugin type:** Effect/synth/utility (affects typical layouts)
- **Vision section:** Any visual references or inspirations

**See:** `references/context-extraction.md` for detailed extraction guidelines

## Phase 1.5: Context-Aware Initial Prompt

**Adapt the prompt based on what's in the creative brief:**

**If rich UI details exist:**
```
I see you want [extracted description from UI Concept] for [PluginName]. Let's refine that vision. Tell me more about the layout, control arrangement, and visual elements you're imagining.
```

**If minimal UI details:**
```
Let's design the UI for [PluginName]. You mentioned it's a [type] with [X] parameters. What layout and style are you envisioning?
```

**If zero UI context:**
```
Let's design the UI for [PluginName]. What do you envision? (layout, style, controls, visual elements)
```

**Why context-aware:** Don't ask the user to repeat information they already provided in the creative brief. Build on what they said.

**Listen for:**

- Layout preferences ("controls on left, visualizer on right")
- Visual references ("like FabFilter Pro-C", "vintage analog gear")
- Mood/feel ("minimal and clean", "skeuomorphic wood panels")
- Special requests ("animated VU meter", "resizable window")

**Capture verbatim notes before moving to targeted questions.**

## Phase 2: Gap Analysis and Question Prioritization

**Question Priority Tiers:**

- **Tier 1 (Critical):** Layout structure, control types
- **Tier 2 (Visual):** Visual style, key visual elements (meters, waveforms, displays)
- **Tier 3 (Polish):** Colors, typography, animations, interactive features

**Extract from Phase 1.5 response and creative brief, then identify gaps:**

1. Parse user's UI description
2. Check which tiers are covered
3. Identify missing critical/visual information
4. Never ask about already-provided information

**Example of context-aware extraction:**

```
Creative brief UI Concept:
"Vintage aesthetic with three knobs"

Phase 1.5 user response:
"I want the knobs arranged horizontally, with a tape reel animation in the background"

Extracted:
- Layout: horizontal arrangement ✓
- Control type: knobs ✓
- Visual style: vintage ✓
- Special element: tape reel animation ✓

Gaps identified:
- Window size? (Tier 1)
- Vintage style details? (Tier 2)
- Knob style (large vintage vs small precise)? (Tier 2)
- VU meters or other feedback? (Tier 2)
```

## Phase 3: Question Batch Generation

**Generate exactly 4 questions using AskUserQuestion based on identified gaps.**

**Rules:**
- If 4+ gaps exist: ask top 4 by tier priority
- If fewer gaps exist: pad with "nice to have" tier 3 questions
- Provide meaningful options (not just open text prompts)
- Always include "Other" option for custom input
- Users can skip questions via "Other" option and typing "skip"

**Example question batch (via AskUserQuestion):**

```
Question 1:
  question: "Layout structure for the three knobs?"
  header: "Layout"
  options:
    - label: "Horizontal row of knobs", description: "Classic single-row layout"
    - label: "Vertical stack", description: "Narrow, tall layout"
    - label: "Triangle arrangement", description: "Two bottom, one top"
    - label: "Other", description: "Custom arrangement"

Question 2:
  question: "Vintage style details?"
  header: "Aesthetic"
  options:
    - label: "Brushed metal", description: "Industrial, professional"
    - label: "Wood panel", description: "Warm, classic studio"
    - label: "Reel-to-reel theme", description: "Tape machine aesthetic"
    - label: "Other", description: "Different vintage style"

Question 3:
  question: "Visual feedback elements?"
  header: "Meters/displays"
  options:
    - label: "VU meters", description: "Needle-style level indicators"
    - label: "Tape reel animation", description: "Spinning reel visual"
    - label: "Both VU and reels", description: "Full tape machine feel"
    - label: "None (controls only)", description: "Clean, minimal"

Question 4:
  question: "Knob style?"
  header: "Controls"
  options:
    - label: "Large vintage knobs", description: "60s-70s style, prominent"
    - label: "Small precise knobs", description: "Modern, technical"
    - label: "Chicken-head style", description: "Classic pointer knobs"
    - label: "Other", description: "Different knob style"
```

**After receiving answers:**
1. Accumulate context with previous responses
2. Re-analyze gaps
3. Proceed to decision gate

### Example: Context-Aware UI Questions

```
Creative brief UI Concept:
"Vintage aesthetic with three knobs"

Phase 1.5 user response:
"I want the knobs arranged horizontally, with a tape reel animation in the background"

Extracted:
- Layout: horizontal arrangement ✓
- Control type: knobs ✓
- Visual style: vintage ✓
- Special element: tape reel animation ✓

Gaps identified (4 needed):
- Layout structure specifics? (Tier 1)
- Vintage style details? (Tier 2)
- Knob style? (Tier 2)
- VU meters or other feedback? (Tier 2)

Question Batch 1 (via AskUserQuestion):
1. "Layout structure for the three knobs?" → [Horizontal row, Vertical stack, Triangle arrangement, Other]
2. "Vintage style details?" → [Brushed metal, Wood panel, Reel-to-reel theme, Other]
3. "Visual feedback elements?" → [VU meters, Tape reel animation, Both, None]
4. "Knob style?" → [Large vintage knobs, Small precise knobs, Chicken-head style, Other]

[Then decision gate]

If user chooses "Ask me 4 more questions":
- User answered: "Horizontal row", "Reel-to-reel theme", "Both VU and reels", "Large vintage knobs"

Updated context:
- Layout: horizontal row of knobs ✓
- Visual style: reel-to-reel theme ✓
- Controls: large vintage knobs ✓
- Visual feedback: VU meters + tape reel animation ✓

New gaps for Batch 2:
- Window size? (Tier 1)
- Color scheme? (Tier 3)
- Reel animation behavior? (Tier 3)
- VU meter style? (Tier 3)

Question Batch 2:
1. "Window size?" → [Compact 500x300, Standard 600x400, Large 800x500, Other]
2. "Color scheme?" → [Warm sepia/brown, Cool metal/gray, Classic black/orange, Other]
3. "Reel animation?" → [Speed matches tempo, Speed matches delay time, Always spinning, Other]
4. "VU meter style?" → [Needle style, LED bars, Classic backlit, Other]

[Then decision gate again]
```

## Phase 3.5: Decision Gate

**Use AskUserQuestion with 3 options after each question batch:**

```
Question:
  question: "Ready to finalize the mockup design?"
  header: "Next step"
  options:
    - label: "Yes, finalize it", description: "Generate YAML and test HTML"
    - label: "Ask me 4 more questions", description: "Continue refining"
    - label: "Let me add more context first", description: "Provide additional details"

Route based on answer:
- Option 1 → Proceed to Phase 4 (generate YAML and test HTML)
- Option 2 → Return to Phase 2 (re-analyze gaps, generate next 4 questions)
- Option 3 → Collect free-form text, merge with context, return to Phase 2
```

## Phase 4: Generate Hierarchical YAML

**Create:** `plugins/[Name]/.ideas/mockups/v[N]-ui.yaml`

**Purpose:** Machine-readable design spec that guides HTML generation and C++ implementation.

**Structure:**

```yaml
window:
  width: 600
  height: 400
  resizable: false

colors:
  background: "#2b2b2b"
  primary: "#4a9eff"
  text: "#ffffff"

layout:
  type: grid  # or: vertical-sections, horizontal-sections, custom
  sections:
    - id: header
      height: 60
      controls: [title]
    - id: main
      flex: 1
      controls: [threshold, ratio, attack, release, gain]

controls:
  - id: threshold
    type: rotary-knob
    label: "Threshold"
    parameter: "threshold"
    range: [-60.0, 0.0]
    unit: "dB"
    default: -20.0
    position: {x: 50, y: 100}
```

**See:** `assets/ui-yaml-template.yaml` for complete template structure

## Phase 5: Generate Browser Test HTML

**Create:** `plugins/[Name]/.ideas/mockups/v[N]-ui-test.html`

**Purpose:** Test UI in browser for rapid design iteration.

**Features:**

- Standalone HTML file (open directly in browser)
- Mock parameter state (simulates C++ backend)
- Interactive controls (test bindings)
- Console logging (verify parameter messages)
- Same visual as production will be
- No JUCE/WebView required

**See:** `references/browser-testing.md` for testing guidelines

## Phase 5.3: Validate WebView Constraints (Before Decision Menu)

**CRITICAL:** Validate generated HTML against WebView constraints before presenting to user.

**Validation checklist:**

```bash
# Check for forbidden CSS viewport units
! grep -q "100vh\|100vw\|100dvh\|100svh" v[N]-ui-test.html

# Check for required html/body height
grep -q "html, body.*height: 100%" v[N]-ui-test.html

# Check for native feel CSS
grep -q "user-select: none" v[N]-ui-test.html

# Check for context menu disabled
grep -q 'contextmenu.*preventDefault' v[N]-ui-test.html
```

**If validation fails:**
- ❌ REJECT: Regenerate mockup with corrections
- Do NOT present to user until constraints are satisfied

**See:** `references/ui-design-rules.md` for complete validation rules

## Phase 5.4: Auto-Open in Browser

**After validation passes, automatically open the test HTML in browser.**

```bash
open plugins/[PluginName]/.ideas/mockups/v[N]-ui-test.html
```

This allows immediate visual inspection without requiring user to manually navigate and open the file.

**Note:** Uses `open` command (macOS). On other platforms, adjust command accordingly (e.g., `xdg-open` on Linux, `start` on Windows).

## Phase 5.45: Version Control Checkpoint

**CRITICAL: Commit each UI version immediately after generation.**

**After Phase 5.4 completes (YAML + test HTML generated and validated):**

```bash
cd plugins/[PluginName]/.ideas/mockups
git add v[N]-ui.yaml v[N]-ui-test.html
git commit -m "feat([PluginName]): UI mockup v[N] (design iteration)"
```

**Why commit at this point:**
- Preserves design history between iterations
- Each version is recoverable
- Enables A/B comparison of different designs
- Atomic commits per iteration (not batched)

**Update workflow state (if in workflow context):**

```bash
if [ -f "plugins/[PluginName]/.ideas/.continue-here.md" ]; then
    # Update version tracking
    sed -i '' "s/latest_mockup_version: .*/latest_mockup_version: [N]/" .continue-here.md
    # Keep mockup_finalized: false until user chooses "finalize"
    git add .continue-here.md
    git commit --amend --no-edit
fi
```

**State tracking in `.continue-here.md`:**

```markdown
current_stage: 0
stage_0_status: ui_design_in_progress
latest_mockup_version: 2
mockup_finalized: false
```

**Only proceed to Phase 5.5 after successful commit.**

---

## ⚠️ CRITICAL STOP POINT - Phase 5.5: Design Decision Menu

**DO NOT PROCEED TO PHASE 6 WITHOUT USER CONFIRMATION**

After generating YAML + test HTML, present this decision menu:

```
✓ Mockup v[N] design created (2 files)

Files generated:
- v[N]-ui.yaml (design specification)
- v[N]-ui-test.html (browser-testable mockup)

What do you think?
1. Check alignment - Run design-sync validation (recommended before finalizing)
2. Provide refinements (iterate on design) ← Creates v[N+1]
3. Finalize and create implementation files (if satisfied and aligned)
4. Save as aesthetic template (add to library for reuse)
5. Finalize AND save aesthetic (do both operations)
6. Test in browser (open v[N]-ui-test.html)
7. Validate WebView constraints (run checks)
8. Other

Choose (1-8): _
```

**WAIT for user response before continuing.**

**Option handling:**
- **Option 1**: Check alignment → Invoke design-sync skill to validate mockup ↔ creative brief consistency
  - Detects drift (parameter mismatches, missing features, style divergence)
  - User resolves any issues within design-sync
  - **design-sync routes back to Phase 5.5 decision menu after completion** (see design-sync Step 7)
- **Option 2**: User gives feedback → Return to Phase 2 with new version number (v2, v3, etc.)
- **Option 3**: User approves → Proceed to Phase 6-10 (generate remaining 5 files + finalize state)
- **Option 4**: Save aesthetic → Invoke ui-template-library skill with "save" operation
  ```
  Invoke Skill tool:
  - skill: "ui-template-library"
  - prompt: "Save aesthetic from plugins/[PluginName]/.ideas/mockups/v[N]-ui.html"
  ```
  After saving, return to decision menu
- **Option 5**: Save aesthetic first, then proceed to Phase 6-10
  ```
  1. Invoke ui-template-library "save" operation
  2. Wait for confirmation
  3. Proceed to Phase 6-10 (generate implementation files)
  ```
- **Option 6**: Offer to open test HTML in browser for interactive review
- **Option 7**: Validate WebView constraints (run Phase 5.3 checks again)
- **Option 8**: Other

**Only execute Phases 6-10 if user chose option 3 or 5 (finalize).**

---

## Phase 6: Generate Production HTML (After Finalization Only)

**Prerequisites:** User confirmed design in Phase 5.5 decision menu.

**Create:** `plugins/[Name]/.ideas/mockups/v[N]-ui.html`

**This HTML IS the plugin UI.** It will be copied to `Source/ui/public/index.html` during Stage 5 (GUI).

### Generation Strategy

**Base template:** `assets/webview-templates/index-template.html`

**Key replacements:**

1. **{{PLUGIN_NAME}}** → Plugin name from creative brief
2. **{{CONTROL_HTML}}** → Generate controls from finalized YAML/HTML
3. **{{PARAMETER_BINDINGS}}** → Generate JavaScript bindings for each parameter

### Parameter ID Extraction

Parse finalized HTML from Phase 5 for JUCE parameter bindings:

```javascript
// Extract parameter IDs from JavaScript code patterns
const parameterIds = [];

// Pattern 1: Juce.getSliderState("PARAM_ID")
const sliderMatches = html.matchAll(/Juce\.getSliderState\("([^"]+)"\)/g);
for (const match of sliderMatches) {
    parameterIds.push({ id: match[1], type: "slider" });
}

// Pattern 2: Juce.getToggleButtonState("PARAM_ID")
const toggleMatches = html.matchAll(/Juce\.getToggleButtonState\("([^"]+)"\)/g);
for (const match of toggleMatches) {
    parameterIds.push({ id: match[1], type: "toggle" });
}

// Pattern 3: Juce.getComboBoxState("PARAM_ID")
const comboMatches = html.matchAll(/Juce\.getComboBoxState\("([^"]+)"\)/g);
for (const match of comboMatches) {
    parameterIds.push({ id: match[1], type: "combo" });
}
```

**Use extracted IDs to generate matching relay/attachment code in C++.**

### Critical Constraints

**Enforce from `references/ui-design-rules.md`:**

- ❌ NO viewport units: `100vh`, `100vw`, `100dvh`, `100svh`
- ✅ REQUIRED: `html, body { height: 100%; }`
- ✅ REQUIRED: `user-select: none` (native feel)
- ✅ REQUIRED: Context menu disabled in JavaScript

**See:** `references/ui-design-rules.md` for complete constraints

## Phase 7: Generate C++ Boilerplate (After Finalization Only)

**Prerequisites:** User confirmed design in Phase 5.5 decision menu.

**Create:**
- `plugins/[Name]/.ideas/mockups/v[N]-PluginEditor.h`
- `plugins/[Name]/.ideas/mockups/v[N]-PluginEditor.cpp`

**Purpose:** WebView integration boilerplate for Stage 5 (GUI).

### File 1: v[N]-PluginEditor.h (C++ Header)

**Base template:** `assets/webview-templates/PluginEditor-webview.h`

**Key replacements:**

**{{RELAY_DECLARATIONS}}** → Generate relay declarations for each parameter:

```cpp
// For each parameter from Phase 6 extraction:
// - Slider/Knob → std::unique_ptr<juce::WebSliderRelay> [name]Relay;
// - Toggle      → std::unique_ptr<juce::WebToggleButtonRelay> [name]Relay;
// - Dropdown    → std::unique_ptr<juce::WebComboBoxRelay> [name]Relay;

// Example:
std::unique_ptr<juce::WebSliderRelay> gainRelay;
std::unique_ptr<juce::WebSliderRelay> toneRelay;
std::unique_ptr<juce::WebToggleButtonRelay> bypassRelay;
std::unique_ptr<juce::WebComboBoxRelay> modeRelay;
```

**{{ATTACHMENT_DECLARATIONS}}** → Generate attachment declarations matching relay types:

```cpp
// Match relay type for each parameter:
std::unique_ptr<juce::WebSliderParameterAttachment> gainAttachment;
std::unique_ptr<juce::WebSliderParameterAttachment> toneAttachment;
std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassAttachment;
std::unique_ptr<juce::WebComboBoxParameterAttachment> modeAttachment;
```

**⚠️ CRITICAL:** Enforce correct member order (relays → webView → attachments) to prevent release build crashes.

### File 2: v[N]-PluginEditor.cpp (C++ Implementation)

**Base template:** `assets/webview-templates/PluginEditor-webview.cpp`

**Key replacements:**

**{{RELAY_CREATION}}** → Generate relay creation code (before WebView):

```cpp
// For each parameter (use parameter IDs from Phase 6 extraction):
gainRelay = std::make_unique<juce::WebSliderRelay>("GAIN");
toneRelay = std::make_unique<juce::WebSliderRelay>("TONE");
bypassRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS");
modeRelay = std::make_unique<juce::WebComboBoxRelay>("MODE");
```

**{{WEBVIEW_OPTIONS}}** → Generate WebView options registration:

```cpp
// For each relay:
.withOptionsFrom(*gainRelay)
.withOptionsFrom(*toneRelay)
.withOptionsFrom(*bypassRelay)
.withOptionsFrom(*modeRelay)
```

**{{ATTACHMENT_CREATION}}** → Generate attachment creation code (after WebView):

```cpp
// For each parameter:
gainAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
    *audioProcessor.apvts.getParameter("GAIN"),
    *gainRelay,
    nullptr  // No undo manager
);
toneAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
    *audioProcessor.apvts.getParameter("TONE"),
    *toneRelay,
    nullptr
);
bypassAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
    *audioProcessor.apvts.getParameter("BYPASS"),
    *bypassRelay,
    nullptr
);
modeAttachment = std::make_unique<juce::WebComboBoxParameterAttachment>(
    *audioProcessor.apvts.getParameter("MODE"),
    *modeRelay,
    nullptr
);
```

**{{RESOURCE_MAPPING}}** → Generate resource provider mappings for all UI files:

```cpp
if (url == "/" || url == "/index.html") {
    return juce::WebBrowserComponent::Resource {
        BinaryData::index_html,
        BinaryData::index_htmlSize,
        "text/html"
    };
}

if (url == "/js/juce/index.js") {
    return juce::WebBrowserComponent::Resource {
        BinaryData::juce_index_js,
        BinaryData::juce_index_jsSize,
        "text/javascript"
    };
}

// Add more resources as needed (CSS, images, etc.)
```

**{{DETERMINE_SIZE_FROM_MOCKUP}}** → Extract window size from YAML or HTML:

```cpp
setSize(600, 400);  // From YAML window.width/height
setResizable(false, false);  // From YAML window.resizable
```

**See:** `assets/webview-templates/` for complete template files

## Phase 8: Generate Build Configuration (After Finalization Only)

**Prerequisites:** User confirmed design in Phase 5.5 decision menu.

**Create:** `plugins/[Name]/.ideas/mockups/v[N]-CMakeLists.txt`

**Purpose:** CMake snippet to enable WebView support in JUCE.

**IMPORTANT:** This is a SNIPPET to append to existing plugin CMakeLists.txt, NOT a complete CMakeLists.txt file.

### Generation Strategy

**Base template:** `assets/webview-templates/CMakeLists-webview-snippet.cmake`

**Critical constraints:**

- ❌ DO NOT include `project()` declaration (handled by root CMakeLists.txt)
- ❌ DO NOT include `add_subdirectory(JUCE)` (JUCE added at root level)
- ✅ This snippet should be APPENDED to the plugin's existing CMakeLists.txt
- ✅ Plugin CMakeLists.txt should already have `juce_add_plugin()` and basic configuration

**Key customizations:**

1. **List all resources from ui/public/ directory:**

```cmake
juce_add_binary_data(${PLUGIN_NAME}_UIResources
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/juce/index.js
        # Add any additional CSS, images, fonts discovered in HTML
)
```

2. **Platform-specific configuration:**

- macOS: No additional config (uses WebKit built-in)
- Windows: Include WebView2 options (if cross-platform)
- Linux: Include webkit2gtk dependency (if cross-platform)

**See:** `assets/webview-templates/CMakeLists-webview-snippet.cmake` for complete template

## Phase 9: Generate Integration Checklist (After Finalization Only)

**Prerequisites:** User confirmed design in Phase 5.5 decision menu.

**Create:** `plugins/[Name]/.ideas/mockups/v[N]-integration-checklist.md`

**Purpose:** Step-by-step guide to integrate UI into plugin during Stage 5.

### Checklist Structure

**Base template:** `assets/integration-checklist-template.md`

**WebView-specific steps:**

```markdown
## Stage 5 (GUI) Integration Steps

### 1. Copy UI Files
- [ ] Copy v[N]-ui.html to Source/ui/public/index.html
- [ ] Copy JUCE frontend library to Source/ui/public/js/juce/index.js
- [ ] Copy any additional resources (CSS, images, etc.)

### 2. Update PluginEditor Files
- [ ] Replace PluginEditor.h with v[N]-PluginEditor.h
- [ ] Verify member order: relays → webView → attachments
- [ ] Replace PluginEditor.cpp with v[N]-PluginEditor.cpp
- [ ] Verify initialization order matches member order

### 3. Update CMakeLists.txt
- [ ] Add juce_add_binary_data for UI resources
- [ ] Link ${PLUGIN_NAME}_UIResources to plugin
- [ ] Add JUCE_WEB_BROWSER=1 definition
- [ ] Add platform-specific config (if cross-platform)

### 4. Build and Test (Debug)
- [ ] Build succeeds without warnings
- [ ] Standalone loads WebView (not blank)
- [ ] Right-click → Inspect works
- [ ] Console shows no JavaScript errors
- [ ] window.__JUCE__ object exists
- [ ] Parameter state objects accessible

### 5. Build and Test (Release)
- [ ] Release build succeeds without warnings
- [ ] Release build runs (tests member order logic)
- [ ] No crashes on plugin reload (test 10 times)

### 6. Test Parameter Binding
- [ ] Moving UI slider changes audio (verify in DAW)
- [ ] Changing parameter in DAW updates UI
- [ ] Parameter values persist after reload
- [ ] Multiple parameters sync independently

### 7. WebView-Specific Validation
- [ ] Verify member order in PluginEditor.h (relays → webView → attachments)
- [ ] Test resource provider returns all files (no 404 in console)
- [ ] Verify parameter binding (automation/preset recall)
- [ ] Test in Debug and Release builds
- [ ] Check for crashes on plugin close (reload 10 times)
- [ ] CSS does NOT use viewport units (100vh, 100vw)
- [ ] Native feel CSS present (user-select: none)
```

**See:** `assets/integration-checklist-template.md` for full template

## Phase 10: Finalize parameter-spec.md (After Finalization Only)

**Prerequisites:** User confirmed design in Phase 5.5 decision menu AND this is the first mockup version.

**If this is the first UI mockup (v1):**

**Create:** `plugins/[Name]/.ideas/parameter-spec.md`

**Purpose:** Lock parameter specification for implementation. This becomes the **immutable contract** for all subsequent stages.

**Extract from YAML:**

```markdown
## Total Parameter Count

**Total:** 5 parameters

## Parameter Definitions

### threshold
- **Type:** Float
- **Range:** -60.0 to 0.0 dB
- **Default:** -20.0
- **Skew Factor:** linear
- **UI Control:** Rotary knob, center-top position
- **DSP Usage:** Compressor threshold level
```

**See:** `assets/parameter-spec-template.md`

## Phase 10.5: Finalization Commit

**CRITICAL: Commit all implementation files and update workflow state.**

**After Phase 10 completes (all 7 files generated):**

```bash
cd plugins/[PluginName]/.ideas/mockups
git add v[N]-ui.html v[N]-PluginEditor.h v[N]-PluginEditor.cpp v[N]-CMakeLists.txt v[N]-integration-checklist.md

# If parameter-spec.md was created (v1 only)
if [ -f "../parameter-spec.md" ]; then
    git add ../parameter-spec.md
fi

git commit -m "feat([PluginName]): UI mockup v[N] finalized (implementation files)"
```

**Update workflow state (if in workflow context):**

```bash
if [ -f "plugins/[PluginName]/.ideas/.continue-here.md" ]; then
    # Update finalization status
    sed -i '' "s/mockup_finalized: .*/mockup_finalized: true/" .continue-here.md
    sed -i '' "s/finalized_version: .*/finalized_version: [N]/" .continue-here.md
    sed -i '' "s/stage_0_status: .*/stage_0_status: ui_design_complete/" .continue-here.md

    git add .continue-here.md
    git commit --amend --no-edit
fi
```

**Updated state in `.continue-here.md`:**

```markdown
current_stage: 0
stage_0_status: ui_design_complete
latest_mockup_version: 2
mockup_finalized: true
finalized_version: 2
```

**Why this matters:**
- Marks design phase as complete
- Enables `/continue` to resume at Stage 1
- Records which version was finalized (if multiple exist)
- Atomic commit of all implementation files

## After Completing All Phases

Once user has finalized a design and all 7 files are generated, present this menu:

```
✓ Mockup v[N] complete (7 files generated)

What's next?
1. Start implementation (invoke plugin-workflow)
2. Create another UI version (explore alternative design)
3. Test in browser (open v[N]-ui-test.html)
4. Other

Choose (1-4): _
```

## Versioning Strategy

**v1, v2, v3...** Each UI version is saved separately.

**Why multiple versions:**

- Explore different layouts without losing previous work
- A/B test designs in browser before committing
- Iterate based on user feedback
- Keep design history

**File naming:**

```
plugins/[Name]/.ideas/mockups/
├── v1-ui.yaml
├── v1-ui.html
├── v1-browser-test.html
├── v1-PluginEditor.h
├── v1-PluginEditor.cpp
├── v1-CMakeLists.txt
├── v1-integration-checklist.md
├── v2-ui.yaml  (if user wants alternative design)
├── v2-ui.html
└── ... (v2 variants)
```

**Latest version is used for Stage 5 implementation** (unless user specifies different version).

## Success Criteria

**Design phase successful when:**
- ✅ YAML spec generated matching user requirements
- ✅ Browser test HTML works (interactive controls, parameter messages)
- ✅ Design files committed to git (Phase 5.45)
- ✅ `.continue-here.md` updated with version number (if in workflow)
- ✅ User presented with Phase 5.5 decision menu
- ✅ Design approved OR user iterates with refinements

**Implementation phase successful when (after finalization):**
- ✅ All 7 files generated and saved to `.ideas/mockups/`
- ✅ Production HTML is complete (no placeholder content)
- ✅ C++ boilerplate matches YAML structure (correct parameter bindings)
- ✅ parameter-spec.md generated and locked (for v1 only)
- ✅ Implementation files committed to git (Phase 10.5)
- ✅ `.continue-here.md` updated with finalization status (if in workflow)

## Integration Points

**Invoked by:**

- `/dream` command → After creative brief, before parameter finalization
- `plugin-workflow` skill → During Stage 0 (UI design phase)
- `plugin-improve` skill → When redesigning existing plugin UI
- Natural language: "Design UI for [PluginName]", "Create mockup for compressor"

**Invokes:**

- (None - terminal skill that generates files only)

**Creates:**

- `plugins/[Name]/.ideas/mockups/v[N]-*.{yaml,html,h,cpp,txt,md}` (7 files)
- `plugins/[Name]/.ideas/parameter-spec.md` (if v1 and doesn't exist)

**Updates:**

- `PLUGINS.md` → Mark UI designed (if part of workflow)
- `.continue-here.md` → Update workflow state (if part of workflow)

**Blocks:**

- Stage 1 (Planning) → Cannot proceed without parameter-spec.md
- Stage 5 (GUI) → Cannot implement without approved UI mockup

## Reference Documentation

- **Context extraction:** `references/context-extraction.md` - Guidelines for loading plugin context
- **Design questions:** `references/design-questions.md` - Targeted question templates and defaults
- **HTML generation:** `references/html-generation.md` - Rules for production HTML generation
- **Browser testing:** `references/browser-testing.md` - Browser test workflow
- **CMake configuration:** `references/cmake-configuration.md` - WebView build settings

## Template Assets

- **UI YAML template:** `assets/ui-yaml-template.yaml` - Complete YAML structure
- **WebView boilerplate:** `assets/webview-boilerplate.md` - C++ integration templates
- **Integration checklist:** `assets/integration-checklist-template.md` - Step-by-step integration guide
- **Parameter spec template:** `assets/parameter-spec-template.md` - Parameter specification format
