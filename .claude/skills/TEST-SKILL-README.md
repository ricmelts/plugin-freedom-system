# SKILL.md Test Suite

Comprehensive validation tool for SKILL.md files in the Plugin Freedom System.

## Overview

`test-skill-md.sh` validates the structure, content, and best practices of SKILL.md files to ensure consistency and correctness across all skills.

## Usage

### Test a Single File

```bash
.claude/skills/test-skill-md.sh .claude/skills/system-setup/SKILL.md
```

### Test All SKILL.md Files

```bash
.claude/skills/test-skill-md.sh --all
```

### Options

- `--all` - Validate all SKILL.md files in `.claude/skills/`
- `--strict` - Treat warnings as errors (non-zero exit code)
- `--quiet` - Only show summary, suppress detailed output
- `--help` - Show help message

### Examples

```bash
# Test single file
./test-skill-md.sh .claude/skills/plugin-ideation/SKILL.md

# Test all files
./test-skill-md.sh --all

# Test all files in strict mode (CI/CD)
./test-skill-md.sh --all --strict

# Quiet mode for quick check
./test-skill-md.sh --all --quiet
```

## Validation Checks

### 1. YAML Frontmatter Validation

- **Frontmatter exists** - File starts with `---` delimiter
- **Required fields present:**
  - `name` - Skill identifier (should match directory name)
  - `description` - Brief description of skill purpose
  - `allowed-tools` - List of Claude Code tools the skill can use
  - `preconditions` - Prerequisites before skill can run
- **Name consistency** - `name` field matches directory name
- **Proper YAML formatting** - `allowed-tools` is a properly formatted list

### 2. Required Sections

- **Purpose section** - `**Purpose:**` statement exists
- **Main heading** - Proper `# <name> Skill` heading format
- **Recommended sections:**
  - `## Overview` - High-level skill description
  - `## Integration Points` - How skill connects to system

### 3. Cross-Reference Validation

- **Local file references** - All `[text](path.md)` links point to existing files
- **Relative path resolution** - References resolved correctly from skill directory
- **Reference count** - Reports total references found and validated

### 4. Critical Sequence Validation

- **Tag matching** - `<critical_sequence>` tags properly opened and closed
- **Count verification** - Equal number of opening/closing tags

### 5. State Management Documentation

- **State management section** - Presence of `<state_management>` or `## State Management`
- **Important for skills that:**
  - Modify PLUGINS.md
  - Create/update `.continue-here.md`
  - Create plugin directories
  - Update configuration files

### 6. Best Practices

- **Integration Points** - Documented entry points, file reads/writes, and skill invocations
- **Delegation rules** - If `Task` tool is allowed, delegation rules should be documented
- **Variable persistence** - State variables (MODE, TEST_MODE) should document scope and persistence

## Exit Codes

- `0` - All tests passed (warnings ignored unless `--strict`)
- `1` - One or more tests failed (or warnings in `--strict` mode)

## Common Issues Found

### Critical Errors (Test Failures)

1. **Missing YAML frontmatter** - No `---` delimiters at start
2. **Missing required fields** - `allowed-tools` or `preconditions` not defined
3. **Name mismatch** - Skill name doesn't match directory name
4. **Missing Purpose** - No `**Purpose:**` statement
5. **Wrong heading format** - Not using `# <name> Skill` format
6. **Broken references** - Links to non-existent files
7. **Tag mismatch** - Unclosed `<critical_sequence>` tags

### Warnings (Non-blocking)

1. **Unknown tools** - Tools like `AskUserQuestion` or `Skill` not in standard list
2. **Missing Overview** - No `## Overview` section
3. **Missing Integration Points** - No integration documentation
4. **No state management** - No state management section (may be OK for stateless skills)
5. **No cross-references** - No references to documentation or other files
6. **Missing delegation rules** - Task tool allowed but no delegation guidance

## Recent Test Results

Last run found:
- **Passed**: 9/16 files (100% of tests)
- **Failed**: 7/16 files (missing required fields, wrong formats)
- **Common issues**:
  - `build-automation`: Missing frontmatter fields
  - `workflow-reconciliation`: Missing frontmatter fields
  - `ui-mockup`: Missing Purpose and heading
  - `system-setup`: Unclosed critical_sequence tag

## Integration with CI/CD

Add to your CI pipeline:

```bash
#!/bin/bash
# Run SKILL.md validation in strict mode
.claude/skills/test-skill-md.sh --all --strict

if [ $? -ne 0 ]; then
  echo "SKILL.md validation failed. Please fix errors before merging."
  exit 1
fi
```

## Extending the Tests

To add new validation rules, edit `test-skill-md.sh` and add functions:

```bash
validate_my_new_check() {
    local file=$1

    # Your validation logic here
    if [[ condition ]]; then
        print_test 0 "My check passed"
    else
        print_test 1 "My check failed"
    fi
}

# Add to main validation function
validate_skill_file() {
    # ... existing checks ...
    validate_my_new_check "$file"
}
```

## Related Files

- `test-skill-md.sh` - Main test script
- `.claude/skills/*/SKILL.md` - Skill definition files
- `.claude/skills/system-setup/test-setup-skill.sh` - Example skill-specific test

## Maintenance

Run tests regularly:
- Before committing changes to SKILL.md files
- After creating new skills
- In CI/CD pipeline
- When updating skill structure standards

## Support

For issues or questions about the test suite, check:
- Test output for specific error messages
- This README for validation rules
- Individual SKILL.md files for examples of proper format
