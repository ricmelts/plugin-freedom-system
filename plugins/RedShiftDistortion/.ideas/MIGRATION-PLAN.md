# Tape Delay Migration Plan

**Date:** 2026-02-13
**Migration Type:** Granular Doppler Delay → One-Sided Tape Delay
**Backup Branch:** `backup/pre-tape-delay-migration-20260213-010141`

## Architecture Change Summary

### OLD Architecture (Granular Doppler)
- Granular synthesis pitch shifting (cumulative doppler effect)
- Stereo width modulation (differential L/R delay timing)
- Complex grain scheduling (4 grains × 2 channels)
- Both channels processed identically

### NEW Architecture (One-Sided Tape Delay)
- Left channel: DRY passthrough (no processing)
- Right channel: WET processing (delay + saturation + filters + feedback)
- Optional ping-pong mode (feedback bounces between L/R)
- Simpler, classic tape delay design

---

## Parameter Migration Map

### Parameters REMOVED (4 total)
| Parameter ID | Old Range | Reason |
|--------------|-----------|--------|
| `stereoWidth` | -100% to +100% | Replaced by one-sided architecture |
| `dopplerShift` | -50% to +50% | Replaced by `delayTime` |
| `grainSize` | 25-200ms | Granular processing removed |
| `grainOverlap` | 2x / 4x | Granular processing removed |

### Parameters ADDED (2 total)
| Parameter ID | Range | Default | Purpose |
|--------------|-------|---------|---------|
| `delayTime` | 10-2000ms (log) | 260ms | Tape delay time (user-controllable) |
| `pingPongAmount` | 0-100% | 0% | Ping-pong feedback amount |

### Parameters MODIFIED (1 total)
| Old ID | New ID | Change |
|--------|--------|--------|
| `bypassStereoWidth` | `bypassPingPong` | Repurposed for ping-pong bypass |

### Parameters KEPT UNCHANGED (8 total)
- `feedback` (0-95%)
- `filterBandLow` (20-20000 Hz)
- `filterBandHigh` (20-20000 Hz)
- `saturation` (-12 to +24 dB)
- `masterOutput` (-60 to +12 dB)
- `bypassDelay`
- `bypassSaturation`
- `bypassFilters`
- `bypassFeedback`

### Final Parameter Count
- **Before:** 13 parameters
- **After:** 11 parameters (-2 simplified)

---

## GUI Knob Repurposing

### Main Knob (168px center)
- **OLD:** Doppler Shift (-50% to +50%)
- **NEW:** Delay Time (10-2000ms, logarithmic)
- **Label:** "doppler effect delay" → "delay time"
- **Value:** "0.0%" → "260ms" (or "1.5s" for >1000ms)

### Top-Left Sub-Knob (130px)
- **OLD:** Stereo Width (-100% to +100%)
- **NEW:** Ping-Pong Amount (0% to 100%)
- **Label:** "stereo width" → "ping-pong"
- **Value:** "0%" → "0%" (format unchanged, meaning different)

### Other 5 Knobs
- **UNCHANGED:** feedback, masterOutput, loCut, saturation, hiCut

---

## DSP Component Changes

### Components REMOVED
- `stereoWidthDelayL` / `stereoWidthDelayR` (delay lines)
- `struct GrainState` (grain playback state)
- `std::array<GrainState, 4> grainsL` / `grainsR` (grain arrays)
- `grainBufferL` / `grainBufferR` (grain circular buffers)
- `grainSpacingSamples` / `samplesSinceLastGrain` (scheduling)

### Components ADDED
- `float pingPongBufferL` / `pingPongBufferR` (cross-feedback state)

### Components MODIFIED
- `tapeDelayL` / `tapeDelayR`: Increased max delay from 260ms → 2000ms

---

## Expected Benefits

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| CPU Usage | 37-57% | 10-15% | **-70%** |
| Code Complexity | 660 lines | ~400 lines | **-39%** |
| Memory Usage | +600KB (grains) | 0KB extra | **-600KB** |
| Latency | 460ms (fixed) | 10-2000ms (variable) | User-controllable |

---

## Preset Compatibility Warning

⚠️ **BREAKING CHANGE:** Old presets will load but show warnings for missing parameters:
- `stereoWidth` → Uses default (no stereo width)
- `dopplerShift` → Uses `delayTime` default (260ms)
- `grainSize` / `grainOverlap` → Ignored (granular removed)

New parameters will use defaults:
- `delayTime` = 260ms
- `pingPongAmount` = 0%

**User Impact:** Presets need one-time adjustment after migration.

---

## Rollback Instructions

If migration fails:
```bash
git checkout backup/pre-tape-delay-migration-20260213-010141
./scripts/build-and-install.sh RedShiftDistortion
```

---

## Migration Status

- [x] Phase 0: Backup created
- [ ] Phase 1: Parameter layout updated
- [ ] Phase 2: DSP components removed
- [ ] Phase 3: prepareToPlay updated
- [ ] Phase 4: processBlock rewritten
- [ ] Phase 5: GUI knobs rebound
- [ ] Phase 6: Build & test complete

**Last Updated:** 2026-02-13 (Phase 0 complete)
