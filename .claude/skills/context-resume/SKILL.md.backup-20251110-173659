---
name: context-resume
description: Load plugin context from handoff files to resume work
allowed-tools:
  - Read
  - Bash
  - Skill # To invoke next skill
preconditions:
  - Handoff file must exist in one of 3 locations
---

# context-resume Skill

**Purpose:** Load plugin development context from `.continue-here.md` handoff files to enable pause/resume across sessions. Creates continuity between work sessions by preserving and restoring complete plugin development state.

## Overview

The handoff system allows plugins to be paused at any point and resumed later with full context preservation. This skill is the universal entry point for resuming any type of work: implementation, ideation, UI mockup iteration, or improvements.

**Key capabilities:**

- Locates handoff files across 3 possible locations
- Parses YAML frontmatter and markdown body for structured context
- Summarizes current state and progress
- Loads relevant contract files and source code
- Routes to appropriate continuation skill
- Handles missing or corrupted handoff files gracefully

## Step 1: Locate Handoff

### 1a: Determine Plugin Context

**If plugin name provided:**

Search the 3 standard handoff locations in order:

1. **Main workflow handoff:**

```bash
test -f "plugins/$PLUGIN_NAME/.continue-here.md"
```

**Location meaning:** Plugin is in active development (Stages 0-6)
**Created by:** plugin-workflow skill
**Contains:** Stage number, phase (if complex), completed work, next steps

2. **Ideation handoff:**

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/.continue-here.md"
```

**Location meaning:** Plugin is in planning/ideation phase
**Created by:** plugin-ideation skill
**Contains:** Creative brief status, mockup status, ready-to-implement flag

3. **Mockup handoff:**

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/mockups/.continue-here.md"
```

**Location meaning:** UI mockup iteration in progress
**Created by:** ui-mockup skill
**Contains:** Current mockup version, iteration notes, finalization status

**If single handoff found:**
Proceed to Step 2 (Parse Context).

**If multiple handoffs found:**
Present disambiguation menu (see below).

**If no handoffs found:**
Skip to Error Handling section.

### 1b: Interactive Plugin Selection

**If no plugin name provided:**

Search for all handoff files:

```bash
find plugins -name ".continue-here.md" -type f
```

For each handoff found, extract context summary:

```bash
# Extract plugin name from path
# Read YAML frontmatter for stage/status
# Calculate time since last_updated
```

Present interactive menu:

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

- Options 1-4: Set `$PLUGIN_NAME` from selection, proceed to Step 2
- Option 5: Ask "Which plugin would you like to resume?" (free text), then search for that plugin

### 1c: Multiple Handoffs for Same Plugin

**If multiple handoffs exist for the same plugin:**

This happens when:

- Plugin has main workflow handoff AND ideation/mockup handoff
- User paused workflow, then explored UI redesign
- Multiple parallel work streams

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

- If main workflow is more recent → Recommend that (as shown)
- If mockup is more recent → Recommend mockup
- If workflow is at Stage 5 (GUI) and mockup exists → Recommend mockup
- Default to main workflow if uncertain

## Step 2: Parse Context

### 2a: Read Handoff File

Read the complete `.continue-here.md` file:

```bash
cat "plugins/$PLUGIN_NAME/.continue-here.md"
```

### 2b: Parse YAML Frontmatter

Extract structured metadata from YAML header:

**Required fields:**

```yaml
---
plugin: PluginName # String: Plugin directory name
stage: N # Integer: Current stage (0-6) or ideation marker
status: in_progress # String: in_progress, paused, blocked, ready
last_updated: 2025-11-10 14:30:00 # Timestamp: When handoff was written
---
```

**Optional fields:**

```yaml
phase: M # Integer: Current phase within stage (for complex plugins)
complexity_score: 3.2 # Float: Calculated complexity score
phased_implementation: true # Boolean: Whether using phase-based workflow
improvement: feature-name # String: Improvement proposal filename (for plugin-improve)
mockup_version: 2 # Integer: Current mockup version number
```

**Field meanings:**

- **plugin**: Directory name exactly as appears in `plugins/` (used for file operations)
- **stage**:
  - 0-6: Workflow stage number
  - "ideation": Creative brief phase
  - "mockup": UI mockup phase
  - "improvement_planning": Improvement proposal phase
