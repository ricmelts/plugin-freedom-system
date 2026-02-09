# Stage 3 (GUI) Integration Checklist - v6

**Plugin:** RedShiftDistortion
**Mockup Version:** v6
**Generated:** 2026-02-08
**Window Size:** 950×700px (fixed, non-resizable)
**Total Parameters:** 13 (7 sliders, 4 toggles, 1 slider, 1 combo)

---

## Overview

This checklist guides integration of the finalized v6 UI mockup into RedShiftDistortion during Stage 3 (GUI). The v6 design features:

- Industrial metal aesthetic with diamond plate texture
- 168px center knob (doppler shift) + 6 surrounding 130px knobs
- Dual stereo VU meters (L + R) with solid red fill
- 4 bypass buttons with LED indicators
- 2 advanced settings (grain size slider + grain overlap dropdown)
- Red screw accents at 4 corners

---

## Phase 1: File Preparation

### 1.1 Create UI Directory Structure

```bash
cd plugins/RedShiftDistortion/Source

# Create UI directories
mkdir -p ui/public/js/juce
mkdir -p ui/public/assets
```

### 1.2 Copy UI Files

```bash
# Copy production HTML
cp .ideas/mockups/v6-ui.html Source/ui/public/index.html

# Copy JUCE WebView bridge (from working plugin or JUCE examples)
# These files are REQUIRED for WebView parameter binding
cp <source>/js/juce/index.js Source/ui/public/js/juce/
cp <source>/js/juce/check_native_interop.js Source/ui/public/js/juce/

# Copy visual assets (from mockups/assets/)
cp .ideas/mockups/assets/00fa754f6e134fb61f5e452b7ba8415e529dd82a.png Source/ui/public/assets/
cp .ideas/mockups/assets/4287f6bace032d3fb347fc04a01722012ff802a1.png Source/ui/public/assets/
cp .ideas/mockups/assets/f26475b6e81b4d3a1423f46d75b9ae5c2611416a.png Source/ui/public/assets/
```

**Verification:**
- [ ] `Source/ui/public/index.html` exists
- [ ] `Source/ui/public/js/juce/index.js` exists
- [ ] `Source/ui/public/js/juce/check_native_interop.js` exists
- [ ] All 3 PNG assets copied to `Source/ui/public/assets/`

---

## Phase 2: Update PluginEditor Files

### 2.1 Backup Current Files

```bash
cp Source/PluginEditor.h Source/PluginEditor.h.backup
cp Source/PluginEditor.cpp Source/PluginEditor.cpp.backup
```

### 2.2 Replace with v6 Templates

```bash
# Copy template files (ADAPT class name to match existing PluginEditor)
cp .ideas/mockups/v6-PluginEditor.h Source/PluginEditor.h
cp .ideas/mockups/v6-PluginEditor.cpp Source/PluginEditor.cpp
```

### 2.3 Verify Member Declaration Order

**CRITICAL:** Open `Source/PluginEditor.h` and verify member order:

```cpp
private:
    RedShiftDistortionAudioProcessor& audioProcessor;

    // 1️⃣ RELAYS FIRST (13 relays)
    std::unique_ptr<juce::WebSliderRelay> dopplerShiftRelay;
    std::unique_ptr<juce::WebSliderRelay> stereoWidthRelay;
    std::unique_ptr<juce::WebSliderRelay> feedbackRelay;
    std::unique_ptr<juce::WebSliderRelay> masterOutputRelay;
    std::unique_ptr<juce::WebSliderRelay> loCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> saturationRelay;
    std::unique_ptr<juce::WebSliderRelay> hiCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> grainSizeRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDopplerRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassSaturationRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassStereoWidthRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDelayRelay;
    std::unique_ptr<juce::WebComboBoxRelay> grainOverlapRelay;

    // 2️⃣ WEBVIEW SECOND
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3️⃣ ATTACHMENTS LAST (13 attachments)
    std::unique_ptr<juce::WebSliderParameterAttachment> dopplerShiftAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> stereoWidthAttachment;
    // ... (11 more attachments)
```

**Why this matters:** Members are destroyed in REVERSE order. Attachments call `evaluateJavascript()` during destruction, so they MUST be destroyed BEFORE webView. Wrong order causes release build crashes.

**Verification:**
- [ ] Relays declared before webView
- [ ] webView declared before attachments
- [ ] Relay count (13) matches attachment count (13)

### 2.4 Verify Initialization Order in .cpp

Open `Source/PluginEditor.cpp` constructor and verify initialization order matches declaration order:

