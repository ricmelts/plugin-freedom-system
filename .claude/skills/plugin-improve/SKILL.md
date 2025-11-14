---
name: plugin-improve
description: Fix bugs, add features to completed plugins. Includes versioning, backups, regression testing, changelog automation. Auto-detects deep-research handoffs to preserve investigation context. Trigger terms - improve, fix, add feature, modify plugin, version bump, rollback
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
  - Task # For deep-research (Tier 3) and plugin-testing (regression tests)
preconditions:
  - Plugin status must be ✅ Working OR 📦 Installed
  - Plugin must NOT be 🚧 In Development
---

# plugin-improve Skill

**Purpose:** Make changes to completed plugins with versioning, backups, changelog automation, and root cause investigation.

**Integration with deep-research:**

<handoff_protocol>
**Trigger:** deep-research invokes plugin-improve via Skill tool
**Detection:** Phase 0.45 scans conversation history for research findings (MANDATORY)
**Action:** Extract research findings, skip investigation (Phase 0.5)
**Benefits:** Preserve expensive research context (Opus + extended thinking)
</handoff_protocol>

Detection mechanism is implemented in Phase 0.45 below. See `references/handoff-protocols.md` for additional workflow documentation.

## Workflow Overview

```
Phase 0: Specificity Detection (assess request clarity)
  ↓
[Specific?] ─NO→ Present menu (brainstorm OR investigate)
  ↓ YES
Phase 0.3: Clarification Questions (4 targeted questions)
  ↓
Phase 0.4: Decision Gate (confirm understanding)
  ↓
Phase 0.45: Research Detection (MANDATORY - scan conversation history)
  ↓
[Research found?] ─YES→ Skip to Phase 0.9
  ↓ NO
Phase 0.5: Investigation (Tier 1/2/3 auto-detected)
  ↓
Phase 0.9: Backup Verification (CRITICAL GATE - must pass to proceed)
  ↓
Phase 1: Pre-Implementation Checks (version, state, commits)
  ↓
Phase 2: Verify Rollback Path (confirm backup ready)
  ↓
Phase 3: Implementation (make changes)
  ↓
Phase 4: CHANGELOG Update (document changes)
  ↓
Phase 5: Build and Test (delegate to build-automation)
  ↓
Phase 5.5: Regression Testing (conditional: if plugin-testing + baseline exist)
  ↓
Phase 6: Git Workflow (stage changes, prepare commit)
  ↓
Phase 7: Installation (optional, delegate to plugin-lifecycle)
  ↓
Phase 8: Completion (decision menu)
```

**Key:**
- MANDATORY: Always executes (Phase 0.45)
- CRITICAL GATE: Blocks workflow if fails (Phase 0.9)
- CONDITIONAL: Only if conditions met (Phase 5.5, Phase 7)

## Progress Checklist

Copy this checklist and check off phases as you complete them:

```
Improvement Progress:
- [ ] Phase 0: Assessed request specificity
- [ ] Phase 0.3: Asked clarification questions (if needed)
- [ ] Phase 0.4: Confirmed understanding with user
- [ ] Phase 0.45: ✓ MANDATORY - Scanned conversation history for research
- [ ] Phase 0.5: Investigated root cause (if no handoff)
- [ ] Phase 0.9: ✓ CRITICAL - Backup verified (blocks if fails)
- [ ] Phase 1: Loaded current state, determined version bump
- [ ] Phase 2: Confirmed rollback path ready
- [ ] Phase 3: Implemented changes
- [ ] Phase 4: Updated CHANGELOG
- [ ] Phase 5: Built and tested (delegated to build-automation)
- [ ] Phase 5.5: Ran regression tests (if available)
- [ ] Phase 6: Staged git changes
- [ ] Phase 7: Installed plugin (if requested)
- [ ] Phase 8: Presented completion menu
```

<gate_preconditions enforcement="strict">
## Precondition Checking

**MUST execute before any other phase. BLOCK if conditions not met.**

**Before starting, verify:**