- **status**:
  - `in_progress`: Actively working, normal workflow
  - `paused`: User explicitly paused, resume where left off
  - `blocked`: Waiting on external dependency (research, decision, etc.)
  - `ready`: Phase complete, ready for next action
- **last_updated**: ISO 8601 timestamp, used to calculate "time ago" display
- **phase**: For complex plugins with multi-phase stages (Stage 4.1, 4.2, etc.)
- **complexity_score**: From planning stage, determines single-pass vs phased
- **phased_implementation**: True if complexity ≥ 3, affects workflow routing
- **improvement**: Links to improvement proposal in `.ideas/improvements/`
- **mockup_version**: Which mockup iteration (v1, v2, v3, etc.)

### 2c: Parse Markdown Body

Extract narrative context from markdown sections:

**Expected structure:**

```markdown
# Resume Point

## Current State: [Stage/Phase Description]

[Prose description of where we are, what's happening now]

## Completed So Far

**Stage 0-N:** ✓ Complete

- [Specific accomplishment 1]
- [Specific accomplishment 2]
- [Key decision made]

**Stage N:** 🚧 In Progress

- [What's already done in current stage]
- [What's next in current stage]

## Next Steps

1. [Specific next action - most immediate]
2. [Following action - after first]
3. [Alternative action - if needed]

## Context to Preserve

**Key Decisions:**

- [Design choice 1 and rationale]
- [Technical approach chosen]
- [Trade-off made]

**Files Modified:**

- plugins/[Name]/Source/PluginProcessor.cpp:123-145
- plugins/[Name]/Source/PluginEditor.cpp:67

**Current Build Status:**

- Last build: [Success/Failed]
- Last test: [Pass/Fail with details]

**Research References:**

- [JUCE doc link for specific API]
- [Example plugin referenced]
- [DSP paper or tutorial]
```

**Extract key information:**

1. **Current state description**: What phase/stage we're in, what's being worked on
2. **Completed work**: What's been finished, what decisions have been made
3. **Next steps**: Specific actionable items (in priority order)
4. **Key decisions**: Design choices that shouldn't be forgotten
5. **Modified files**: Specific file:line references for quick navigation
6. **Build status**: Whether last build/test succeeded
7. **Research links**: References to look up if needed

### 2d: Calculate Time Ago

Parse `last_updated` timestamp and calculate human-readable "time ago":

```bash
# Example calculation
last_updated="2025-11-10 14:30:00"
current_time=$(date +"%Y-%m-%d %H:%M:%S")
# Calculate difference and format as:
# "5 minutes ago"
# "2 hours ago"
# "1 day ago"
# "3 days ago"
# "2 weeks ago"
```

## Step 3: Summarize and Present

### 3a: Build Summary

Construct user-facing summary combining all parsed information:

**Example for workflow resume:**

```
Resuming TapeDelay at Stage 4 (DSP Implementation)...

Status: Stage 4.2 in progress (modulation system)
Last session: 2 hours ago

Progress so far:
✓ Stage 0: Research (tape delay algorithms, wow/flutter)
✓ Stage 1: Planning (complexity 3.8, phased implementation)
✓ Stage 2: Foundation (CMake configured, project compiles)
✓ Stage 3: Shell (plugin loads in DAW, processes silence)
✓ Stage 4.1: Core delay algorithm (delay line, feedback working)
🚧 Stage 4.2: Modulation system (in progress)

Current work:
- Implementing LFOs for wow/flutter modulation
- Parameter ranges validated against research
- Next: Add modulation to delay read pointer

Last build: ✓ Success
Last test: ✓ Pass (4/5 tests, 1 skipped - modulation not complete)

Next steps:
1. ⭐ Continue Stage 4.2 (implement modulation matrix)
2. Review Stage 4.1 code (verify core delay algorithm)
3. Test wow/flutter parameters manually
4. Pause here

Ready to continue with modulation implementation?
```

**Example for ideation resume:**

