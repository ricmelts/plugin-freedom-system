---
name: plugin-workflow
description: Complete 7-stage JUCE plugin development workflow
allowed-tools:
  - Task # For subagents (Stages 2-5)
  - Bash # For git commits
  - Read # For contracts
  - Write # For documentation
  - Edit # For state updates
preconditions:
  - Plugin must not exist OR status must be 💡 (ideated)
  - Plugin must NOT be 🚧 (use /continue instead)
---

# plugin-workflow Skill

**Purpose:** Orchestrate complete 7-stage JUCE plugin development from research to validated, production-ready plugin.

## Overview

This skill manages the complete plugin implementation workflow with the following stages:

- **Stage 0:** Research - Understand requirements, identify JUCE modules, create DSP architecture
- **Stage 1:** Planning - Calculate complexity, determine implementation strategy
- **Stage 2:** Foundation - Create build system, verify compilation
- **Stage 3:** Shell - Implement APVTS with all parameters
- **Stage 4:** DSP - Implement audio processing
- **Stage 5:** GUI - Integrate WebView UI with parameter bindings
- **Stage 6:** Validation - Factory presets, pluginval, CHANGELOG

Each stage is fully documented in its own reference file in `references/` subdirectory.

## Precondition Checking

**Before starting, verify:**

1. Read PLUGINS.md:

```bash
grep "^### $PLUGIN_NAME$" PLUGINS.md
```

2. Check status:

   - If not found → OK to proceed (new plugin)
   - If status = 💡 Ideated → OK to proceed
   - If status = 🚧 Stage N → BLOCK with message:
     ```
     [PluginName] is already in development (Stage [N]).
     Use /continue [PluginName] to resume the workflow.
     ```
   - If status = ✅ Working or 📦 Installed → BLOCK with message:
     ```
     [PluginName] is already complete.
     Use /improve [PluginName] to make changes.
     ```

3. Check for creative brief:

```bash
test -f "plugins/$PLUGIN_NAME/.ideas/creative-brief.md"
```

If missing, offer:

```
No creative brief found for [PluginName].

Would you like to:
1. Create one now (/dream [PluginName]) (recommended)
2. Continue without brief (not recommended)

Choose (1-2): _
```

If user chooses 1, exit and instruct them to run `/dream [PluginName]`.

---

## Resume Entry Point

**Purpose:** Handle workflow resume with orchestration mode support.

**When invoked via context-resume skill:**

1. **Check if handoff file exists:**
   ```bash
   if [ ! -f "plugins/${PLUGIN_NAME}/.continue-here.md" ]; then
       echo "No handoff file found. Starting new workflow."
       CURRENT_STAGE=0
   fi
   ```

2. **Parse handoff for orchestration mode:**
   ```bash
   ORCHESTRATION_MODE=$(grep "^orchestration_mode:" plugins/${PLUGIN_NAME}/.continue-here.md | awk '{print $2}')
   NEXT_ACTION=$(grep "^next_action:" plugins/${PLUGIN_NAME}/.continue-here.md | awk '{print $2}')
   NEXT_PHASE=$(grep "^next_phase:" plugins/${PLUGIN_NAME}/.continue-here.md | awk '{print $2}')
   ```

3. **If orchestration_mode: true:**
   - Read next_action (e.g., "invoke_dsp_agent")
   - Read next_phase (e.g., "4.4")
   - Parse phase to determine stage and sub-phase
   - Invoke specified subagent using Task tool
   - DO NOT implement directly in skill context

4. **If orchestration_mode: false or not present:**
   - Use legacy dispatcher (below)

---

## Stage Dispatcher

**Purpose:** Route to correct stage implementation based on current state.

**Entry point:** Called by /implement command or when resuming workflow WITHOUT orchestration mode.

### Implementation

1. **Determine current stage:**

```bash
# Check if handoff file exists (resuming)
if [ -f "plugins/${PLUGIN_NAME}/.continue-here.md" ]; then
    # Parse stage from handoff YAML frontmatter
    CURRENT_STAGE=$(grep "^stage:" plugins/${PLUGIN_NAME}/.continue-here.md | awk '{print $2}')
    echo "Resuming from Stage ${CURRENT_STAGE}"
else
    # Starting new workflow
    CURRENT_STAGE=0
    echo "Starting workflow at Stage 0"
fi
```

2. **Verify preconditions for target stage:**

See `references/state-management.md` for `checkStagePreconditions()` function.

3. **Route to stage implementation:**

```javascript
function dispatchStage(pluginName, stageNumber) {
  // Check preconditions
  const preconditionCheck = checkStagePreconditions(pluginName, stageNumber)

  if (!preconditionCheck.allowed) {
    // Display blocking message
    console.log(`✗ BLOCKED: ${preconditionCheck.reason}`)
    console.log(`Action: ${preconditionCheck.action}`)
    return { status: 'blocked', reason: preconditionCheck.reason }
  }

  // Route to stage (see reference files for implementation)
  switch(stageNumber) {
    case 0:
      return executeStage0Research(pluginName)  // See references/stage-0-research.md
    case 1:
      return executeStage1Planning(pluginName)  // See references/stage-1-planning.md
    case 2:
      return executeStage2Foundation(pluginName)  // See references/stage-2-foundation.md
    case 3:
      return executeStage3Shell(pluginName)  // See references/stage-3-shell.md
    case 4:
      return executeStage4DSP(pluginName)  // See references/stage-4-dsp.md
    case 5:
      return executeStage5GUI(pluginName)  // See references/stage-5-gui.md
    case 6:
      return executeStage6Validation(pluginName)  // See references/stage-6-validation.md
    default:
      return { status: 'error', reason: `Invalid stage: ${stageNumber}` }
  }
}
```

