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

## Handoff File Locations

### 1. Main workflow handoff

**Location:** `plugins/[PluginName]/.continue-here.md`
**Meaning:** Plugin is in active development (Stages 0-6)
**Created by:** plugin-workflow skill
**Contains:** Stage number, phase (if complex), completed work, next steps

### 2. Ideation handoff

**Location:** `plugins/[PluginName]/.ideas/.continue-here.md`
**Meaning:** Plugin is in planning/ideation phase
**Created by:** plugin-ideation skill
**Contains:** Creative brief status, mockup status, ready-to-implement flag

### 3. Mockup handoff

**Location:** `plugins/[PluginName]/.ideas/mockups/.continue-here.md`
**Meaning:** UI mockup iteration in progress
**Created by:** ui-mockup skill
**Contains:** Current mockup version, iteration notes, finalization status

---

## Resume Workflow

The complete resume process is documented in detail in the reference files:

### Step 1: Locate Handoff

Search for handoff files across 3 locations, handle interactive plugin selection if no name provided, and disambiguate when multiple handoffs exist for same plugin.

See **[references/handoff-location.md](references/handoff-location.md)** for complete location logic.

### Step 2 & 3: Parse Context and Present Summary

Parse YAML frontmatter (plugin, stage, status, last_updated, etc.) and markdown body (current state, completed work, next steps, key decisions).

Calculate "time ago" and build user-facing summary that shows:
- Where we are in the workflow
- What's been completed
- What's next (prioritized)
- Build/test status
- Time since last session

See **[references/context-parsing.md](references/context-parsing.md)** for parsing and presentation logic.

### Step 4: Proceed with Continuation

Determine routing based on stage type (workflow, ideation, mockup, improvement), load relevant context files (contracts, source code, git history), and invoke appropriate continuation skill.

See **[references/continuation-routing.md](references/continuation-routing.md)** for routing logic.

---

## Error Recovery

Common error scenarios with recovery strategies:

- **No Handoff Found**: Check PLUGINS.md and git log to infer state, offer reconstruction options
- **Corrupted Handoff File**: Parse git log to infer stage, offer manual recreation
- **Stale Handoff (>2 weeks old)**: Warn about staleness, offer to verify code changes
- **Multiple Handoffs for Same Plugin**: Present disambiguation menu with recommendations

See **[references/error-recovery.md](references/error-recovery.md)** for all error scenarios and advanced features.

---

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

---

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

---

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