```
Resuming SpringReverb at ideation phase...

Status: Creative brief complete, UI mockup ready
Last session: 3 days ago

Completed so far:
✓ Creative brief: Spring reverb with vintage character
✓ UI mockup v2: Finalized design (3 knobs, vintage aesthetic)
✓ Parameter spec: Generated from mockup (3 parameters defined)

Current state:
Ready to begin implementation (Stage 0: Research)

Key decisions preserved:
- Using Schroeder reverb architecture + spring simulation
- 3-parameter design (spring tension, damping, mix)
- Vintage UI with analog-style knobs

Next steps:
1. ⭐ Start implementation (/implement SpringReverb)
2. Refine mockup further (iterate to v3)
3. Review creative brief
4. Research spring reverb algorithms more

Ready to start implementation?
```

**Example for mockup resume:**

```
Resuming DelayPlugin mockup iteration...

Status: Mockup v3 in progress
Last session: 1 day ago

Mockup history:
✓ v1: Initial 4-knob layout (too cramped)
✓ v2: Expanded to 600x450px (better, but color scheme off)
🚧 v3: Refining color scheme and spacing

Current iteration (v3):
- Exploring darker background (#1a1a1a vs #2b2b2b)
- Adjusting knob spacing (80px between controls)
- Testing hover states and animations

Next steps:
1. ⭐ Finalize v3 design (complete color refinements)
2. Generate v4 with different layout (explore alternatives)
3. Finalize v2 instead (go back to previous version)
4. Test v3 in browser (open v3-ui-test.html)

Ready to finalize v3 color scheme?
```

### 3b: Present Summary

Display the summary to user and wait for confirmation:

```
[Summary from 3a above]

Choose (1-4): _
```

**Wait for user response before proceeding.**

Do not auto-proceed - user must explicitly choose next action.

## Step 4: Proceed with Continuation

### 4a: Determine Routing

Based on handoff content and user selection, route to appropriate skill:

**Workflow resume (stage = 0-6):**

```
User chose: Continue Stage [N]

Routing to: plugin-workflow skill
Parameters:
  - plugin_name: [PluginName]
  - resume_from: stage [N]
  - phase: [M] (if phased)
```

Invoke plugin-workflow skill with resume context:

```
Resuming [PluginName] at Stage [N]...
```

plugin-workflow will:

- Read handoff file for context
- Skip completed stages
- Continue from current stage/phase
- Update handoff file as it progresses

**Ideation resume (stage = "ideation"):**

```
User chose option 1 (Start implementation) or 2 (Refine mockup)

If option 1:
  Routing to: plugin-workflow skill (Stage 0)

If option 2:
  Routing to: plugin-ideation skill (improvement mode) or ui-mockup skill
```

**Mockup resume (stage = "mockup"):**

```
User chose option 1 (Finalize) or 2 (Iterate)

If option 1 (Finalize):
  - Generate parameter-spec.md if not exists
  - Offer to start Stage 5 (GUI) with plugin-workflow
  - Or offer to start Stage 0 (Research) for new implementation

If option 2 (Iterate):
  Routing to: ui-mockup skill (creates v[N+1])
```

**Improvement resume (stage = "improvement_planning"):**

```
User chose: Start implementation

Routing to: plugin-improve skill
Parameters:
  - plugin_name: [PluginName]
  - improvement_file: [from YAML]
```

### 4b: Load Context Files

Before invoking continuation skill, load relevant context:

**For workflow resume:**

```bash
# Read all contract files
cat "plugins/$PLUGIN_NAME/.ideas/creative-brief.md"
cat "plugins/$PLUGIN_NAME/.ideas/parameter-spec.md"
cat "plugins/$PLUGIN_NAME/.ideas/architecture.md"
cat "plugins/$PLUGIN_NAME/.ideas/plan.md"

# Show recent commits for this plugin
git log --oneline plugins/$PLUGIN_NAME/ -5

# Read source files mentioned in handoff (if any)
cat "plugins/$PLUGIN_NAME/Source/PluginProcessor.cpp"  # if referenced
```

**For ideation/mockup resume:**

```bash
# Read creative brief
cat "plugins/$PLUGIN_NAME/.ideas/creative-brief.md"

# Read latest mockup if exists
find "plugins/$PLUGIN_NAME/.ideas/mockups/" -name "v*-ui.yaml" | sort -V | tail -1 | xargs cat
```

**For improvement resume:**

```bash
# Read improvement proposal
cat "plugins/$PLUGIN_NAME/.ideas/improvements/$IMPROVEMENT_FILE"

# Read CHANGELOG for version context
cat "plugins/$PLUGIN_NAME/CHANGELOG.md"

# Read PLUGINS.md entry
grep -A 20 "^### $PLUGIN_NAME$" PLUGINS.md
```