4. **Stage loop for continuous execution:**

```javascript
function runWorkflow(pluginName, startStage = 0) {
  let currentStage = startStage
  let shouldContinue = true

  while (shouldContinue && currentStage <= 6) {
    console.log(`\n━━━ Stage ${currentStage} ━━━\n`)

    // Execute stage
    const result = dispatchStage(pluginName, currentStage)

    if (result.status === 'blocked') {
      console.log("\nWorkflow blocked. Resolve issues and resume with /continue.")
      return result
    }

    if (result.status === 'error') {
      console.log(`\nError: ${result.reason}`)
      return result
    }

    // Present decision menu
    const choice = presentDecisionMenu({
      stage: currentStage,
      completionStatement: result.completionStatement,
      pluginName: pluginName
    })

    // Handle user choice
    if (choice === 'continue' || choice === 1) {
      currentStage++
    } else if (choice === 'pause') {
      console.log("\n✓ Workflow paused. Resume anytime with /continue")
      shouldContinue = false
    } else {
      // Handle other menu options (review, test, etc.)
      handleMenuChoice(choice, pluginName, currentStage)
    }
  }

  if (currentStage > 6) {
    console.log("\n✓ All stages complete!")
  }
}
```

**Usage:**

```javascript
// From /implement command:
runWorkflow(pluginName, 0)

// From /continue command (via context-resume):
const handoff = readHandoffFile(pluginName)
const resumeStage = handoff.stage
runWorkflow(pluginName, resumeStage)
```

---

## Stage Reference Files

Each stage has detailed implementation in its own reference file:

- **[Stage 0: Research](references/stage-0-research.md)** - Understand requirements, identify JUCE modules, create DSP architecture (~235 lines)
- **[Stage 1: Planning](references/stage-1-planning.md)** - Calculate complexity, determine implementation strategy (~335 lines)
- **[Stage 2: Foundation](references/stage-2-foundation.md)** - Create build system, verify compilation (~372 lines)
- **[Stage 3: Shell](references/stage-3-shell.md)** - Implement APVTS with all parameters (~275 lines)
- **[Stage 4: DSP](references/stage-4-dsp.md)** - Implement audio processing (~531 lines)
- **[Stage 5: GUI](references/stage-5-gui.md)** - Integrate WebView UI with parameter bindings (~528 lines)
- **[Stage 6: Validation](references/stage-6-validation.md)** - Factory presets, pluginval, CHANGELOG (~190 lines)
- **[State Management](references/state-management.md)** - State machine, git commits, handoffs, decision menus (~300 lines)

---

## Integration Points

**Invoked by:**

- `/implement` command
- `plugin-ideation` skill (after creative brief)
- `context-resume` skill (when resuming)

**Invokes:**

- `foundation-agent` subagent (Stage 2)
- `shell-agent` subagent (Stage 3)
- `dsp-agent` subagent (Stage 4)
- `gui-agent` subagent (Stage 5)
- `validator` subagent (Stages 4, 5, 6)
- `build-automation` skill (Stages 2-5)
- `plugin-testing` skill (Stages 4, 5, 6)
- `plugin-lifecycle` skill (after Stage 6, if user chooses install)

**Creates:**

- `.continue-here.md` (handoff file)
- `architecture.md` (Stage 0 - DSP specification)
- `plan.md` (Stage 1)
- `CHANGELOG.md` (Stage 6)
- `Presets/` directory (Stage 6)

**Updates:**

- PLUGINS.md (status changes throughout)

---

## Error Handling

**If contract files missing at Stage 1:**
Block and guide to create UI mockup first. See `references/stage-1-planning.md`.

**If build fails at any stage:**
Present 4-option menu (investigate, show code, show build output, manual fix). Handled by build-automation skill.

**If tests fail:**
Present menu with investigation options. Do NOT proceed to next stage.

**If git staging fails:**
Continue anyway, log warning.

---

## Success Criteria

Workflow is successful when:

- Plugin compiles without errors
- All stages completed in sequence
- Tests pass (if run)
- PLUGINS.md updated to ✅ Working
- Handoff file deleted (workflow complete)
- Git history shows all stage commits

---

## Notes for Claude

**When executing this skill:**

1. Follow stage reference files exactly - they contain complete implementation details
2. Use decision menus at every checkpoint - never auto-proceed without user confirmation
3. State management functions are in `references/state-management.md`
4. Build failures are handled by build-automation skill (don't duplicate logic)
5. All git commits use standardized format from `commitStage()` function
6. Handoff files preserve context across sessions - update after every stage
7. Complexity score determines single-pass vs phased implementation
8. Validator is advisory for complexity ≥4 plugins (user makes final decision)

**Common pitfalls:**

- Auto-proceeding without user confirmation
- Not updating handoff file after each stage
- Forgetting to check contract prerequisites
- Proceeding to next stage when tests fail
- Not presenting decision menu after stage completion
