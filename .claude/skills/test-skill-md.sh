#!/bin/bash

# test-skill-md.sh - Validates SKILL.md files for structural correctness
# Tests YAML frontmatter, required sections, cross-references, and best practices

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
WARNINGS=0

# Test results storage
declare -a FAILED_TEST_MESSAGES
declare -a WARNING_MESSAGES

# Print functions
print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_test() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗${NC} $2"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_TEST_MESSAGES+=("$2")
    fi
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
    WARNINGS=$((WARNINGS + 1))
    WARNING_MESSAGES+=("$1")
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

# Usage
usage() {
    cat << EOF
Usage: $0 [SKILL_MD_FILE] [OPTIONS]

Validates SKILL.md files for structural correctness and best practices.

Arguments:
    SKILL_MD_FILE    Path to SKILL.md file to validate
                     If not provided, validates all SKILL.md files in .claude/skills/

Options:
    --all            Validate all SKILL.md files (default if no file specified)
    --strict         Treat warnings as errors
    --quiet          Only show summary
    --help           Show this help message

Examples:
    $0 .claude/skills/system-setup/SKILL.md
    $0 --all
    $0 .claude/skills/plugin-ideation/SKILL.md --strict

EOF
    exit 0
}

# Parse arguments
SKILL_FILE=""
VALIDATE_ALL=false
STRICT_MODE=false
QUIET_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --all)
            VALIDATE_ALL=true
            shift
            ;;
        --strict)
            STRICT_MODE=true
            shift
            ;;
        --quiet)
            QUIET_MODE=true
            shift
            ;;
        --help|-h)
            usage
            ;;
        *)
            if [[ -f "$1" ]]; then
                SKILL_FILE="$1"
            else
                echo -e "${RED}Error: File not found: $1${NC}"
                exit 1
            fi
            shift
            ;;
    esac
done

# If no file specified, default to --all
if [[ -z "$SKILL_FILE" && "$VALIDATE_ALL" == "false" ]]; then
    VALIDATE_ALL=true
fi

# Validation functions
validate_yaml_frontmatter() {
    local file=$1
    local skill_dir=$(dirname "$file")

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Validating YAML frontmatter..."
    fi

    # Check if frontmatter exists
    if ! grep -q "^---$" "$file"; then
        print_test 1 "YAML frontmatter exists"
        return 1
    fi
    print_test 0 "YAML frontmatter exists"

    # Extract frontmatter (between first two ---)
    local frontmatter=$(sed -n '/^---$/,/^---$/p' "$file" | sed '1d;$d')

    # Check required fields
    local required_fields=("name" "description" "allowed-tools" "preconditions")
    for field in "${required_fields[@]}"; do
        if echo "$frontmatter" | grep -q "^${field}:"; then
            print_test 0 "Required field: $field"
        else
            print_test 1 "Required field: $field"
        fi
    done

    # Validate name matches directory
    local skill_name=$(echo "$frontmatter" | grep "^name:" | sed 's/name: *//')
    local dir_name=$(basename "$skill_dir")

    if [[ "$skill_name" == "$dir_name" ]]; then
        print_test 0 "Skill name matches directory name"
    else
        print_test 1 "Skill name '$skill_name' does not match directory '$dir_name'"
    fi

    # Check if allowed-tools is a list
    if echo "$frontmatter" | grep -A 5 "^allowed-tools:" | grep -q "^  -"; then
        print_test 0 "allowed-tools is a proper YAML list"
    else
        print_warning "allowed-tools should be a YAML list with '  -' items"
    fi

    # Validate common allowed tools
    local valid_tools=("Read" "Write" "Edit" "Bash" "Grep" "Glob" "Task" "SlashCommand" "WebSearch" "WebFetch")
    local tools_section=$(echo "$frontmatter" | sed -n '/^allowed-tools:/,/^[a-z-]*:/p' | grep "^  -")

    while IFS= read -r tool_line; do
        local tool=$(echo "$tool_line" | sed 's/^  - //' | sed 's/ #.*//' | xargs)
        if [[ -n "$tool" ]]; then
            if [[ " ${valid_tools[@]} " =~ " ${tool} " ]]; then
                : # Valid tool, no output in quiet mode
            else
                print_warning "Unknown tool in allowed-tools: $tool"
            fi
        fi
    done <<< "$tools_section"
}