### 4c: Invoke Continuation Skill

Execute the appropriate skill with full context:

**Example: Workflow continuation**

```
Loaded context:
- Creative brief: Tape delay with vintage character
- Parameter spec: 8 parameters defined
- Architecture: Schroeder topology with modulation
- Plan: Complexity 3.8, phased implementation

Invoking plugin-workflow at Stage 4.2...
```

Skill then continues from documented next steps.

## Error Handling

### Error 1: No Handoff Found

**Scenario:** `.continue-here.md` doesn't exist in any of the 3 locations.

**Response:**

```
No resumable work found for [PluginName].

Let me check the plugin status...
```

**Check PLUGINS.md:**

```bash
grep -A 10 "^### $PLUGIN_NAME$" PLUGINS.md
```

**If plugin found in PLUGINS.md:**

```
Plugin status: [Status from PLUGINS.md]
Last updated: [Date from PLUGINS.md]
```

**Check git log:**

```bash
git log --oneline plugins/$PLUGIN_NAME/ -5
```

**Present findings:**

```
Last commits:
- abc1234 feat: TapeDelay Stage 4.2 - modulation system (2 hours ago)
- def5678 feat: TapeDelay Stage 4.1 - core delay (4 hours ago)
- ghi9012 feat: TapeDelay Stage 3 - shell (1 day ago)

Best guess: Stage 4 was in progress but handoff file was deleted or lost.

How would you like to proceed?

1. Start from Stage 5 (GUI) - assume Stage 4 complete
2. Review Stage 4 code to verify completion
3. Restart Stage 4 from scratch (re-implement DSP)
4. Check if Stage 6 (Validation) is more appropriate
5. Other

Choose (1-5): _
```

**Handle user choice:**

- Option 1: Invoke plugin-workflow at Stage 5
- Option 2: Read source files, show code, re-present menu
- Option 3: Invoke plugin-workflow at Stage 4 (fresh start)
- Option 4: Invoke plugin-workflow at Stage 6
- Option 5: Ask what they'd like to do

**If plugin NOT found in PLUGINS.md:**

```
[PluginName] not found in PLUGINS.md.

This plugin may not exist yet or was never registered.

Options:
1. Create new plugin with this name (/dream [PluginName])
2. List all existing plugins
3. Check if directory exists but not registered
4. Other

Choose (1-4): _
```

### Error 2: Ambiguous State (Already Covered)

See Step 1c above - multiple handoffs for same plugin.

### Error 3: Corrupted Handoff File

**Scenario:** File exists but YAML is invalid or structure is wrong.

**Detection:**

```bash
# Try to parse YAML frontmatter
# If parse fails → corrupted
```

**Response:**

```
Handoff file exists but has invalid format.

Error: [Specific YAML parse error]

Attempting to reconstruct state from git history...
```

**Parse git log to infer state:**

```bash
git log --oneline plugins/$PLUGIN_NAME/ -10
```

**Look for stage markers in commit messages:**

```
- "Stage 4.2" in message → likely at Stage 4.2
- "Stage 5 - GUI" in message → likely at Stage 5
- "validation complete" → likely at Stage 6
```

**Present reconstruction:**

```
Reconstructed state from git commits:

Inferred stage: Stage 4 (based on commit messages)
Last commit: feat: TapeDelay Stage 4.2 - modulation system
Commit date: 2 hours ago

Warning: This is a best guess. Handoff file was corrupted.

Options:
1. Proceed with Stage 5 (next stage after inferred position)
2. Review code to manually verify current state
3. Recreate handoff file manually (I'll help)
4. Other

Choose (1-4): _
```

**If user chooses option 3 (recreate):**

```
I'll help you recreate the handoff file.

First, let me check the current implementation state:
[Read source files, check build status, read contracts]

Based on code review:
- PluginProcessor implements: [list features found]
- PluginEditor implements: [list UI elements found]
- Tests directory: [exists/missing]

Does this match Stage 4 completion? (y/n): _
```

Create new handoff file based on code analysis and user confirmation.

### Error 4: Stale Handoff (Old Timestamp)

**Scenario:** Handoff file exists but `last_updated` is > 2 weeks ago.

**Warning:**

