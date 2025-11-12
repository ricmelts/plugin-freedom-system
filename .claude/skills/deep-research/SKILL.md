---
name: deep-research
description: Multi-agent parallel investigation for complex JUCE problems
model: claude-opus-4-1-20250805
allowed-tools:
  - Read # Local troubleshooting docs
  - Grep # Search docs
  - Task # Spawn parallel research subagents (Level 3)
  - WebSearch # Web research (Level 2-3)
preconditions:
  - Problem is non-trivial (quick Context7 lookup insufficient)
extended-thinking: false # Note: Level 3 enforcement happens via <delegation_rule> gate below, not this YAML setting
timeout: 3600 # 60 min max

<extended_thinking_gate>
IF level = 3:
  MUST enable extended-thinking with 15,000 token budget
  MUST verify activation before spawning subagents
ELSE IF level IN [1, 2]:
  MUST NOT enable extended-thinking
  Use standard reasoning for quick/moderate research
</extended_thinking_gate>
---

# deep-research Skill

**Purpose:** Multi-level autonomous investigation for complex JUCE plugin development problems using graduated research depth protocol.

## Overview

Multi-level autonomous investigation (Level 1: 5 min local → Level 2: 30 min web → Level 3: 60 min parallel). Stops at first confident answer. User controls depth.

---

<invariant type="read_only_protocol" severity="critical">
This skill is READ-ONLY and ADVISORY ONLY.

NEVER:
- Edit code files via Edit/Write/NotebookEdit tools
- Run build commands (npm, CMake, compiler)
- Modify contracts or configurations
- Implement any solutions

ONLY:
- Search for information (Grep, Glob, Read, WebSearch, WebFetch)
- Analyze existing code and documentation
- Generate reports and recommendations
- Present findings with decision menus

ALWAYS delegate implementation to plugin-improve skill via handoff protocol.
Violation of this invariant breaks the system architecture.
</invariant>

<handoff_protocol target_skill="plugin-improve">
<trigger>User selects option 1 ("Apply solution") from decision menu</trigger>

<deep_research_action>
When user selects "Apply solution":
1. Output confirmation: "User selected: Apply solution. Invoking plugin-improve skill..."
2. Use Skill tool to invoke plugin-improve directly
3. STOP execution (no further implementation)
</deep_research_action>

<context_passing>
plugin-improve reads research findings from conversation history.
plugin-improve skips Phase 0.5 investigation (already completed by deep-research).
</context_passing>

<enforcement>
deep-research NEVER implements. Only plugin-improve implements.
deep-research MUST invoke plugin-improve via Skill tool.
</enforcement>
</handoff_protocol>

**Note:** deep-research invokes plugin-improve directly via the Skill tool when user selects "Apply solution". This ensures the handoff happens automatically without requiring main conversation orchestration.

**Why separation matters:**

- Research uses Opus + extended thinking (expensive)
- Implementation needs codebase context (different focus)
- Clear decision gate between "here are options" and "making changes"
- Research can't break anything (safe exploration)

---

## Entry Points

**Invoked by:**

- troubleshooter agent (Level 4 investigation)
- User manual: `/research [topic]`
- build-automation "Investigate" option
- Natural language: "research [topic]", "investigate [problem]"

**Entry parameters:**

- **Topic/question**: What to research
- **Context** (optional): Plugin name, stage, error message
- **Starting level** (optional): Skip to Level 2/3 if explicitly requested

---

<critical_sequence name="graduated_research_protocol" skip_prevention="strict">
<level number="1" name="quick_check" max_time_minutes="10">
  <goal>Find quick answer from local knowledge base or JUCE API docs</goal>

  <required_steps>
    1. Search local troubleshooting docs
    2. Quick Context7 lookup
    3. Assess confidence
  </required_steps>

  <exit_conditions>
    IF confidence = HIGH: Present decision menu, ALLOW user to proceed
    ELSE: MUST escalate to Level 2 (no skip option)
  </exit_conditions>