```cpp
RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(...)
{
    // 1. Create relays FIRST
    dopplerShiftRelay = std::make_unique<juce::WebSliderRelay>("DOPPLER_SHIFT");
    // ... (12 more relays)

    // 2. Create WebView with relay options
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](auto& url) { return getResource(url); })
            .withOptionsFrom(*dopplerShiftRelay)
            // ... (12 more .withOptionsFrom() calls)
    );

    // 3. Create attachments LAST (JUCE 8 requires nullptr as 3rd parameter)
    dopplerShiftAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("DOPPLER_SHIFT"), *dopplerShiftRelay, nullptr);
    // ... (12 more attachments)
}
```

**Verification:**
- [ ] All 13 relays created before webView
- [ ] All 13 `.withOptionsFrom()` calls present
- [ ] All 13 attachments created after webView
- [ ] All attachments have `nullptr` as 3rd parameter (JUCE 8 requirement)

---

## Phase 3: Update CMakeLists.txt

### 3.1 Update juce_add_plugin() Call

Add `NEEDS_WEB_BROWSER TRUE` to existing `juce_add_plugin()` call:

```cmake
juce_add_plugin(RedShiftDistortion
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE RsDi
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "RedShiftDistortion"
    NEEDS_WEB_BROWSER TRUE  # ADD THIS LINE
)
```

**Why:** VST3 format requires explicit WebView flag. Without it, plugin builds successfully but won't appear in DAW VST3 scanner.

### 3.2 Append WebView Configuration

Append contents of `v6-CMakeLists.txt` to `CMakeLists.txt`:

```bash
cat .ideas/mockups/v6-CMakeLists.txt >> CMakeLists.txt
```

This adds:
- `juce_add_binary_data()` for UI resources
- `target_link_libraries()` for juce::juce_gui_extra
- `target_compile_definitions()` for JUCE_WEB_BROWSER=1

**Verification:**
- [ ] `NEEDS_WEB_BROWSER TRUE` in juce_add_plugin()
- [ ] `juce_add_binary_data()` includes all 6 files (1 HTML + 2 JS + 3 PNG)
- [ ] `juce::juce_gui_extra` linked
- [ ] `JUCE_WEB_BROWSER=1` defined

---

## Phase 4: Build and Test (Debug)

### 4.1 Clean Build

```bash
# Clean build directory
rm -rf build
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build plugin
cmake --build . --config Debug
```

**Verification:**
- [ ] Build completes without errors
- [ ] No warnings about missing headers
- [ ] BinaryData namespace includes all UI resources

### 4.2 Install to System

```bash
# Use build script (handles signing and cache clearing)
./scripts/build-and-install.sh RedShiftDistortion
```

**Verification:**
- [ ] VST3 installed to `~/Library/Audio/Plug-Ins/VST3/`
- [ ] AU installed to `~/Library/Audio/Plug-Ins/Components/`
- [ ] Code signature valid (check script output)

### 4.3 Test in Standalone

```bash
# Launch standalone app
open build/RedShiftDistortion_artefacts/Debug/Standalone/RedShiftDistortion.app
```

**Verification:**
- [ ] Standalone launches without crashes
- [ ] WebView displays UI (not blank screen)
- [ ] Right-click → Inspect works (developer tools)
- [ ] Console shows no JavaScript errors
- [ ] `window.__JUCE__` object exists in console

### 4.4 Test in DAW (Ableton/Logic)

**Verification:**
- [ ] Plugin appears in VST3 browser
- [ ] Plugin appears in AU browser
- [ ] Plugin loads without errors
- [ ] WebView displays UI correctly

---

## Phase 5: Test Parameter Binding

### 5.1 Test All 13 Parameters

| Parameter ID | Type | UI Control | Range | Default |
|--------------|------|------------|-------|---------|
| DOPPLER_SHIFT | Float | Center knob | -50 to +50% | 0% |
| STEREO_WIDTH | Float | Top-left knob | -100 to +100% | 0% |
| FEEDBACK | Float | Top-center knob | 0 to 95% | 0% |
| MASTER_OUTPUT | Float | Top-right knob | -60 to +12 dB | 0 dB |
| FILTER_BAND_LOW | Float | Bottom-left knob | 20 to 20000 Hz | 100 Hz |
| SATURATION | Float | Bottom-center knob | -12 to +24 dB | 0 dB |
| FILTER_BAND_HIGH | Float | Bottom-right knob | 20 to 20000 Hz | 8000 Hz |
| GRAIN_SIZE | Float | Bottom-right slider | 25 to 200 ms | 50 ms |
| BYPASS_DOPPLER | Bool | Button 1 | On/Off | Off |
| BYPASS_SATURATION | Bool | Button 2 | On/Off | Off |
| BYPASS_STEREO_WIDTH | Bool | Button 3 | On/Off | Off |
| BYPASS_DELAY | Bool | Button 4 | On/Off | Off |
| GRAIN_OVERLAP | Choice | Dropdown | 2x/4x | 4x |

