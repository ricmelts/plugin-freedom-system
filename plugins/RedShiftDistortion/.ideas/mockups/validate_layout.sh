#!/bin/bash

VALIDATION_PASSED=true
ERRORS=()
WARNINGS=()

WINDOW_WIDTH=900
WINDOW_HEIGHT=650

echo "Validating layout: ${WINDOW_WIDTH}×${WINDOW_HEIGHT}"

# Define controls with positions and sizes
declare -A controls=(
  ["stereo_width"]="80,150,150,150"
  ["feedback"]="280,150,150,150"
  ["doppler_shift"]="335,80,230,230"
  ["filter_band_low"]="50,390,130,130"
  ["filter_band_high"]="210,390,130,130"
  ["saturation"]="370,390,130,130"
  ["master_output"]="530,390,130,130"
  ["bypass_stereo_width"]="100,565,80,80"
  ["bypass_delay"]="220,565,80,80"
  ["bypass_doppler"]="340,565,80,80"
  ["bypass_saturation"]="460,565,80,80"
  ["grain_size"]="715,470,150,30"
  ["grain_overlap"]="715,530,150,35"
)

# Validation 1: Bounds containment
echo ""
echo "=== Bounds Validation ==="
for ctrl_id in "${!controls[@]}"; do
  IFS=',' read -r x y w h <<< "${controls[$ctrl_id]}"
  
  right_edge=$((x + w))
  bottom_edge=$((y + h))
  
  if [ $right_edge -gt $WINDOW_WIDTH ]; then
    echo "❌ FAIL: $ctrl_id overflows right edge ($right_edge > $WINDOW_WIDTH)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: $ctrl_id overflows right edge by $((right_edge - WINDOW_WIDTH))px")
  fi
  
  if [ $bottom_edge -gt $WINDOW_HEIGHT ]; then
    echo "❌ FAIL: $ctrl_id overflows bottom edge ($bottom_edge > $WINDOW_HEIGHT)"
    VALIDATION_PASSED=false
    ERRORS+=("Bounds violation: $ctrl_id overflows bottom edge by $((bottom_edge - WINDOW_HEIGHT))px")
  fi
  
  if [ $x -lt 0 ]; then
    ERRORS+=("Bounds violation: $ctrl_id extends past left edge (x=$x)")
    VALIDATION_PASSED=false
  fi
  
  if [ $y -lt 0 ]; then
    ERRORS+=("Bounds violation: $ctrl_id extends past top edge (y=$y)")
    VALIDATION_PASSED=false
  fi
done

# Validation 2: Edge padding (15px minimum)
echo ""
echo "=== Edge Padding Validation ==="
MIN_EDGE_PADDING=15

for ctrl_id in "${!controls[@]}"; do
  IFS=',' read -r x y w h <<< "${controls[$ctrl_id]}"
  
  right_edge=$((x + w))
  bottom_edge=$((y + h))
  right_padding=$((WINDOW_WIDTH - right_edge))
  bottom_padding=$((WINDOW_HEIGHT - bottom_edge))
  
  if [ $x -lt $MIN_EDGE_PADDING ]; then
    echo "❌ FAIL: $ctrl_id too close to left edge (${x}px, need ${MIN_EDGE_PADDING}px minimum)"
    VALIDATION_PASSED=false
    ERRORS+=("Edge padding violation: $ctrl_id only ${x}px from left edge")
  fi
  
  if [ $y -lt $MIN_EDGE_PADDING ]; then
    echo "❌ FAIL: $ctrl_id too close to top edge (${y}px, need ${MIN_EDGE_PADDING}px minimum)"
    VALIDATION_PASSED=false
    ERRORS+=("Edge padding violation: $ctrl_id only ${y}px from top edge")
  fi
  
  if [ $right_padding -lt $MIN_EDGE_PADDING ]; then
    echo "❌ FAIL: $ctrl_id too close to right edge (${right_padding}px padding, need ${MIN_EDGE_PADDING}px minimum)"
    VALIDATION_PASSED=false
    ERRORS+=("Edge padding violation: $ctrl_id only ${right_padding}px from right edge")
  fi
  
  if [ $bottom_padding -lt $MIN_EDGE_PADDING ]; then
    echo "❌ FAIL: $ctrl_id too close to bottom edge (${bottom_padding}px padding, need ${MIN_EDGE_PADDING}px minimum)"
    VALIDATION_PASSED=false
    ERRORS+=("Edge padding violation: $ctrl_id only ${bottom_padding}px from bottom edge")
  fi
done

# Validation 3: Minimum sizes
echo ""
echo "=== Minimum Size Validation ==="

for ctrl_id in "${!controls[@]}"; do
  IFS=',' read -r x y w h <<< "${controls[$ctrl_id]}"
  
  # Knobs should be at least 40px diameter
  if [[ "$ctrl_id" =~ (stereo_width|feedback|doppler_shift|filter_band_low|filter_band_high|saturation|master_output) ]]; then
    if [ $w -lt 40 ] || [ $h -lt 40 ]; then
      echo "❌ FAIL: $ctrl_id too small (knobs need min 40px diameter)"
      VALIDATION_PASSED=false
      ERRORS+=("Size violation: $ctrl_id too small (${w}×${h}, need min 40px diameter)")
    fi
  fi
  
  # Toggles should be at least 20px
  if [[ "$ctrl_id" =~ bypass ]]; then
    if [ $w -lt 20 ] || [ $h -lt 20 ]; then
      echo "❌ FAIL: $ctrl_id too small (toggles need min 20px)"
      VALIDATION_PASSED=false
      ERRORS+=("Size violation: $ctrl_id too small (${w}×${h}, need min 20px)")
    fi
  fi
  
  # Sliders (grain_size) should be at least 100px long
  if [[ "$ctrl_id" == "grain_size" ]]; then
    if [ $w -lt 100 ]; then
      echo "❌ FAIL: $ctrl_id too short (sliders need min 100px length)"
      VALIDATION_PASSED=false
      ERRORS+=("Size violation: $ctrl_id too short (${w}px, need min 100px)")
    fi
  fi
done

echo ""
if [ "$VALIDATION_PASSED" = true ]; then
  echo "✅ Layout validation passed"
  exit 0
else
  echo "❌ Layout validation failed - ${#ERRORS[@]} errors found"
  printf '%s\n' "${ERRORS[@]}"
  exit 1
fi