validate_required_sections() {
    local file=$1

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Validating required sections..."
    fi

    # Check for Purpose section
    if grep -q "^\*\*Purpose:\*\*" "$file"; then
        print_test 0 "Purpose section exists"
    else
        print_test 1 "Purpose section (**Purpose:**) is missing"
    fi

    # Check for h1 heading with skill name
    if grep -q "^# .* Skill$" "$file"; then
        print_test 0 "Main heading (# ... Skill) exists"
    else
        print_test 1 "Main heading should be '# <name> Skill'"
    fi

    # Common recommended sections
    local recommended_sections=("Overview" "Integration Points")
    for section in "${recommended_sections[@]}"; do
        if grep -q "^## $section$" "$file"; then
            : # Found, no output in quiet mode
        else
            print_warning "Recommended section missing: ## $section"
        fi
    done
}

validate_cross_references() {
    local file=$1
    local skill_dir=$(dirname "$file")

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Validating cross-references..."
    fi

    # Extract all markdown links to local files: [text](path.md)
    local refs=$(grep -o '\[.*\]([^)]*\.md)' "$file" | sed 's/.*(\([^)]*\))/\1/')

    local ref_count=0
    local missing_refs=0

    while IFS= read -r ref; do
        if [[ -n "$ref" && ! "$ref" =~ ^http ]]; then
            ref_count=$((ref_count + 1))
            # Resolve relative paths
            local ref_path
            if [[ "$ref" == /* ]]; then
                ref_path="$ref"
            else
                ref_path="$skill_dir/$ref"
            fi

            if [[ -f "$ref_path" ]]; then
                : # Reference exists, no output in quiet mode
            else
                print_warning "Referenced file does not exist: $ref (resolved: $ref_path)"
                missing_refs=$((missing_refs + 1))
            fi
        fi
    done <<< "$refs"

    if [[ $ref_count -gt 0 ]]; then
        if [[ $missing_refs -eq 0 ]]; then
            print_test 0 "All $ref_count cross-references exist"
        else
            print_test 1 "$missing_refs of $ref_count cross-references are missing"
        fi
    else
        print_warning "No cross-references found (consider adding references/documentation)"
    fi
}

validate_critical_sequences() {
    local file=$1

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Validating critical sequences..."
    fi

    # Check for critical sequence tags
    if grep -q "<critical_sequence" "$file"; then
        local seq_count=$(grep -c "<critical_sequence" "$file")
        print_test 0 "Critical sequences defined ($seq_count found)"

        # Validate closing tags
        local close_count=$(grep -c "</critical_sequence>" "$file")
        if [[ $seq_count -eq $close_count ]]; then
            print_test 0 "All critical sequences properly closed"
        else
            print_test 1 "Critical sequence tag mismatch (open: $seq_count, close: $close_count)"
        fi
    else
        # Not required, just informational
        : # No critical sequences
    fi
}

validate_state_management() {
    local file=$1

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Validating state management documentation..."
    fi

    # Check for state management section
    if grep -q "<state_management>" "$file" || grep -q "## State Management" "$file"; then
        print_test 0 "State management documented"
    else
        print_warning "No state management section found (add if skill modifies files)"
    fi
}

validate_best_practices() {
    local file=$1

    if [[ "$QUIET_MODE" == "false" ]]; then
        print_info "Checking best practices..."
    fi

    # Check for Integration Points
    if grep -q "## Integration Points" "$file"; then
        print_test 0 "Integration Points section exists"
    else
        print_warning "Consider adding '## Integration Points' section"
    fi

    # Check for delegation rules if Task tool is allowed
    if grep -q "allowed-tools:" "$file" && grep -A 20 "allowed-tools:" "$file" | grep -q "Task"; then
        if grep -q "delegation" "$file" || grep -q "<delegation_rules>" "$file"; then
            print_test 0 "Delegation rules documented (Task tool is allowed)"
        else
            print_warning "Task tool is allowed but no delegation rules found"
        fi
    fi

    # Check for proper mode/state variable documentation
    if grep -q "MODE" "$file" || grep -q "TEST_MODE" "$file"; then
        if grep -q "## State Management" "$file" || grep -q "persist" "$file"; then
            : # Variables are documented
        else
            print_warning "State variables found but persistence not clearly documented"
        fi
    fi
}

# Main validation function
validate_skill_file() {
    local file=$1
    local skill_name=$(basename $(dirname "$file"))

    print_header "Validating: $skill_name (${file})"

    # Reset counters for this file
    TOTAL_TESTS=0
    PASSED_TESTS=0
    FAILED_TESTS=0
    WARNINGS=0
    FAILED_TEST_MESSAGES=()
    WARNING_MESSAGES=()

    # Run validations
    validate_yaml_frontmatter "$file"
    validate_required_sections "$file"
    validate_cross_references "$file"
    validate_critical_sequences "$file"
    validate_state_management "$file"
    validate_best_practices "$file"

    # Print summary for this file
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "Summary for ${BLUE}$skill_name${NC}:"
    echo -e "  ${GREEN}Passed:${NC} $PASSED_TESTS/$TOTAL_TESTS tests"

    if [[ $FAILED_TESTS -gt 0 ]]; then
        echo -e "  ${RED}Failed:${NC} $FAILED_TESTS/$TOTAL_TESTS tests"
        for msg in "${FAILED_TEST_MESSAGES[@]}"; do
            echo -e "    ${RED}✗${NC} $msg"
        done
    fi

    if [[ $WARNINGS -gt 0 ]]; then
        echo -e "  ${YELLOW}Warnings:${NC} $WARNINGS"
        if [[ "$QUIET_MODE" == "false" ]]; then
            for msg in "${WARNING_MESSAGES[@]}"; do
                echo -e "    ${YELLOW}⚠${NC} $msg"
            done
        fi
    fi

    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""

    # Return status
    if [[ $FAILED_TESTS -gt 0 ]]; then
        return 1
    fi

    if [[ "$STRICT_MODE" == "true" && $WARNINGS -gt 0 ]]; then
        echo -e "${YELLOW}Strict mode: Treating warnings as errors${NC}"
        return 1
    fi

    return 0
}

# Main execution
main() {
    local exit_code=0

    if [[ "$VALIDATE_ALL" == "true" ]]; then
        # Find all SKILL.md files
        local skill_files=()
        while IFS= read -r file; do
            skill_files+=("$file")
        done < <(find .claude/skills -name "SKILL.md" -type f | sort)

        if [[ ${#skill_files[@]} -eq 0 ]]; then
            echo -e "${RED}No SKILL.md files found in .claude/skills/${NC}"
            exit 1
        fi

        print_header "Validating ${#skill_files[@]} SKILL.md files"
        echo ""

        local total_files=${#skill_files[@]}
        local passed_files=0
        local failed_files=0

        for file in "${skill_files[@]}"; do
            if validate_skill_file "$file"; then
                passed_files=$((passed_files + 1))
            else
                failed_files=$((failed_files + 1))
                exit_code=1
            fi
        done

        # Overall summary
        print_header "Overall Summary"
        echo -e "Total files validated: $total_files"
        echo -e "${GREEN}Passed: $passed_files${NC}"
        if [[ $failed_files -gt 0 ]]; then
            echo -e "${RED}Failed: $failed_files${NC}"
        fi
        echo ""

    else
        # Validate single file
        if [[ ! -f "$SKILL_FILE" ]]; then
            echo -e "${RED}Error: File not found: $SKILL_FILE${NC}"
            exit 1
        fi

        if ! validate_skill_file "$SKILL_FILE"; then
            exit_code=1
        fi
    fi

    exit $exit_code
}

# Run main
main
