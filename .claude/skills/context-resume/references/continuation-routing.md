# Continuation Routing

**Context:** This file is part of the context-resume skill.
**Invoked by:** After user confirms continuation from summary
**Purpose:** Route to appropriate continuation skill based on handoff content

---

## Step 4a: Determine Routing

Based on handoff content and user selection, route to appropriate skill:

### Workflow resume (stage = 0-6)

```
User chose: Continue Stage [N]

Routing to: plugin-workflow skill
Parameters:
  - plugin_name: [PluginName]
  - resume_from: stage [N]
  - phase: [M] (if phased)
```

Invoke plugin-workflow skill with resume context.

plugin-workflow will:
- Read handoff file for context
- Skip completed stages
- Continue from current stage/phase
- Update handoff file as it progresses

### Ideation resume (stage = "ideation")

```
User chose option 1 (Start implementation) or 2 (Refine mockup)

If option 1:
  Routing to: plugin-workflow skill (Stage 0)

If option 2:
  Routing to: plugin-ideation skill (improvement mode) or ui-mockup skill
```

### Mockup resume (stage = "mockup")

```
User chose option 1 (Finalize) or 2 (Iterate)

If option 1 (Finalize):
  - Generate parameter-spec.md if not exists
  - Offer to start Stage 5 (GUI) with plugin-workflow
  - Or offer to start Stage 0 (Research) for new implementation

If option 2 (Iterate):
  Routing to: ui-mockup skill (creates v[N+1])
```

### Improvement resume (stage = "improvement_planning")

```
User chose: Start implementation

Routing to: plugin-improve skill
Parameters:
  - plugin_name: [PluginName]
  - improvement_file: [from YAML]
```

## Step 4b: Load Context Files

Before invoking continuation skill, load relevant context:

**For workflow resume:**

```bash
# Read all contract files
cat "plugins/$PLUGIN_NAME/.ideas/creative-brief.md"
cat "plugins/$PLUGIN_NAME/.ideas/parameter-spec.md"
cat "plugins/$PLUGIN_NAME/.ideas/architecture.md"
cat "plugins/$PLUGIN_NAME/.ideas/plan.md"

# Show recent commits
git log --oneline plugins/$PLUGIN_NAME/ -5

# Read source files mentioned in handoff (if any)
cat "plugins/$PLUGIN_NAME/Source/PluginProcessor.cpp"
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

## Step 4c: Invoke Continuation Skill

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

---

**Return to:** Main context-resume orchestration in `SKILL.md`
