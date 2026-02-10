#!/bin/bash

WINDOW_WIDTH=950
WINDOW_HEIGHT=700
VALIDATION_PASSED=true
ERRORS=()
WARNINGS=()

echo "Validating v5 layout: ${WINDOW_WIDTH}×${WINDOW_HEIGHT}px"

# Check bounds for central doppler knob (168px diameter)
DOPPLER_X=391
DOPPLER_Y=266
DOPPLER_SIZE=168

RIGHT_EDGE=$((DOPPLER_X + DOPPLER_SIZE))
BOTTOM_EDGE=$((DOPPLER_Y + DOPPLER_SIZE))

if [ $RIGHT_EDGE -gt $WINDOW_WIDTH ]; then
    echo "❌ FAIL: doppler_shift overflows right edge ($RIGHT_EDGE > $WINDOW_WIDTH)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: doppler_shift overflows right edge by $((RIGHT_EDGE - WINDOW_WIDTH))px")
fi

if [ $BOTTOM_EDGE -gt $WINDOW_HEIGHT ]; then
    echo "❌ FAIL: doppler_shift overflows bottom edge ($BOTTOM_EDGE > $WINDOW_HEIGHT)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: doppler_shift overflows bottom edge by $((BOTTOM_EDGE - WINDOW_HEIGHT))px")
fi

# Check VU meter position (870, 175, 50×350)
VU_X=870
VU_Y=175
VU_W=50
VU_H=350

VU_RIGHT=$((VU_X + VU_W))
VU_BOTTOM=$((VU_Y + VU_H))

if [ $VU_RIGHT -gt $WINDOW_WIDTH ]; then
    echo "❌ FAIL: VU meter overflows right edge ($VU_RIGHT > $WINDOW_WIDTH)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: VU meter overflows right edge by $((VU_RIGHT - WINDOW_WIDTH))px")
else
    echo "✅ PASS: VU meter right edge at ${VU_RIGHT}px (within ${WINDOW_WIDTH}px)"
fi

if [ $VU_BOTTOM -gt $WINDOW_HEIGHT ]; then
    echo "❌ FAIL: VU meter overflows bottom edge ($VU_BOTTOM > $WINDOW_HEIGHT)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: VU meter overflows bottom edge by $((VU_BOTTOM - WINDOW_HEIGHT))px")
else
    echo "✅ PASS: VU meter bottom edge at ${VU_BOTTOM}px (within ${WINDOW_HEIGHT}px)"
fi

# Check edge padding for VU meter
RIGHT_PADDING=$((WINDOW_WIDTH - VU_RIGHT))
if [ $RIGHT_PADDING -lt 15 ]; then
    echo "⚠️  WARNING: VU meter only ${RIGHT_PADDING}px from right edge (recommend 15px minimum)"
    WARNINGS+=("Edge padding: VU meter only ${RIGHT_PADDING}px from right edge")
else
    echo "✅ PASS: VU meter has ${RIGHT_PADDING}px padding from right edge"
fi

# Check advanced settings (bottom-right)
GRAIN_SIZE_X=815
GRAIN_SIZE_Y=630
GRAIN_SIZE_W=90
GRAIN_SIZE_H=23

GRAIN_OVERLAP_X=815
GRAIN_OVERLAP_Y=665
GRAIN_OVERLAP_W=90
GRAIN_OVERLAP_H=26

GRAIN_SIZE_RIGHT=$((GRAIN_SIZE_X + GRAIN_SIZE_W))
GRAIN_OVERLAP_RIGHT=$((GRAIN_OVERLAP_X + GRAIN_OVERLAP_W))
GRAIN_OVERLAP_BOTTOM=$((GRAIN_OVERLAP_Y + GRAIN_OVERLAP_H))

if [ $GRAIN_SIZE_RIGHT -gt $WINDOW_WIDTH ]; then
    echo "❌ FAIL: grain_size overflows right edge"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: grain_size overflows right edge")
fi

if [ $GRAIN_OVERLAP_BOTTOM -gt $WINDOW_HEIGHT ]; then
    echo "❌ FAIL: grain_overlap overflows bottom edge"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: grain_overlap overflows bottom edge")
fi

echo ""
if [ "$VALIDATION_PASSED" = true ]; then
    echo "✅ VALIDATION PASSED"
    if [ ${#WARNINGS[@]} -gt 0 ]; then
        echo ""
        echo "⚠️  ${#WARNINGS[@]} warning(s):"
        printf '  - %s\n' "${WARNINGS[@]}"
    fi
    exit 0
else
    echo "❌ VALIDATION FAILED - ${#ERRORS[@]} errors"
    printf '  - %s\n' "${ERRORS[@]}"
    exit 1
fi