```
Resuming [PluginName]...

⚠️ Note: This handoff is 3 weeks old.
Context may be stale. Recent git activity detected.

Last handoff: 2025-10-20 (3 weeks ago)
Recent commit: 2025-11-08 (2 days ago)

The code may have changed since this handoff was written.

Options:
1. Use handoff anyway (may be outdated)
2. Check git log first (verify recent changes)
3. Review code to assess current state
4. Recreate handoff from current code state

Choose (1-4): _
```

## Advanced Features

### Feature 1: Time-Travel Resume

Support resuming from older handoff file if multiple backups exist:

```bash
# If .continue-here.md.backup files exist
find plugins/$PLUGIN_NAME -name ".continue-here.md*"
```

**Present options:**

```
Multiple handoff files found (including backups):

1. Current: Stage 4.2 (2 hours ago)
2. Backup: Stage 4.1 (1 day ago)
3. Backup: Stage 3 complete (2 days ago)

Which version would you like to resume from?

Choose (1-3): _
```

### Feature 2: Diff Between Sessions

Show what changed since last session:

```bash
# Get timestamp from handoff
last_updated="2025-11-10 12:00:00"

# Git log since that time
git log --since="$last_updated" plugins/$PLUGIN_NAME/ --oneline

# Git diff since that time
git diff "HEAD@{$last_updated}" HEAD -- plugins/$PLUGIN_NAME/
```

**Present summary:**

```
Changes since last session (2 hours ago):

Commits:
- abc1234 feat: Added modulation LFO (1 hour ago)
- def5678 fix: Fixed feedback calculation (30 min ago)

Files changed:
- PluginProcessor.cpp: +45 lines, -12 lines
- PluginEditor.cpp: +23 lines, -5 lines

Continue from current state? (y/n): _
```

### Feature 3: Cross-Session Notes

Allow users to leave notes for their future self:

**In handoff file markdown:**

```markdown
## Notes for Next Session

- Remember to test wow/flutter at extreme values
- Check if feedback parameter needs nonlinear scaling
- UI mockup v3 might need iteration before Stage 5
```

Display these prominently in summary.

## Integration Points

**Invoked by:**

- `/continue` command (no args → interactive plugin selection)
- `/continue [PluginName]` command (specific plugin)
- Natural language: "resume [PluginName]", "continue working on [PluginName]"

**Invokes:**

- `plugin-workflow` (workflow resume at specific stage)
- `plugin-ideation` (ideation resume for improvements)
- `ui-mockup` (mockup iteration resume)
- `plugin-improve` (improvement implementation resume)

**Reads:**

- `.continue-here.md` files (all 3 locations)
- PLUGINS.md (status and version verification)
- Git log (commit history for inference)
- Contract files (creative-brief.md, parameter-spec.md, architecture.md, plan.md)
- Source files (if mentioned in handoff)
- CHANGELOG.md (for improvements)

**Updates:**

- Nothing directly (just reads and routes)
- Continuation skills will update handoff files as they proceed

## Success Criteria

Resume is successful when:

1. **Handoff located:** Found correct handoff file(s) from 3 possible locations
2. **Context parsed:** YAML and markdown extracted without errors
3. **State understood:** User sees clear summary of where they left off
4. **Continuity felt:** User doesn't need to remember details, handoff provides everything
5. **Appropriate routing:** Correct continuation skill invoked with right parameters
6. **Context loaded:** Contract files and relevant code loaded before proceeding
7. **Error handled:** Missing/corrupt handoff handled gracefully with fallbacks
8. **User control:** User explicitly chooses to continue, not auto-proceeded

## Notes for Claude

**When executing this skill:**

1. Always search all 3 handoff locations before declaring "not found"
2. Parse YAML carefully - handle missing optional fields gracefully
3. Present time-ago in human-readable format (not raw timestamps)
4. Show enough context that user remembers where they were
5. Don't auto-proceed - wait for explicit user choice
6. Load contract files BEFORE invoking continuation skill (provides context)
7. If handoff is stale/corrupt, use git log as backup source of truth
8. Preserve user's mental model - summary should match how they think about the plugin

**Common pitfalls:**

- Forgetting to check all 3 locations
- Auto-proceeding without user confirmation
- Not loading contract files before continuation
- Showing raw YAML instead of human summary
- Missing disambiguation when multiple handoffs exist