**For each parameter:**
- [ ] UI control responds to mouse drag (knobs) or click (buttons/dropdown)
- [ ] UI updates when parameter changed via DAW automation
- [ ] Value persists after closing and reopening plugin
- [ ] Preset recall updates UI correctly

**CRITICAL TEST:** GRAIN_OVERLAP dropdown
- [ ] Uses `WebComboBoxRelay` (NOT WebSliderRelay)
- [ ] Uses `WebComboBoxParameterAttachment` (NOT WebSliderParameterAttachment)
- [ ] Displays "2x" and "4x" options (not 0 and 1)

### 5.2 Test Knob Interaction

**Center knob (doppler shift):**
- [ ] Rotates smoothly with mouse drag
- [ ] Neutral position (0%) = 12 o'clock (indicator points straight up)
- [ ] Range: -135° (min) to +135° (max)
- [ ] Red glow visible

**6 surrounding knobs:**
- [ ] Knob image rotates with value
- [ ] Text labels stay static (don't rotate)
- [ ] Neutral position at 12 o'clock for parameters with default=0

### 5.3 Test Bypass Buttons

**For each bypass button:**
- [ ] Click toggles active state
- [ ] Active state shows red gradient background
- [ ] LED indicator lights up when active
- [ ] Red glow visible when active

---

## Phase 6: Build and Test (Release)

### 6.1 Clean Release Build

```bash
# Clean build directory
rm -rf build
mkdir build
cd build

# Configure with CMake (Release)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build plugin
cmake --build . --config Release
```

**Verification:**
- [ ] Build completes without errors
- [ ] No warnings about member order

### 6.2 Install and Test

```bash
./scripts/build-and-install.sh RedShiftDistortion --config Release
```

**Verification:**
- [ ] Release build installs successfully
- [ ] Plugin loads in DAW without crashes
- [ ] **CRITICAL:** Reload plugin 10 times - no crashes (tests member order)
- [ ] All parameters still work correctly

---

## Phase 7: WebView-Specific Validation

### 7.1 CSS Validation

Open `Source/ui/public/index.html` and verify:

- [ ] NO viewport units (`100vh`, `100vw`, `100dvh`, `100svh`)
- [ ] `html, body { height: 100%; }` present
- [ ] `user-select: none` on body (native feel)
- [ ] Context menu disabled in JavaScript

### 7.2 Resource Provider Validation

Open `Source/PluginEditor.cpp` and verify `getResource()` method:

- [ ] Explicit URL mapping (no generic loops)
- [ ] Correct MIME types:
  - `index.html` → `"text/html"`
  - `index.js` → `"application/javascript"`
  - `check_native_interop.js` → `"application/javascript"`
  - PNG files → `"image/png"`
- [ ] All 6 resources mapped (1 HTML + 2 JS + 3 PNG)

### 7.3 Browser Developer Tools

Right-click plugin UI → Inspect → Console:

- [ ] No 404 errors for resources
- [ ] No JavaScript errors
- [ ] `window.__JUCE__` object exists
- [ ] `getSliderState()` function exists
- [ ] `getToggleState()` function exists
- [ ] `getComboBoxState()` function exists

---

## Phase 8: Visual Quality Checks

### 8.1 Mockup Comparison

Compare running plugin to `v6-ui-test.html` in browser:

- [ ] Window size matches (950×700px)
- [ ] Diamond plate texture displays correctly
- [ ] Red screws visible at 4 corners
- [ ] Title "RED SHIFT" with red glow
- [ ] Subtitle "digital delay" with red glow
- [ ] Center knob with red glow (168px)
- [ ] 6 surrounding knobs (130px) with knob image
- [ ] 4 bypass buttons with LED indicators
- [ ] Grain size slider (90×23px)
- [ ] Grain overlap dropdown (90×26px)
- [ ] Dual VU meters (L + R, 40px each, solid red)

### 8.2 Font Loading

- [ ] Title uses Jacquard 12 font (not fallback sans-serif)
- [ ] Labels use Handjet font (not fallback sans-serif)
- [ ] Google Fonts CDN loads successfully

### 8.3 Visual Effects

- [ ] Red glow on title/subtitle/labels
- [ ] Center knob red glow animates with value
- [ ] Bypass button LED glow when active
- [ ] VU meter fills animate smoothly

---

## Phase 9: Performance Validation

### 9.1 CPU Usage

**In DAW:**
- [ ] CPU usage <5% with no audio playing
- [ ] CPU usage <10% with audio processing
- [ ] No spikes when adjusting knobs

### 9.2 UI Responsiveness

- [ ] Knobs respond immediately to drag (no lag)
- [ ] Parameter updates from automation smooth (no jitter)
- [ ] VU meters update at ~60fps

---

## Phase 10: Final Verification

### 10.1 Parameter Count Verification

**Total:** 13 parameters
- **7 sliders:** doppler_shift, stereo_width, feedback, master_output, lo_cut_filter, saturation, hi_cut_filter
- **4 toggles:** bypass_doppler, bypass_saturation, bypass_filters (maps to BYPASS_STEREO_WIDTH), bypass_feedback (maps to BYPASS_DELAY)
- **1 slider:** grain_size
- **1 combo:** grain_overlap

**CRITICAL:** UI parameter IDs must match APVTS parameter IDs:
- UI `lo_cut_filter` → APVTS `FILTER_BAND_LOW`
- UI `hi_cut_filter` → APVTS `FILTER_BAND_HIGH`
- UI `bypass_filters` → APVTS `BYPASS_STEREO_WIDTH`
- UI `bypass_feedback` → APVTS `BYPASS_DELAY`

### 10.2 Member Count Verification

**In PluginEditor.h:**
- [ ] 13 relay declarations (8 WebSliderRelay + 4 WebToggleButtonRelay + 1 WebComboBoxRelay)
- [ ] 1 webView declaration
- [ ] 13 attachment declarations (8 WebSliderParameterAttachment + 4 WebToggleButtonParameterAttachment + 1 WebComboBoxParameterAttachment)

**In PluginEditor.cpp:**
- [ ] 13 relay creations in constructor
- [ ] 13 `.withOptionsFrom()` calls for webView
- [ ] 13 attachment creations in constructor

---

## Completion Criteria

Integration is COMPLETE when:

- [x] All 13 parameters bind correctly (UI ↔ APVTS)
- [x] Release build stable (no crashes on plugin reload)
- [x] Visual design matches v6 mockup
- [x] WebView best practices followed (no viewport units, correct member order, explicit resource mapping)
- [x] No console errors in developer tools
- [x] Plugin works in VST3, AU, and Standalone formats

---

## Troubleshooting

### Issue: Knobs frozen (don't respond to drag)

**Symptoms:**
- UI displays correctly
- Knobs visible but don't rotate when dragged
- Console error: "Failed to get slider state for [paramId]"

**Fix:**
1. Verify `type="module"` on script tags in index.html
2. Verify `import { getSliderState }` in JavaScript
3. Verify parameter IDs match APVTS exactly (case-sensitive)

### Issue: VST3 doesn't appear in DAW

**Symptoms:**
- Build succeeds
- AU works fine
- VST3 missing from plugin list

**Fix:**
1. Add `NEEDS_WEB_BROWSER TRUE` to juce_add_plugin()
2. Rebuild and reinstall
3. Clear DAW cache and restart

### Issue: Release build crashes on reload

**Symptoms:**
- Debug build works perfectly
- Release build crashes when closing plugin
- DAW freezes

**Fix:**
1. Verify member order in PluginEditor.h: relays → webView → attachments
2. Verify initialization order matches declaration order
3. Rebuild release

### Issue: Images don't load (404 errors)

**Symptoms:**
- UI displays but images missing
- Console shows 404 for PNG files
- Developer tools Network tab shows failed requests

**Fix:**
1. Verify all PNG files in `Source/ui/public/assets/`
2. Verify `juce_add_binary_data()` includes all PNG files
3. Verify `getResource()` maps all PNG URLs with correct MIME type
4. Rebuild (BinaryData regenerated)

---

## Success Metrics

**GUI Stage 3 succeeds when:**

- Plugin loads in all 3 formats (VST3, AU, Standalone)
- All 13 parameters work bidirectionally (UI ↔ DAW)
- Visual design matches v6 mockup exactly
- No crashes in debug OR release builds
- Performance <10% CPU usage
- Passes all 10 phases of this checklist

**Ready for Stage 4 (Testing) when all checkboxes marked.**
