# Handoff Location and Selection

**Context:** This file is part of the context-resume skill.
**Invoked by:** Main skill when user runs /continue [PluginName]
**Purpose:** Locate handoff files across 3 possible locations and handle disambiguation

---

## Step 1a: Determine Plugin Context

**If plugin name provided:**

Search the 3 standard handoff locations in order:

### 1. Main workflow handoff

```bash
test -f "plugins/$PLUGIN_NAME/.continue-here.md"
```

**Location meaning:** Plugin is in active development (Stages 0-6)
**Created by:** plugin-workflow skill
**Contains:** Stage number, phase (if complex), completed work, next steps

### 2. Ideation handoff

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/.continue-here.md"
```

**Location meaning:** Plugin is in planning/ideation phase
**Created by:** plugin-ideation skill
**Contains:** Creative brief status, mockup status, ready-to-implement flag

### 3. Mockup handoff

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/mockups/.continue-here.md"
```

**Location meaning:** UI mockup iteration in progress
**Created by:** ui-mockup skill
**Contains:** Current mockup version, iteration notes, finalization status

**If single handoff found:** Proceed to context parsing (see `references/context-parsing.md`)

**If multiple handoffs found:** Present disambiguation menu (see below)

**If no handoffs found:** See `references/error-recovery.md`

## Step 1b: Interactive Plugin Selection

**If no plugin name provided:**

Search for all handoff files:

```bash
find plugins -name ".continue-here.md" -type f
```

For each handoff found, extract context summary and present interactive menu:

```
Which plugin would you like to resume?

1. TapeDelay
   Stage 4 (DSP implementation) • In progress • 2 hours ago
   Last: Core delay algorithm implemented, testing modulation next

2. VintageComp
   Stage 6 (Validation) • Testing • 1 day ago
   Last: Pluginval passed, creating factory presets

3. SpringReverb
   Mockup v2 ready • Ready to implement • 3 days ago
   Last: UI design finalized, parameter-spec.md generated

4. FilterBank
   Creative brief complete • Not started • 1 week ago
   Last: Ideation complete, ready for mockup or implementation

5. Other

Choose (1-5): _
```

**Menu details:**

- Shows plugin name
- Shows current activity/stage
- Shows status (in progress, ready, complete, etc.)
- Shows time since last update (human-readable)
- Shows brief summary of last action
- Sorted by recency (most recent first)

**Handle user selection:**

- Options 1-4: Set `$PLUGIN_NAME` from selection, proceed to context parsing
- Option 5: Ask "Which plugin would you like to resume?" (free text), then search for that plugin

## Step 1c: Multiple Handoffs for Same Plugin

**If multiple handoffs exist for the same plugin:**

Present disambiguation:

```
Multiple resume points found for TapeDelay:

1. Main workflow: Stage 4 (DSP implementation)
   Location: plugins/TapeDelay/.continue-here.md
   Last updated: 2 hours ago
   Context: Implementing modulation matrix for wow/flutter

2. UI mockup iteration: Mockup v3 in progress
   Location: plugins/TapeDelay/.ideas/mockups/.continue-here.md
   Last updated: 1 day ago
   Context: Exploring vintage tape UI design options

Which context would you like to resume?
1. Main workflow (recommended for implementation)
2. UI mockup (continue design work)
3. Show both and let me decide
4. Other

Choose (1-4): _
```

**Recommendation logic:**

- If main workflow is more recent → Recommend that
- If mockup is more recent → Recommend mockup
- If workflow is at Stage 5 (GUI) and mockup exists → Recommend mockup
- Default to main workflow if uncertain

---

**Return to:** Main context-resume orchestration in `SKILL.md`
