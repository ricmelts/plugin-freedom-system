# Stage 6: Validation

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher after Stage 5 completion
**Purpose:** Final validation, ready to install and use

---

**Goal:** Ready to install and use

**Duration:** 10-20 minutes

**Preconditions:**

- Stages 0-5 complete
- Plugin compiles successfully
- Automated tests pass (if run)

**Actions:**

**CRITICAL: Always rebuild after Stage 5 completion**

Stage 5 modifies source code (WebView UI integration, parameter bindings). Without rebuilding, Stage 6 will install stale binaries from Stage 4 that lack GUI changes.

1. Rebuild with Stage 5 changes:

```bash
# Rebuild from root directory (matches foundation-agent build location)
echo "Rebuilding ${PLUGIN_NAME} with Stage 5 GUI changes..."

cmake --build build --config Release \
  --target ${PLUGIN_NAME}_VST3 \
  --target ${PLUGIN_NAME}_AU \
  --parallel

if [ $? -ne 0 ]; then
  echo "❌ Build failed after Stage 5 changes"
  echo "Check logs/[PluginName]/build_*.log for errors"
  exit 1
fi

echo "✓ Build complete with Stage 5 WebView integration"

# Verify binaries are fresh (newer than source files)
LATEST_SOURCE=$(find plugins/${PLUGIN_NAME}/Source -type f -exec stat -f "%m" {} \; 2>/dev/null | sort -n | tail -1)
VST3_BINARY="build/plugins/${PLUGIN_NAME}/${PLUGIN_NAME}_artefacts/Release/VST3/${PRODUCT_NAME}.vst3/Contents/MacOS/${PRODUCT_NAME}"

if [ -f "$VST3_BINARY" ]; then
  BINARY_TIME=$(stat -f "%m" "$VST3_BINARY" 2>/dev/null)

  if [ -n "$BINARY_TIME" ] && [ -n "$LATEST_SOURCE" ] && [ $BINARY_TIME -lt $LATEST_SOURCE ]; then
    echo "❌ ERROR: Binary is older than source files"
    echo "   Binary timestamp: $(date -r $BINARY_TIME '+%Y-%m-%d %H:%M:%S')"
    echo "   Latest source:    $(date -r $LATEST_SOURCE '+%Y-%m-%d %H:%M:%S')"
    echo "   This indicates a build system bug - rebuild should have created fresh binary"
    exit 1
  fi

  echo "✓ Binary timestamp verification passed"
  echo "  Binary built at: $(date -r $BINARY_TIME '+%Y-%m-%d %H:%M:%S')"
else
  echo "⚠️  VST3 binary not found (expected location: $VST3_BINARY)"
  echo "   Build may have failed or used different output path"
  exit 1
fi
```

**Git commit:**

After successful rebuild:

```bash
# No source changes to commit (rebuild only updates binaries in build/)
# But log the rebuild event
echo "Stage 6: Rebuild complete with Stage 5 changes" >> logs/${PLUGIN_NAME}/build_$(date +%Y%m%d).log
```

2. Create factory presets:

```bash
mkdir -p plugins/[PluginName]/Presets/
```

Create 3-5 preset files showcasing plugin capabilities.

**Preset format (.preset or .xml):**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<preset name="[PresetName]">
  <param id="[paramID]" value="[value]"/>
  <param id="[paramID]" value="[value]"/>
  ...
</preset>
```

3. Invoke plugin-testing skill:

Present test method choice:

```
How would you like to test [PluginName]?

1. Automated stability tests (if Tests/ directory exists)
2. Build and run pluginval (recommended)
3. Manual DAW testing checklist
4. Skip testing (not recommended)

Choose (1-4): _
```

If tests fail, STOP and wait for fixes.

4. Generate CHANGELOG.md:

**Format:**

```markdown
# Changelog

All notable changes to [PluginName] will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.0] - [YYYY-MM-DD]

### Added

- Initial release
- [Feature 1]
- [Feature 2]
- [Parameter 1]: [Description]
- [Parameter 2]: [Description]

### Audio Processing

