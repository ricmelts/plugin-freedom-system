#!/bin/bash

HTML_FILE="v5-ui-test.html"
VALIDATION_PASSED=true
ISSUES=()

echo "Validating WebView constraints for v5-ui-test.html"

# Check 1: No viewport units
if grep -q "100vh\|100vw\|100dvh\|100svh" "$HTML_FILE"; then
    echo "❌ FAIL: Forbidden viewport units found"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: viewport units (100vh/100vw) detected")
else
    echo "✅ PASS: No viewport units detected"
fi

# Check 2: Required html/body height
if grep -q "html, body.*height: 100%" "$HTML_FILE"; then
    echo "✅ PASS: Required html/body height: 100% present"
else
    echo "❌ FAIL: Missing required html/body height: 100%"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing html/body height: 100%")
fi

# Check 3: Native feel CSS
if grep -q "user-select: none" "$HTML_FILE"; then
    echo "✅ PASS: user-select: none present"
else
    echo "❌ FAIL: Missing user-select: none"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing user-select: none")
fi

# Check 4: Context menu disabled
if grep -q 'contextmenu.*preventDefault' "$HTML_FILE"; then
    echo "✅ PASS: Context menu disabled"
else
    echo "❌ FAIL: Context menu not disabled"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: context menu not disabled")
fi

echo ""
if [ "$VALIDATION_PASSED" = true ]; then
    echo "✅ ALL WEBVIEW CONSTRAINTS VALIDATED"
    exit 0
else
    echo "❌ VALIDATION FAILED - ${#ISSUES[@]} issues"
    printf '  - %s\n' "${ISSUES[@]}"
    exit 1
fi
