<objective>
Implement workflow orchestration fixes to ensure proper subagent invocation during plugin development. The current issue: when /continue loads a handoff file, Claude implements directly instead of invoking the proper subagent chain through skills. This fix makes handoff files prescriptive (not just informational) and ensures explicit skill invocation.

This matters because the dispatcher pattern with subagents ensures clean separation of concerns, proper git commits between phases, and consistent workflow execution.
</objective>

<context>
Plugin Freedom System architecture with:
- Slash commands in .claude/commands/
- Skills in .claude/skills/
- Subagents in .claude/agents/
- Handoff files (.continue-here.md) for pause/resume

Current workflow: /continue → context-resume skill → plugin-workflow skill → subagent invocation
Problem: Claude bypasses skills and implements directly in main thread
Solution: Make handoff files prescriptive and slash commands explicit about skill invocation
</context>

<requirements>
1. Update handoff file format to include prescriptive fields
2. Modify /continue command to explicitly require Skill tool usage
3. Update context-resume skill to read and follow handoff instructions
4. Update plugin-workflow skill to handle resume with orchestration mode
5. Ensure the entire chain uses Skill/Task tools, not manual implementation
</requirements>

<implementation>
Thoroughly analyze the skill invocation chain and consider how each component should hand off to the next. The key principle: handoff files become executable instructions, not just state descriptions.

## Part 1: Handoff File Format Update

Update the handoff template to include:
- `orchestration_mode: true` - Flag indicating dispatcher behavior required
- `next_action: invoke_[subagent]_agent` - Explicit subagent to invoke
- `next_phase: X.Y` - Which phase/stage to implement
- Add warning header: "⚠️ ORCHESTRATION MODE ACTIVE"

## Part 2: /continue Command Update (.claude/commands/continue.md)

Modify line 8 and after to be prescriptive:
```markdown
When user runs `/continue [PluginName?]`, YOU MUST invoke the context-resume skill using the Skill tool.

IMPORTANT: Use this exact invocation:
```
Skill({ skill: "context-resume", plugin: "[PluginName]" })
```

DO NOT manually read handoff files or present summaries. The context-resume skill handles all of this.
```

## Part 3: context-resume Skill Updates

In `.claude/skills/context-resume/references/continuation-routing.md`, add section 4a-1:
- Check orchestration_mode flag in handoff
- If true, read next_action and next_phase
- Present orchestration mode notice to user
- After confirmation, invoke specified skill/subagent
- DO NOT implement directly

## Part 4: plugin-workflow Skill Updates

In `.claude/skills/plugin-workflow/SKILL.md`, add resume entry point logic:
- When invoked with resume_from parameter
- Check handoff for orchestration_mode
- If true, follow next_action instruction explicitly
- Use Task tool to invoke specified subagent

## Part 5: Example Handoff Update

Update existing DriveVerb handoff to demonstrate new format:
```yaml
---
plugin: DriveVerb
stage: 4.3
status: in_progress
last_updated: 2025-11-11T17:00:00Z
complexity_score: 5.0
phased_implementation: true
orchestration_mode: true      # NEW
next_action: invoke_dsp_agent  # NEW
next_phase: 4.4               # NEW
---
```
</implementation>

<files_to_modify>
1. `.claude/commands/continue.md` - Make skill invocation explicit
2. `.claude/skills/context-resume/references/continuation-routing.md` - Add orchestration check
3. `.claude/skills/plugin-workflow/SKILL.md` - Add resume entry point with orchestration
4. `.claude/skills/plugin-workflow/references/stage-dispatcher.md` - Update handoff generation
5. `plugins/DriveVerb/.continue-here.md` - Update to new format as example
</files_to_modify>

<verification>
After implementation:
1. Check that /continue.md explicitly says "USE SKILL TOOL"
2. Verify handoff files include orchestration fields
3. Confirm context-resume checks orchestration_mode
4. Verify plugin-workflow respects next_action
5. Test with DriveVerb: /continue should invoke context-resume skill, which should invoke plugin-workflow, which should invoke dsp-agent
</verification>

<success_criteria>
- Handoff files contain prescriptive routing instructions
- Slash commands explicitly require Skill tool usage
- Skills check and follow orchestration instructions
- The complete chain: /continue → context-resume → plugin-workflow → subagent works without manual implementation
- DriveVerb example demonstrates the new format
</success_criteria>