- [DSP description]

### User Interface

- [UI description]

### Validation

- Passed pluginval strict validation
- Tested in [DAW names]
```

5. Invoke validator subagent:

Call validator to verify Stage 6 completion:

```typescript
const validation = Task({
  subagent_type: "validator",
  description: `Validate ${pluginName} Stage 6`,
  prompt: `
Validate Stage 6 completion for ${pluginName}.

**Stage:** 6
**Plugin:** ${pluginName}
**Contracts:**
- parameter-spec.md: [paste content or "not applicable"]
- architecture.md: [paste content or "not applicable"]
- plan.md: [paste content]

**Expected outputs for Stage 6:**
- CHANGELOG.md exists in Keep a Changelog format
- Version 1.0.0 for initial release
- Presets/ directory has 3+ preset files
- pluginval passed (or skipped with reason)
- PLUGINS.md updated to ✅ Working

Return JSON validation report with status, checks, and recommendation.
  `
})

const report = JSON.parse(validation)

if (report.status === "FAIL") {
  // Present validation failure menu
  presentValidationFailure(report)
  // Options: Fix issues / Continue anyway / Pause
  // Wait for user choice before proceeding
}
```

6. Update PLUGINS.md:

Call `updatePluginStatus(pluginName, "✅ Working")`.

Add final fields to entry:
```markdown
**Version:** 1.0.0
**Completed:** [YYYY-MM-DD]

**Validation:**
- ✓ Factory presets: [N] presets created
- ✓ CHANGELOG.md: Generated in Keep a Changelog format
- ✓ Pluginval: [Passed | Skipped (no build)]

**Formats:** VST3, AU (if built)
```

Add timeline entry: `updatePluginTimeline(pluginName, 6, "Validation complete")`.

7. **CRITICAL: Delete .continue-here.md handoff file**

**Purpose:** Remove workflow state when plugin is complete. Prevents stale handoff pollution.

Call `deleteHandoff(pluginName)`:

```bash
# MANDATORY cleanup at workflow completion
HANDOFF_FILE="plugins/${PLUGIN_NAME}/.continue-here.md"

if [ -f "$HANDOFF_FILE" ]; then
  rm "$HANDOFF_FILE"
  echo "✓ Handoff file deleted (workflow complete)"
else
  echo "⚠️  Handoff file already deleted"
fi
```

**Enforcement:** This step is MANDATORY. If handoff file is not deleted:
- Future /continue commands will attempt to resume completed workflow
- verifyStateIntegrity() will detect stale handoff and auto-cleanup
- PLUGINS.md will show plugin as complete but handoff suggests in-progress

**Git commit:**

Call `commitStage(pluginName, 6, "validation complete")`.

This commits:
- `plugins/[PluginName]/Presets/`
- `plugins/[PluginName]/CHANGELOG.md`
- `PLUGINS.md`

**IMPORTANT:** The `.continue-here.md` file is deleted BEFORE commit, so it will NOT be in the commit. The deletion itself is the state change (handoff → no handoff = workflow complete).

8. Auto-install plugin:

Invoke plugin-lifecycle skill via Skill tool:

```typescript
Skill({
  skill: "plugin-lifecycle"
})
```

The plugin-lifecycle skill will:
- Verify Release binaries exist
- Extract PRODUCT_NAME from CMakeLists.txt
- Remove old versions
- Copy VST3 and AU to system folders
- Set permissions (755)
- Clear DAW caches
- Verify installation
- Update PLUGINS.md status to 📦 Installed

If installation fails, plugin-lifecycle will present error handling options. If successful, it returns control here.

**Post-installation decision menu:**

```
✓ Plugin installed to system folders.

What's next?
1. Test in DAW (recommended)
2. Make improvements
3. Create new plugin
4. Other

Choose (1-4): _
```

**Handle responses:**

- Option 1 → Provide instructions for manual DAW testing
- Option 2 → Invoke `plugin-improve` skill via Skill tool
- Option 3 → Invoke `plugin-ideation` skill via Skill tool
- Option 4 → Ask what they'd like to do

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
