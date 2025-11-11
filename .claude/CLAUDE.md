# PLUGIN FREEDOM SYSTEM - Plugin Development System

## System Components
- **Scripts**: `scripts/` - Build and installation automation
  - build-and-install.sh - Centralized build automation (7-phase pipeline: validate, build, install, verify)
  - verify-backup.sh - Backup integrity verification (Phase 7)
- **Skills**: `.claude/skills/` - Each skill follows Anthropic's pattern with `SKILL.md`, `references/`, and `assets/` subdirectories
  - plugin-workflow, plugin-ideation, plugin-improve (enhanced with regression testing), ui-mockup, context-resume, plugin-testing, plugin-lifecycle, build-automation, troubleshooting-docs, deep-research, design-sync
- **Subagents**: `.claude/agents/` - foundation-agent, shell-agent, dsp-agent, gui-agent, validator, troubleshooter
- **Commands**: `.claude/commands/` - /dream, /implement, /improve, /continue, /test, /install-plugin, /uninstall, /show-standalone, /troubleshoot-juce, /doc-fix, /research, /sync-design
- **Hooks**: `.claude/hooks/` - Validation gates (PostToolUse, SubagentStop, UserPromptSubmit, Stop, PreCompact, SessionStart)
- **Knowledge Base**: `troubleshooting/` - Dual-indexed (by-plugin + by-symptom) problem solutions

## Contracts (Single Source of Truth)
- `plugins/[Name]/.ideas/` - creative-brief.md (vision), parameter-spec.md (parameters), architecture.md (DSP design), plan.md (implementation strategy)
- **State**: PLUGINS.md (all plugins), .continue-here.md (active workflow)
- **Templates**: Contract templates stored in skill assets (`.claude/skills/*/assets/`)

## Key Principles
1. **Contracts are immutable during implementation** - All stages reference the same specs (zero drift)
2. **Dispatcher pattern** - Each subagent runs in fresh context (no accumulation)
3. **Discovery through play** - Features found via slash command autocomplete and decision menus
4. **Instructed routing** - Commands expand to prompts, Claude invokes skills

## Workflow Entry Points
- New plugin: `/dream` → `/implement`
- Resume work: `/continue [PluginName]`
- Modify existing: `/improve [PluginName]`
- Test plugin: `/test [PluginName]`

## Implementation Status
- ✓ Phase 0: Foundation & Contracts (complete)
- ✓ Phase 1: Discovery System (complete)
- ✓ Phase 2: Workflow Engine (complete)
- ✓ Phase 3: Implementation Subagents (complete)
- ✓ Phase 4: Build & Troubleshooting System (complete)
- ✓ Phase 5: Validation System (complete - hybrid validation operational)
- ✓ Phase 6: WebView UI System (complete)
- ✓ Phase 7: Polish & Enhancement (complete - feedback loop operational)

## Phase 7 Components (Polish & Enhancement)

### Skills
- **plugin-lifecycle** (`.claude/skills/plugin-lifecycle/`) - Installation/uninstallation management
- **design-sync** (`.claude/skills/design-sync/`) - Mockup ↔ brief validation, drift detection
- **deep-research** (`.claude/skills/deep-research/`) - Multi-level problem investigation (3-level graduated protocol)
- **troubleshooting-docs** (`.claude/skills/troubleshooting-docs/`) - Knowledge base capture with dual-indexing
- **plugin-improve** (`.claude/skills/plugin-improve/`) - Version management with regression testing (enhanced)

### Commands
- `/install-plugin [Name]` - Install to system folders
- `/uninstall [Name]` - Remove from system folders
- `/sync-design [Name]` - Validate design alignment (mockup ↔ brief)
- `/research [topic]` - Deep investigation (3-level protocol)
- `/doc-fix` - Document solved problems
- `/improve [Name]` - Fix bugs or add features (enhanced with regression testing)

### Knowledge Base
- `troubleshooting/build-failures/` - Build and compilation errors
- `troubleshooting/runtime-issues/` - Crashes, exceptions, performance issues
- `troubleshooting/gui-issues/` - UI layout and rendering problems
- `troubleshooting/api-usage/` - JUCE API misuse and migration issues
- `troubleshooting/dsp-issues/` - Audio processing problems
- `troubleshooting/parameter-issues/` - APVTS and state management
- `troubleshooting/validation-problems/` - pluginval failures
- `troubleshooting/patterns/` - Common patterns and solutions

### Scripts
- `scripts/build-and-install.sh` - Build automation (supports --uninstall)
- `scripts/verify-backup.sh` - Backup integrity verification

## Feedback Loop

The complete improvement cycle:
```
Build → Test → Find Issue → Research → Improve → Document → Validate → Deploy
    ↑                                                                      ↓
    └──────────────────────────────────────────────────────────────────────┘
```

- **deep-research** finds solutions (3-level graduated protocol: Quick → Moderate → Deep)
- **plugin-improve** applies changes (with regression testing and backup verification)
- **troubleshooting-docs** captures knowledge (dual-indexed for fast lookup)
- **design-sync** prevents drift (validates contracts before implementation)
- **plugin-lifecycle** manages deployment (install/uninstall with cache clearing)