</level>

<level number="2" name="moderate_investigation" max_time_minutes="30">
  <prerequisite>Level 1 completed OR user manually started at Level 2</prerequisite>

  <goal>Deep-dive JUCE docs, forums, GitHub for authoritative answers</goal>

  <required_steps>
    1. Context7 deep-dive (advanced queries, cross-references)
    2. JUCE forum search via WebSearch
    3. GitHub issue search (juce-framework/JUCE)
    4. Synthesize findings from multiple sources
  </required_steps>

  <exit_conditions>
    IF confidence IN [HIGH, MEDIUM]: Present decision menu
    ELSE IF confidence = LOW OR novel_problem = true: MUST escalate to Level 3
  </exit_conditions>
</level>

<level number="3" name="deep_research" max_time_minutes="60">
  <prerequisite>Level 2 completed OR user manually started at Level 3</prerequisite>

  <goal>Parallel subagent investigation for novel/complex problems</goal>

  <model_requirements>
    MUST use: claude-opus-4-1-20250805
    MUST enable: extended-thinking with 15k budget
    NEVER use: Sonnet (insufficient synthesis capacity)
  </model_requirements>

  <required_steps>
    1. Switch to Opus + extended thinking
    2. Identify 2-3 research approaches
    3. Spawn parallel subagents via Task tool (NOT serial)
    4. Synthesize findings with extended thinking
    5. Generate comprehensive report
  </required_steps>

  <exit_conditions>
    ALWAYS: Present decision menu (no further escalation possible)
  </exit_conditions>
</level>

<enforcement_rules>
- NEVER skip Level 1 unless user explicitly requests starting at Level 2/3
- NEVER use serial investigation at Level 3 (must be parallel)
- NEVER use Sonnet at Level 3 (must be Opus)
- NEVER forget extended thinking at Level 3
- NEVER implement solutions (always delegate to plugin-improve)
</enforcement_rules>
</critical_sequence>

## Level 1: Quick Check (5-10 min, Sonnet, no extended thinking)

Search local docs + Context7 for known solutions.

See `references/research-protocol.md#level-1-quick-check` for detailed process.

<success_criteria level="1">
- Found exact match in local docs OR clear API documentation
- HIGH confidence (80%+)
- Solution directly applicable without modification
</success_criteria>

<critical_sequence name="level1_quick_check" enforce_order="strict">
<step number="1" required="true">Parse research topic (keywords, JUCE components, problem type)</step>
<step number="2" required="true">Search local troubleshooting docs</step>
<step number="3" required="true">Check Context7 JUCE docs</step>
<step number="4" required="true">Assess confidence (HIGH/MEDIUM/LOW)</step>

<decision_gate name="level1_outcome" enforce="strict">
IF confidence = HIGH:
  THEN present decision menu with solution
  WAIT for user selection

ELSE IF confidence IN [MEDIUM, LOW]:
  THEN auto-escalate to Level 2
  OUTPUT notification: "Level 1: No confident solution found. Escalating to Level 2..."
  PROCEED to Level 2 process
</decision_gate>
</critical_sequence>

---

## Level 2: Moderate Investigation (15-30 min, Sonnet, no extended thinking)

Deep-dive Context7, JUCE forums, GitHub for authoritative answers.

See `references/research-protocol.md#level-2-moderate-investigation` for detailed process.

<success_criteria level="2">
- Found authoritative answer verified by multiple sources
- MEDIUM-HIGH confidence (60%+)
- Solution adaptable with minor modifications
</success_criteria>

<decision_gate name="level2_outcome" enforce="strict">
IF confidence IN [HIGH, MEDIUM]:
  THEN present decision menu with solution options
  OPTIONS: Apply solution | Review findings | Escalate to Level 3 | Other
  WAIT for user selection

