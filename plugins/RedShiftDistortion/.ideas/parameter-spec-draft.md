# Parameter Specification (Draft)

**Status:** Draft - Awaiting UI mockup for full specification
**Created:** 2026-02-05 21:35:00
**Source:** Quick capture during ideation

This is a lightweight specification to enable parallel DSP research.
Full specification will be generated from finalized UI mockup.

## Parameters

### saturation
- **Type:** Float
- **Range:** -12.0 to 24.0 dB
- **Default:** 0.0 dB
- **DSP Purpose:** Controls tube-style harmonic distortion drive amount applied to the distortion path

### dopplerShift
- **Type:** Float
- **Range:** -50.0 to 50.0 %
- **Default:** 0.0 %
- **DSP Purpose:** Bidirectional pitch/time shift intensity (negative = red shift/receding, positive = blue shift/approaching). Controls delay time modulation, pitch shifting, and time stretching simultaneously.

### pitchEnable
- **Type:** Bool
- **Default:** true (On)
- **DSP Purpose:** Toggles pitch shifting component of doppler effect. When off, only time stretching is applied.

### delayTime
- **Type:** Float (tempo-sync aware)
- **Range:** 1/16 to 8 bars (tempo-synced) OR 0-16000 ms (free time)
- **Default:** 1/4 note (or 500ms in free mode)
- **DSP Purpose:** Sets the delay line length. Switches between musical subdivisions (tempo-synced) and milliseconds (free time) based on tempoSync parameter.

### tempoSync
- **Type:** Bool
- **Default:** true (On)
- **DSP Purpose:** Locks delay time to host tempo (musical subdivisions) when on, or uses free millisecond timing when off.

### delayLevel
- **Type:** Float
- **Range:** -60.0 to 0.0 dB
- **Default:** -6.0 dB (50%)
- **DSP Purpose:** Volume control for the delay path in the parallel mix before master output.

### distortionLevel
- **Type:** Float
- **Range:** -60.0 to 0.0 dB
- **Default:** -6.0 dB (50%)
- **DSP Purpose:** Volume control for the distortion path in the parallel mix before master output.

### masterOutput
- **Type:** Float
- **Range:** -60.0 to 12.0 dB
- **Default:** 0.0 dB
- **DSP Purpose:** Final output level control after parallel mixing of delay and distortion paths.

## Next Steps

- [ ] Complete UI mockup workflow (/dream → option 3)
- [ ] Finalize design and generate full parameter-spec.md
- [ ] Validate consistency between draft and final spec