1. Read PLUGINS.md:

```bash
grep "^### $PLUGIN_NAME$" PLUGINS.md
```

2. Check status:
   - If status = ✅ Working or 📦 Installed → OK to proceed
   - If status = 🚧 In Development → BLOCK with message:
     ```
     [PluginName] is still in development (Stage [N]).
     Complete the workflow first with /continue [PluginName].
     Cannot use /improve on in-progress plugins.
     ```
   - If status = 💡 Ideated → BLOCK with message:
     ```
     [PluginName] is not implemented yet (Status: 💡 Ideated).
     Use /implement [PluginName] to build it first.
     ```
   - If not found → BLOCK with message:
     ```
     Plugin [PluginName] not found in PLUGINS.md.
     ```

</gate_preconditions>

## Phase 0: Specificity Detection

**Check if request is specific:**

Request IS specific if it has:

- Feature name (e.g., "resonance parameter", "bypass switch")
- Action (e.g., "add", "remove", "fix", "change from X to Y")
- Acceptance criteria (e.g., "range 0-1", "increase to 500px", "reduce by 3dB")

Request IS vague if lacking above:

- "improve the filters"
- "better presets"
- "UI feels cramped"
- "make it sound warmer"

**Assess specificity:**

- **Specific enough (1-2 clarification questions max):** Proceed to Phase 0.3 (4-question clarification batch)
- **Vague:** Present inline decision menu:

```
Your request needs more detail. How should I proceed?

1. Brainstorm approaches together - I'll ask questions to explore options
2. Implement something reasonable - I'll investigate and propose a solution
3. Other

Choose (1-3): _
```

**Handle responses:**
- Option 1 → Invoke plugin-ideation skill in improvement mode
- Option 2 → Proceed to Phase 0.45 (Research Detection)
- Option 3 → Collect free-form text, reassess

## Phase 0.2: Headless Plugin Detection (GUI-Optional Flow)

**Purpose:** Detect headless plugins and offer "Create custom UI" option before proceeding to normal improvement flow.

**Detection logic:**

```typescript
function detectHeadlessPlugin(pluginName: string): boolean {
  // Read .continue-here.md
  const handoffPath = `plugins/${pluginName}/.continue-here.md`;
  const handoffContent = readFile(handoffPath);

  // Check for gui_type field
  if (handoffContent.includes("gui_type: headless")) {
    return true;
  }

  // Fallback: Check if WebView UI files exist
  const webviewPath = `plugins/${pluginName}/Source/ui/public/index.html`;
  if (!fileExists(webviewPath)) {
    return true;
  }

  return false;
}
```

**If headless detected:**

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Headless Plugin Detected
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[PluginName] is currently headless (no custom UI).

What would you like to do?

1. Create custom UI - Design WebView interface (becomes v1.1.0)
2. Keep headless and make other changes - Parameter tweaks, DSP fixes, etc.
3. Show me what headless means - Explain DAW controls
4. Other

Choose (1-4): _
```

**Handle responses:**

- **Option 1:** Invoke `handleCreateCustomUi(pluginName)` (see below)
- **Option 2:** Proceed to Phase 0.3 (normal /improve flow)
- **Option 3:** Display headless explanation, then re-present menu
- **Option 4:** Collect free-form text, reassess

**Option 3 - Headless explanation:**

```
Headless Plugin Explanation
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

A "headless" plugin has no custom UI. Instead:

✓ DAW provides generic controls automatically:
  - Sliders for float parameters
  - Checkboxes for bool parameters
  - Dropdowns for choice parameters

✓ All parameters are exposed for automation

✓ Fully functional for mixing and production

✓ Smaller binary size (no WebView overhead)

Headless is ideal for:
- Utility plugins (gain, filters, limiters)
- Quick prototyping and testing
- Plugins where visual design isn't critical