ELSE IF confidence = LOW:
  THEN auto-escalate to Level 3
  OUTPUT notification: "Level 2: Low confidence. Escalating to Level 3 (deep research)..."
  PROCEED to Level 3 process

ELSE IF problem_type = "novel_implementation":
  THEN auto-escalate to Level 3
  OUTPUT notification: "Level 2: Novel problem requires parallel investigation. Escalating to Level 3..."
  PROCEED to Level 3 process
</decision_gate>

---

## Level 3: Deep Research (30-60 min, Opus, extended thinking 15k budget)

**Goal:** Comprehensive investigation with parallel subagent research for novel/complex problems

<delegation_rule level="3" name="level3_requirements" enforce="strict">
<model_switch>
MUST switch to: claude-opus-4-1-20250805
MUST enable: extended-thinking with 15,000 token budget
MUST set timeout: 3600 seconds (60 min)
NEVER use: Sonnet at Level 3 (insufficient capacity for synthesis)
</model_switch>

<subagent_requirement>
MUST spawn 2-3 parallel research subagents via Task tool.
MUST invoke ALL subagents in PARALLEL (single response with multiple Task calls).
NEVER invoke subagents sequentially (defeats 60-min → 20-min optimization).

Each subagent runs in fresh context with focused research goal.
</subagent_requirement>

<timing_expectation>
3 parallel subagents × 20 min each = 20 min total wall time
NOT 60 min serial time
</timing_expectation>
</delegation_rule>

Parallel subagent investigation for novel/complex problems.

See `references/research-protocol.md#level-3-deep-research` for detailed process.

<success_criteria level="3">
- Comprehensive investigation from multiple angles
- Novel insights or authoritative precedent found
- HIGH confidence after synthesis
</success_criteria>

---

## Report Generation

Each level generates a structured report using templates in `assets/`:
- Level 1: `assets/level1-report-template.md`
- Level 2: `assets/level2-report-template.md`
- Level 3: `assets/level3-report-template.md`

Reports include: findings summary, confidence assessment, recommended solution, and source references.

---

<state_requirement name="checkpoint_protocol">
At the end of each level (when presenting findings), MUST:

1. Present decision menu (numbered list format, NOT AskUserQuestion tool)
2. WAIT for user response (NEVER auto-proceed)
3. Route based on selection:

<response_handler option="1" action="apply_solution">
<condition>User selects option 1 ("Apply solution")</condition>
<action>
1. Output confirmation: "User selected: Apply solution. Invoking plugin-improve skill..."
2. Use Skill tool to invoke plugin-improve
3. STOP execution (no further implementation)
</action>
</response_handler>

<response_handler option="2" action="review_findings">
<condition>User selects option 2 ("Review full findings")</condition>
<action>
1. Display complete research report
2. Re-present decision menu
3. WAIT for new selection
</action>
</response_handler>

<response_handler option="3" action="escalate">
<condition>User selects option 3 ("Escalate to next level")</condition>
<action>
1. Proceed to next level (Level 1 → Level 2 → Level 3)
2. If already at Level 3, inform user no further escalation available
3. Continue with next level's process
</action>
</response_handler>

<response_handler option="other" action="clarify">
<condition>User provides custom response</condition>
<action>
1. Ask for clarification
2. Re-present decision menu with context
3. WAIT for selection
</action>
</response_handler>

<enforcement>
NEVER auto-proceed to implementation.
NEVER skip decision menu.
ALWAYS wait for user response before continuing.
</enforcement>
</state_requirement>

## Decision Menus

After each level, present decision menu (user controls depth) per checkpoint_protocol above.

---

## Integration with troubleshooter

**troubleshooter Level 4:**

When troubleshooter agent exhausts Levels 0-3, it invokes deep-research:

```markdown
## troubleshooter.md - Level 4

If Levels 0-3 insufficient, escalate to deep-research skill:

"I need to investigate this more deeply. Invoking deep-research skill..."

[Invoke deep-research with problem context]

deep-research handles:

- Graduated research protocol (Levels 1-3)
- Parallel investigation (Level 3)
- Extended thinking budget
- Returns structured report with recommendations
```

**Integration flow:**

1. troubleshooter detects complex problem (Level 3 insufficient)
2. Invokes deep-research skill
3. deep-research starts at Level 1 (may escalate)
4. Returns structured report to troubleshooter
5. troubleshooter presents findings to user

---

## Integration with troubleshooting-docs

After successful application of Level 2-3 findings:

**Auto-suggest documentation:**

```
✓ Solution applied successfully

This was a complex problem (Level N research). Document for future reference?

1. Yes - Create troubleshooting doc (recommended)
2. No - Skip documentation
3. Other
```

If user chooses "Yes":

- Invoke troubleshooting-docs skill
- Pass research report + solution as context
- Creates dual-indexed documentation
- Future Level 1 searches will find it instantly

**The feedback loop:**

1. Level 3 research solves complex problem (45 min)
2. Document solution → troubleshooting/
3. Similar problem occurs → Level 1 finds it (5 min)
4. Knowledge compounds, research gets faster over time

---


## Error Handling

**Timeout (>60 min):**

```
⚠️ Research timeout (60 min limit)

Returning best findings based on investigation so far:
[Partial findings]

What's next?
1. Use current findings - Proceed with partial answer
2. Retry with extended timeout - Continue research (80 min)
3. Other
```

**No solution found:**

```
Research exhausted (Level 3 complete, no definitive solution)

Attempted:
✓ Local troubleshooting docs (0 matches)
✓ Context7 JUCE documentation (API exists but undocumented)
✓ JUCE forum + GitHub (no clear consensus)
✓ Parallel investigation (3 approaches, all have significant drawbacks)

Recommendations:
1. Post to JUCE forum with detailed investigation
2. Try experimental approach: [Suggestion]
3. Consider alternative feature: [Workaround]

I can help formulate forum post if needed.
```

**Subagent failure (Level 3):**

```
⚠️ Subagent [N] failed to complete research

Proceeding with findings from other subagents (N-1 completed)...
```

Continue with available findings, note partial investigation in report.

**WebSearch unavailable:**
Level 2 proceeds with Context7 only:

```
⚠️ Web search unavailable

Proceeding with Context7 JUCE documentation only.
Results may be incomplete for community knowledge.
```

---

---

## Performance Budgets

**Level 1:**

- Time: 5-10 min
- Extended thinking: No
- Success rate: 40% of problems (known solutions)

**Level 2:**

- Time: 15-30 min
- Extended thinking: No
- Success rate: 40% of problems (documented solutions)

**Level 3:**

- Time: 30-60 min
- Extended thinking: Yes (15k budget)
- Success rate: 20% of problems (novel/complex)

**Overall:**

- Average resolution time: 15 min (weighted by success rates)
- 80% of problems solved at Level 1-2 (fast)
- 20% require Level 3 (deep research justified)

---

## Example Scenarios

See `references/example-scenarios.md` for detailed walkthroughs of:
- Scenario 1: WebView freeze (solved at Level 1)
- Scenario 2: APVTS visibility (escalated to Level 2)
- Scenario 3: Novel DSP implementation (escalated to Level 3)
- Scenario 4: JUCE 8 migration (solved at Level 1 via Required Reading)
- Scenario 5: LookAndFeel propagation (answered at Level 1)

Each scenario shows escalation logic and time estimates.

---

## Future Enhancements

**Not in Phase 7 scope, but potential:**

- Learning from research patterns (which sources most useful)
- Caching Context7 queries (avoid repeated lookups)
- Confidence calibration (track prediction accuracy)
- Custom subagent types (specialized researchers)
- Integration with external knowledge bases (Stack Overflow, papers)