You can add custom UI anytime via /improve [PluginName] → Create custom UI
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Press Enter to return to menu...
```

**Helper function: handleCreateCustomUi(pluginName)**

```typescript
function handleCreateCustomUi(pluginName: string) {
  console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Creating Custom UI for ${pluginName}
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

This will upgrade ${pluginName} from headless to WebView UI.

Steps:
1. Design UI mockup (iterative, can refine)
2. Generate WebView integration code
3. Update version to v1.1.0 (minor version bump - new feature)
4. Build, test, and install

This will NOT break existing automation or presets.

Ready to proceed?

1. Yes, create UI - Start ui-mockup workflow
2. No, go back - Return to /improve menu
3. Learn more - What is WebView UI?
4. Other

Choose (1-4): _
  `);

  const choice = getUserInput();

  if (choice === "1") {
    // Invoke ui-mockup skill
    console.log("\n━━━ Invoking ui-mockup skill ━━━\n");

    Skill({ skill: "ui-mockup" });

    // After mockup finalized, invoke gui-agent
    console.log(`
✓ UI mockup finalized

Now integrating WebView UI into ${pluginName}...
    `);

    const mockupPath = findLatestMockup(pluginName);

    if (!mockupPath) {
      console.log("✗ Mockup finalization failed or was cancelled. Returning to menu.");
      return;
    }

    // Read contracts
    const creativeBrief = readFile(`plugins/${pluginName}/.ideas/creative-brief.md`);
    const parameterSpec = readFile(`plugins/${pluginName}/.ideas/parameter-spec.md`);
    const architecture = readFile(`plugins/${pluginName}/.ideas/architecture.md`);

    // Invoke gui-agent (same as plugin-workflow Stage 3)
    const guiResult = Task({
      subagent_type: "gui-agent",
      description: `Add WebView UI to ${pluginName}`,
      prompt: `
Integrate WebView UI for ${pluginName} (upgrading from headless).

**Contracts:**
- creative-brief.md: ${creativeBrief}
- parameter-spec.md: ${parameterSpec}
- architecture.md: ${architecture}
- Mockup: ${mockupPath}

**Tasks:**
1. Read mockup and extract UI structure
2. Generate Source/ui/ directory with WebView resources
3. Update PluginEditor.h/cpp to use WebView (replace minimal editor)
4. Bind all parameters to UI controls
5. Update CMakeLists.txt (add JUCE_WEB_BROWSER=1, binary data)
6. Build and verify
7. Return JSON report with status

This is an upgrade from headless to WebView UI (v1.0.0 → v1.1.0).
      `
    });

    // Parse result
    const report = parseSubagentReport(guiResult);

    if (report.status !== "success") {
      console.log("✗ GUI integration failed. See error above.");
      return;
    }

    // Update version to v1.1.0
    const currentVersion = getCurrentVersion(pluginName); // Read from PLUGINS.md
    const newVersion = bumpMinorVersion(currentVersion); // 1.0.0 → 1.1.0

    // Update .continue-here.md gui_type field
    const handoffPath = `plugins/${pluginName}/.continue-here.md`;
    bash(`sed -i '' 's/^gui_type: headless/gui_type: webview/' "${handoffPath}"`);

    // Update PLUGINS.md table row
    updatePluginStatus(pluginName, "✅ Working");
    updatePluginVersion(pluginName, newVersion);

    // Update NOTES.md status and timeline
    const notesPath = `plugins/${pluginName}/NOTES.md`;
    if (!fileExists(notesPath)) {
      createNotesFile(pluginName, "✅ Working");
    }

    // Update status in NOTES.md
    bash(`sed -i '' 's/^- \\*\\*Current Status:\\*\\* .*$/- **Current Status:** ✅ Working/' "${notesPath}"`);

    // Update version in NOTES.md
    bash(`sed -i '' 's/^- \\*\\*Version:\\*\\* .*$/- **Version:** ${newVersion}/' "${notesPath}"`);

    // Add timeline entry to NOTES.md
    updatePluginTimeline(pluginName, 4, `WebView UI added (v${newVersion}) - Custom interface with visual controls`);

    // CHANGELOG.md entry
    const paramCount = getParameterCount(pluginName);
    updateChangelog(pluginName, newVersion, `
## [${newVersion}] - ${getCurrentDate()}

### Added
- **WebView UI:** Custom interface with visual parameter controls
  - Replaces minimal editor (DAW controls still work)
  - Designed with ui-mockup skill
  - All ${paramCount} parameters bound to UI
  - VU meters and visual feedback added

### Changed
- **Binary size:** Increased due to WebView resources (expected)

### Notes
- Existing automation and presets are unaffected
- DAW generic UI still accessible as fallback
    `);

    // Git commit
    bash(`
git add plugins/${pluginName}/Source/
git add plugins/${pluginName}/.continue-here.md
git add plugins/${pluginName}/CHANGELOG.md
git add PLUGINS.md

git commit -m "$(cat <<'EOF'
feat(${pluginName}): add WebView UI (v${newVersion})

Upgraded from headless to custom WebView interface
All parameters bound to visual controls
Minor version bump (backward compatible)

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
    `);

    // Build and install
    console.log(`\nBuilding ${pluginName} v${newVersion} with WebView UI...`);
    bash(`./scripts/build-and-install.sh ${pluginName}`);

    // Present completion menu
    console.log(`
✓ ${pluginName} v${newVersion} complete

UI upgrade successful:
- Headless (v${currentVersion}) → WebView UI (v${newVersion})
- Custom interface integrated
- All parameters bound to visual controls
- Built and installed

What's next?

1. Test in DAW - Try out your new interface
2. Refine UI styling - Polish visual design
3. Make another improvement
4. Create new plugin
5. Other

Choose (1-5): _
    `);

    const nextChoice = getUserInput();

    if (nextChoice === "1") {
      console.log("\nInstructions for DAW testing:\n1. Load plugin in DAW\n2. Test all UI controls\n3. Verify parameter automation\n\nReturn here when ready.");
    } else if (nextChoice === "2") {
      console.log("\nTo refine UI:\n1. Run /improve ${pluginName}\n2. Describe styling changes\n3. I'll update mockup and regenerate\n");
    } else if (nextChoice === "3") {
      // Re-run /improve
      console.log(`\nRunning /improve ${pluginName} again...`);
      // Restart /improve flow (normal mode, not headless)
    }

  } else if (choice === "2") {
    // Return to /improve main menu
    return;
  } else if (choice === "3") {
    // Explain WebView UI
    console.log(`
WebView UI Explanation
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

WebView UI uses HTML/CSS/JavaScript to create custom plugin interfaces:

✓ Full visual control:
  - Custom layouts, colors, fonts
  - Animations and visual feedback
  - Branded, professional appearance

✓ Rapid iteration:
  - Design in browser first (ui-mockup skill)
  - Iterate on styling without recompiling
  - Modern web development tools

✓ All parameters bound to UI:
  - Knobs, sliders, toggles, dropdowns
  - Two-way sync (UI ↔ DSP)
  - Automation still works

Trade-offs:
- Larger binary size (+1-2MB due to WebView resources)
- 10-15 min to implement (design + integration)
- Slightly higher CPU usage (minimal, unnoticeable)

Headless plugins are faster to build but less visually polished.
WebView plugins have custom branding and professional appearance.

Press Enter to return to menu...
    `);

    getUserInput(); // Wait for Enter
    handleCreateCustomUi(pluginName); // Re-present menu
  }
}
```

**If NOT headless:** Skip to Phase 0.3 (normal flow)

## Phase 0.3: Clarification Questions (If Specific)

**See**: [references/clarification-protocol.md](references/clarification-protocol.md) for 4 targeted questions (what to change, scope, version bump, testing approach). Collect all responses before proceeding to Phase 0.4.

## Phase 0.4: Decision Gate

**Show user what you understand, ask for confirmation:**

```
I understand you want to:
- [Summary of change from Question 1]
- Scope: [Answer from Question 2]
- Version bump: [Answer from Question 3]
- Regression testing: [Answer from Question 4]

Is this correct?

1. Yes, proceed - Continue to Phase 0.45 (Research Detection)
2. No, refine - Ask me follow-up questions
3. No, cancel - Stop the workflow
4. Other

Choose (1-4): _
```

**Handle responses:**
- Option 1 → Proceed to Phase 0.45
- Option 2 → Return to Phase 0.3, ask follow-up questions
- Option 3 → Stop workflow, wait for new instruction
- Option 4 → Collect free-form text, reassess

## Phase 0.45: Research Detection

**MANDATORY**: Scan conversation history for deep-research findings to avoid duplicate investigation.

**See**: [references/research-detection.md](references/research-detection.md) for complete detection algorithm, extraction logic, and decision trees.

**Decision**: If research detected → Skip to Phase 0.9 | If not detected → Continue to Phase 0.5

## Phase 0.5: Investigation (Auto-Tiered)

**Purpose:** Find root causes, prevent band-aid fixes

### Automatic Tier Detection

Analyze request and select tier automatically. Never ask user which tier.

**Detection algorithm** (check in order, first match wins):

| Priority | Condition | Tier |
|----------|-----------|------|
| 1 | Known pattern in troubleshooting/ | Tier 1 (quick) |
| 2 | Keywords: "crash", "performance", "architecture", "all plugins" | Tier 3 (complex, delegate) |
| 3 | Scope: Multiple components OR unclear cause | Tier 3 (complex, delegate) |
| 4 | Scope: Single file + clear symptom | Tier 2 (moderate) |
| 5 | Keywords: "cosmetic", "typo", "rename", "color", "text" | Tier 1 (quick) |
| 6 | Default (if no matches) | Tier 1, escalate if needed |

Log selected tier: "Analyzing issue (quick investigation)..." or "Analyzing issue (deep investigation)..."

### Tier 3: Deep Research (Delegate)

For complex issues (Tier 3 detected), invoke deep-research skill:

```
Complex issue detected. Invoking deep-research skill...
```

Use Skill tool to invoke deep-research with problem context. It performs graduated investigation and returns structured findings.

**See**: [references/investigation-tiers.md](references/investigation-tiers.md) for complete protocols for each tier.

### Present Findings

After investigation (any tier), present findings and wait for approval before implementing.

<critical_sequence phase="backup-verification" enforcement="strict">
## Phase 0.9: Backup Verification

**CRITICAL INVARIANT:** Phase 1 MUST NOT execute until backup verified.
**ENFORCEMENT:** Block execution, halt workflow if backup fails.
**VIOLATION CONSEQUENCE:** Data loss, no rollback path.

**Goal:** Ensure rollback is possible if improvement fails

**Check if backup exists:**

```bash
BACKUP_PATH="backups/${PLUGIN_NAME}/v${CURRENT_VERSION}/"
if [[ ! -d "$BACKUP_PATH" ]]; then
  echo "⚠️ No backup found for v${CURRENT_VERSION}"
  CREATE_BACKUP=true
fi
```

**Create backup if missing using template:**

```bash
# See assets/backup-template.sh for complete script
mkdir -p "backups/${PLUGIN_NAME}/v${CURRENT_VERSION}/"
rsync -a --exclude='build/' --exclude='build.log' \
  "plugins/${PLUGIN_NAME}/" "backups/${PLUGIN_NAME}/v${CURRENT_VERSION}/"
```

**Verify backup integrity:**

```bash
# Use verify-backup.sh script
./scripts/verify-backup.sh "${PLUGIN_NAME}" "${CURRENT_VERSION}"

if [ $? -ne 0 ]; then
  echo "❌ Backup verification failed. Cannot proceed safely."
  echo "Fix backup issues before continuing."
  exit 1
fi
```

**Present verification results:**

```
✓ Backup verified: backups/[PluginName]/v[CurrentVersion]/

- All source files present
- CMakeLists.txt valid
- Dry-run build successful

Rollback available if needed.
```

</critical_sequence>

<phase id="1" name="pre-implementation" dependencies="backup-verification">
## Phase 1: Pre-Implementation Checks

**DEPENDENCY:** MUST NOT execute until Phase 0.9 (Backup Verification) completes.

**Load current state:**

1. Read CHANGELOG.md:

```bash
cat plugins/[PluginName]/CHANGELOG.md
```

Extract current version (e.g., v1.2.3).

2. Read PLUGINS.md entry for additional context.

3. Check recent commits:

```bash
git log --oneline plugins/[PluginName]/ -10
```

**Determine version bump:**

Present choice:

```
Current version: v[X.Y.Z]

What type of change is this?
1. PATCH (v[X.Y.Z] → v[X.Y.Z+1]) - Bug fixes, cosmetic changes
2. MINOR (v[X.Y] → v[X.Y+1]) - New features, enhancements
3. MAJOR (v[X] → v[X+1]) - Breaking changes (presets won't load, parameters changed)

Choose (1-3): _
```

**If Major version selected, warn:**

```
⚠️ Major version bump will break compatibility.

Breaking changes include:
- Changed parameter IDs (presets won't load)
- Removed parameters (sessions will have missing automation)
- Changed state format (existing sessions corrupted)

Are you sure? This should be rare. (y/n): _
```

Calculate new version based on selection.

### Breaking Change Detection

Check for breaking changes BEFORE confirming version bump:

**Breaking if:**
- ❌ Parameter ID renamed (automation breaks)
- ❌ Parameter range changed (saved values invalid)
- ❌ Parameter removed (automation breaks)
- ❌ Parameter type changed (e.g., float → choice)
- ❌ State format changed (old presets won't load)
- ❌ Feature removed (users lose functionality)
- ❌ Public API signature changed

**If breaking changes detected:** Force MAJOR version bump, warn user, require confirmation.

**See**: [references/breaking-changes.md](references/breaking-changes.md) for detailed detection criteria and edge cases.

</phase>

<critical_sequence phase="backup-creation" enforcement="strict">
## Phase 2: Verify Rollback Path

**Baseline backup verified in Phase 0.9. Confirm ready to proceed:**

```
✓ Backup verified: backups/[PluginName]/v[CurrentVersion]/
Ready to implement changes for v[NewVersion]
```

</critical_sequence>

<phase id="3" name="implementation" dependencies="backup-creation">
## Phase 3: Implementation

**DEPENDENCY:** MUST NOT execute until Phase 2 (Backup Creation) completes.
**SAFETY:** If implementation fails, rollback path guaranteed by Phase 2 backup.

**Execute the change:**

1. Modify source files according to investigation findings
2. Update build configuration if needed (CMakeLists.txt)
3. Adjust UI if required (PluginEditor.cpp)
4. Update parameter definitions if needed (PluginProcessor.cpp)

**Follow best practices:**

- Real-time safety in processBlock
- No allocations in audio thread
- Thread-safe parameter access
- JUCE API correctness

**Log changes as you go for CHANGELOG.**

</phase>

## Phase 4: CHANGELOG Update

**Add version entry at top of CHANGELOG.md:**

### Template Structure

```markdown
## [VERSION] - YYYY-MM-DD

### Added (for new features)
- **Feature name:** Brief description
  - Technical detail
  - Impact: What this enables

### Changed (for changes to existing functionality)
- **Component name:** What changed
  - Before: Previous behavior
  - After: New behavior

### Fixed (for bug fixes)
- **Issue:** What was broken
  - Root cause: Technical explanation
  - Solution: How it was fixed

### Breaking Changes (MAJOR versions only)
- **API change:** What's incompatible
  - Migration: How to update existing uses
```

### Section Usage by Version Type

- **PATCH (0.0.X):** Use "Fixed" section primarily
- **MINOR (0.X.0):** Use "Added" and/or "Changed" sections
- **MAJOR (X.0.0):** Include "Breaking Changes" and "Migration Notes"

**Always include:**
- Date in ISO format (YYYY-MM-DD)
- Root cause for fixes (from Phase 0.5 investigation)
- Testing notes (regression test results if Phase 5.5 ran)

**See**: [references/changelog-format.md](references/changelog-format.md) for detailed examples and best practices.

<delegation_rule target="build-automation" required="true">
## Phase 5: Build and Test

**DELEGATION:** MUST invoke build-automation skill for all build operations.
**REASON:** Centralized build logic, 7-phase pipeline with verification.

**Delegate to build-automation skill:**

```
Invoking build-automation skill to build and install updated plugin...
```

build-automation will:

- Run build script: `scripts/build-and-install.sh [PluginName]` (full build)
- Build VST3 and AU formats in parallel
- Install to system folders
- Clear DAW caches
- Verify installation

If build succeeds:

- build-automation displays success message with installation paths
- Returns control to plugin-improve
- Proceed to Phase 5, step 2 (Run tests)

If build fails:

- build-automation presents 4-option failure protocol:
  1. Investigate (troubleshooter agent)
  2. Show build log
  3. Show code
  4. Wait for manual fix
- After resolution and successful retry, returns to plugin-improve
- Proceed to Phase 5, step 2 (Run tests)

**Note:** Build failure handling is entirely managed by build-automation skill. plugin-improve does not need custom build error menus.

**2. Run tests:**

Invoke `plugin-testing` skill (Phase 1b Task 8):

Present test method choice:

```
Build successful. How would you like to test?

1. Automated stability tests (if Tests/ exists)
2. Build and run pluginval (recommended)
3. Manual DAW testing checklist
4. Skip testing (not recommended)

Choose (1-4): _
```

If tests fail, present investigation options.

</delegation_rule>

<validation_gate gate="regression-tests" required="conditional">
## Phase 5.5: Regression Testing

**GATE CONDITION:** Conditional - only runs if both conditions met
**GATE FAILURE:** Present rollback options, require user decision

### Decision Tree

```
Does plugin-testing skill exist?
├─ NO → Skip regression tests
│       Warn: "Manual regression testing required (plugin-testing skill not found)"
│       Add to CHANGELOG: "Manual regression testing required"
│       Continue to Phase 6
│
└─ YES → Does baseline backup exist (backups/[Plugin]/v[baseline]/)?
    ├─ NO → Skip regression tests
    │       Warn: "No baseline backup found for v[baseline]"
    │       Add to CHANGELOG: "Manual regression testing required (no baseline)"
    │       Continue to Phase 6
    │
    └─ YES → Run regression tests
            1. Build baseline version
            2. Run tests on baseline
            3. Run tests on current version
            4. Compare results
            5. Present findings with decision menu
```

### Regression Test Process (if both conditions met)

**1. Determine baseline version:**
- If improving v1.0.0 → v1.1.0, baseline is v1.0.0
- Baseline path: `backups/[Plugin]/v[baseline]/`

**2. Build baseline version:**
```bash
cd backups/[Plugin]/v[baseline]/
../../scripts/build-and-install.sh --no-install
```

**3. Run tests on baseline:**
- Invoke plugin-testing skill on baseline build
- Capture results: BASELINE_RESULTS

**4. Run tests on current version:**
- Invoke plugin-testing skill on new build
- Capture results: CURRENT_RESULTS

**5. Compare and present:**

See `references/regression-testing.md` for complete RegressionReport interface and comparison logic.

**Quick summary:**
- Collect RegressionReport (build, load, parameter, audio tests)
- Analyze failures: critical → rollback, warnings → review, pass → deploy
- Present results with decision menu

**If regression tests fail, present rollback options before proceeding.**

</validation_gate>

## Phase 6: Git Workflow

**Stage changes:**

```bash
git add plugins/[PluginName]/
git add backups/[PluginName]-v[X.Y.Z]-[timestamp]/  # Include backup in git
```

**Commit with conventional format:**

```bash
# Format: improve: [PluginName] v[X.Y.Z] - [brief description]
# Example: improve: MicroGlitch v1.3.0 - add preset system

git commit -m "improve: [PluginName] v[X.Y.Z] - [description]"
```

**Tag release:**

```bash
git tag -a "v[X.Y.Z]" -m "[PluginName] v[X.Y.Z]"
```

Note: Display git commands for user to run manually. Do not execute git commit or git push.

**Confirm git ready:**

```
✓ Changes staged for commit
✓ Tag ready: v[X.Y.Z]

Git commit message:
  improve: [PluginName] v[X.Y.Z] - [description]

You can commit these changes when ready.
```

<delegation_rule target="plugin-lifecycle" required="false">
## Phase 7: Installation (Optional)

**DELEGATION:** If user requested installation, invoke plugin-lifecycle skill.
**REASON:** Centralized installation logic with cache clearing and verification.

**If user requested installation:**

Invoke `plugin-lifecycle` skill:

```
Installing [PluginName] v[X.Y.Z]...
```

**Update state files:**

After plugin-lifecycle completes installation:

1. Update PLUGINS.md table row:
   - Version: [X.Y.Z]
   - Last Updated: [YYYY-MM-DD]
   - Status: 📦 Installed (if previously ✅ Working)

2. Update NOTES.md:
   - Version: [X.Y.Z]
   - Status: 📦 Installed
   - Add timeline entry: "Installed to system folders (VST3 + AU)"

</delegation_rule>

<checkpoint_protocol>
## Phase 8: Completion

**MUST present numbered decision menu using inline format (NOT AskUserQuestion tool)**

**Present numbered decision menu (inline format):**

```
✓ [PluginName] v[X.Y.Z] complete

What's next?
1. Test in DAW (recommended)
2. Make another improvement
3. Create new plugin
4. Document this change
5. Other

Choose (1-5): _
```

**Handle responses:**

- Option 1 → Provide manual testing guidance
- Option 2 → Ask what to improve, restart workflow
- Option 3 → Suggest `/dream` or `/implement`
- Option 4 → Suggest creating documentation
- Option 5 → Ask what they'd like to do

</checkpoint_protocol>

## Version History

**Phase 7 enhancements (2025-11):**
- Regression testing integration (Phase 5.5)
- Enhanced changelog format (Phase 4)
- Backup verification protocol (Phase 0.9)
- One-command rollback mechanism
- Breaking change detection

**See:** `architecture/17-testing-strategy.md`, `scripts/verify-backup.sh`, `references/regression-testing.md`

## Integration Points

**Invoked by:**

- `/improve` command
- Natural language: "Fix [plugin]", "Add [feature] to [plugin]"
- `plugin-ideation` skill (after improvement brief)

**Invokes:**

- `plugin-ideation` skill (if vague request, user chooses brainstorm)
- `deep-research` skill (Tier 3 investigation) - Phase 7
- `build-automation` skill (building) - Phase 4
- `plugin-testing` skill (validation)
- `plugin-lifecycle` skill (installation)

**Updates:**

- CHANGELOG.md (adds version entry)
- PLUGINS.md (table row: version, last updated, status)
- plugins/[Name]/NOTES.md (status, version, timeline entries, known issues)
- Source files (implementation changes)

**Creates:**

- Backup in `backups/[PluginName]-v[X.Y.Z]-[timestamp]/`
- Git tag `v[X.Y.Z]`

## Error Handling

**Build failure:**
Present investigation menu, wait for user decision.

**Test failure:**
Present investigation menu, don't proceed to installation.

**Breaking change detected:**
Warn user, require confirmation, force MAJOR version.

**Backup creation fails:**
STOP immediately, don't proceed with changes.

**Git operations fail:**
Log warning, continue (non-critical).

## Success Criteria

Improvement is successful when:

- Root cause investigated (not band-aid fix)
- Backup created before changes
- Changes implemented correctly
- CHANGELOG updated with version entry
- Build succeeds without errors
- Tests pass
- Git staged with conventional commit message
- PLUGINS.md table row updated (version, status, last updated)
- NOTES.md updated (status, version, timeline entry)
- User knows how to rollback if needed
