#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter layout creation (BEFORE constructor)
juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // MAIN CONTROLS (7 FloatParameters)

    // 1. STEREO_WIDTH - Stereo Width (-100% to +100%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "STEREO_WIDTH", 1 },
        "Stereo Width",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 2. FEEDBACK - Feedback (0% to 95%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FEEDBACK", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 3. FILTER_BAND_LOW - Lo-Cut (20Hz to 20000Hz, default: 100Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_LOW", 1 },
        "Lo-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        100.0f,
        "Hz"
    ));

    // 4. FILTER_BAND_HIGH - Hi-Cut (20Hz to 20000Hz, default: 8000Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_HIGH", 1 },
        "Hi-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        8000.0f,
        "Hz"
    ));

    // 5. DOPPLER_SHIFT - Doppler Shift (-50% to +50%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DOPPLER_SHIFT", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 6. SATURATION - Saturation (-12dB to +24dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SATURATION", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // 7. MASTER_OUTPUT - Master Output (-60dB to +12dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "MASTER_OUTPUT", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // BYPASS CONTROLS (4 BoolParameters)

    // 8. BYPASS_STEREO_WIDTH - Bypass Stereo Width (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_STEREO_WIDTH", 1 },
        "Bypass Stereo Width",
        false
    ));

    // 9. BYPASS_DELAY - Bypass Delay (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_DELAY", 1 },
        "Bypass Delay",
        false
    ));

    // 10. BYPASS_DOPPLER - Bypass Doppler (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_DOPPLER", 1 },
        "Bypass Doppler",
        false
    ));

    // 11. BYPASS_SATURATION - Bypass Saturation (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_SATURATION", 1 },
        "Bypass Saturation",
        false
    ));

    // ADVANCED SETTINGS (2 parameters)

    // 12. GRAIN_SIZE - Grain Size (25ms to 200ms, default: 100ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "GRAIN_SIZE", 1 },
        "Grain Size",
        juce::NormalisableRange<float>(25.0f, 200.0f, 1.0f),
        100.0f,
        "ms"
    ));

    // 13. GRAIN_OVERLAP - Grain Overlap (2x or 4x, default: 4x = index 1)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "GRAIN_OVERLAP", 1 },
        "Grain Overlap",
        juce::StringArray { "2x", "4x" },
        1  // Default: 4x (index 1)
    ));

    return layout;
}

RedShiftDistortionAudioProcessor::RedShiftDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)   // Effect: stereo input
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)) // Effect: stereo output
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

RedShiftDistortionAudioProcessor::~RedShiftDistortionAudioProcessor()
{
}

void RedShiftDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Create ProcessSpec for juce::dsp components (Pattern #17)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    currentSampleRate = static_cast<float>(sampleRate);

    // Calculate stereo width smoothing coefficient (10ms time constant)
    const float smoothingTimeMs = 10.0f;
    stereoSmoothingCoeff = 1.0f - std::exp(-1.0f / (smoothingTimeMs * 0.001f * static_cast<float>(sampleRate)));

    // Calculate maximum delay in samples
    const int maxStereoWidthDelaySamples = static_cast<int>(std::ceil((ITD_HALF_MS / 1000.0f) * sampleRate));
    const int maxTapeDelaySamples = static_cast<int>(std::ceil((MAX_DELAY_MS / 1000.0f) * sampleRate));

    // Initialize stereo width delay lines (Stage 1)
    stereoWidthDelayLeft.prepare(spec);
    stereoWidthDelayLeft.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayLeft.reset();

    stereoWidthDelayRight.prepare(spec);
    stereoWidthDelayRight.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayRight.reset();

    // Initialize tape delay line (Stage 2)
    tapeDelay.prepare(spec);
    tapeDelay.setMaximumDelayInSamples(maxTapeDelaySamples);
    tapeDelay.reset();

    // Reset smoothed control
    smoothedStereoControl = 0.0f;

    // Initialize granular engines for both channels (Phase 2.2)
    for (auto& engine : grainEngines)
    {
        // Clear grain buffer
        engine.grainBuffer.fill(0.0f);
        engine.grainBufferWritePos = 0;

        // Pre-calculate Hann window for maximum grain size
        for (int i = 0; i < GrainEngine::MAX_GRAIN_SIZE_SAMPLES; ++i)
        {
            const float phase = static_cast<float>(i) / static_cast<float>(GrainEngine::MAX_GRAIN_SIZE_SAMPLES);
            engine.hannWindow[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));
        }

        // Reset all grains
        for (auto& grain : engine.grains)
        {
            grain.active = false;
            grain.readPosition = 0.0f;
            grain.grainPhase = 0;
        }

        engine.samplesUntilNextGrain = 0;
        engine.grainAdvanceSamples = 0;
    }
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (Phase 2.1+)
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Early exit if no channels
    if (numChannels == 0 || numSamples == 0)
        return;

    // Read parameters (atomic, real-time safe)
    const float stereoWidthParam = parameters.getRawParameterValue("STEREO_WIDTH")->load();
    const bool bypassStereoWidth = parameters.getRawParameterValue("BYPASS_STEREO_WIDTH")->load() > 0.5f;
    const bool bypassDelay = parameters.getRawParameterValue("BYPASS_DELAY")->load() > 0.5f;
    const float masterOutputDb = parameters.getRawParameterValue("MASTER_OUTPUT")->load();

    // Granular parameters (Phase 2.2)
    const float dopplerShiftParam = parameters.getRawParameterValue("DOPPLER_SHIFT")->load();
    const float grainSizeMs = parameters.getRawParameterValue("GRAIN_SIZE")->load();
    const int grainOverlapIndex = static_cast<int>(parameters.getRawParameterValue("GRAIN_OVERLAP")->load());
    const bool bypassDoppler = parameters.getRawParameterValue("BYPASS_DOPPLER")->load() > 0.5f;

    // Calculate target stereo control (-1.0 to +1.0)
    const float targetStereoControl = bypassStereoWidth ? 0.0f : (stereoWidthParam / 100.0f);

    // Calculate master output gain (dB to linear)
    const float masterGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // Get sample rate for delay time calculations
    const float sampleRate = static_cast<float>(getSampleRate());

    // Calculate pitch ratio from doppler shift (Phase 2.2)
    const float semitones = (dopplerShiftParam / 100.0f) * 12.0f;  // ±50% → ±12 semitones
    const float pitchRatio = std::pow(2.0f, semitones / 12.0f);

    // Calculate grain parameters (Phase 2.2)
    const int grainSizeSamples = static_cast<int>(std::ceil((grainSizeMs / 1000.0f) * currentSampleRate));
    const int grainOverlap = (grainOverlapIndex == 0) ? 2 : 4;  // 2x or 4x
    const int grainAdvanceSamples = grainSizeSamples / grainOverlap;

    // Clamp grain size to maximum
    const int actualGrainSize = std::min(grainSizeSamples, static_cast<int>(GrainEngine::MAX_GRAIN_SIZE_SAMPLES));

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Smooth stereo control (exponential smoothing, 10ms time constant)
        smoothedStereoControl += (targetStereoControl - smoothedStereoControl) * stereoSmoothingCoeff;

        // Calculate L/R delay times (ITD_HALF = 260ms maximum)
        const float stereoDelayTimeSamples = (ITD_HALF_MS / 1000.0f) * sampleRate * smoothedStereoControl;
        const float leftDelaySamples = stereoDelayTimeSamples;   // Positive offset
        const float rightDelaySamples = -stereoDelayTimeSamples;  // Negative offset (opposite)

        // STAGE 1: Stereo Width Modulation
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            const float input = channelData[sample];

            if (ch == 0)  // Left channel
            {
                stereoWidthDelayLeft.pushSample(0, input);
                stereoWidthDelayLeft.setDelay(std::abs(leftDelaySamples));
                channelData[sample] = stereoWidthDelayLeft.popSample(0);
            }
            else if (ch == 1)  // Right channel
            {
                stereoWidthDelayRight.pushSample(0, input);
                stereoWidthDelayRight.setDelay(std::abs(rightDelaySamples));
                channelData[sample] = stereoWidthDelayRight.popSample(0);
            }
        }

        // STAGE 3: Granular Pitch Shifter (Phase 2.2 - isolated testing)
        if (!bypassDoppler && std::abs(pitchRatio - 1.0f) > 0.001f)  // Only process if pitch shift active
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* channelData = buffer.getWritePointer(ch);
                const float stereoWidthOutput = channelData[sample];
                auto& engine = grainEngines[ch];

                // Write to grain buffer (circular)
                engine.grainBuffer[engine.grainBufferWritePos] = stereoWidthOutput;
                engine.grainBufferWritePos = (engine.grainBufferWritePos + 1) % actualGrainSize;

                // Update grain advance counter
                engine.grainAdvanceSamples = grainAdvanceSamples;

                // Spawn new grain if needed
                if (engine.samplesUntilNextGrain <= 0)
                {
                    // Find inactive grain slot
                    for (auto& grain : engine.grains)
                    {
                        if (!grain.active)
                        {
                            grain.active = true;
                            grain.readPosition = 0.0f;
                            grain.grainPhase = 0;
                            break;
                        }
                    }

                    // Reset spawn counter
                    engine.samplesUntilNextGrain = grainAdvanceSamples;
                }
                engine.samplesUntilNextGrain--;

                // Overlap-add all active grains
                float grainOutput = 0.0f;
                for (auto& grain : engine.grains)
                {
                    if (!grain.active)
                        continue;

                    // Read from grain buffer at fractional position (linear interpolation)
                    const int readPosInt = static_cast<int>(grain.readPosition);
                    const float readPosFrac = grain.readPosition - static_cast<float>(readPosInt);

                    const int idx0 = readPosInt % actualGrainSize;
                    const int idx1 = (readPosInt + 1) % actualGrainSize;

                    const float sample0 = engine.grainBuffer[idx0];
                    const float sample1 = engine.grainBuffer[idx1];
                    const float grainSample = sample0 + readPosFrac * (sample1 - sample0);

                    // Apply Hann window
                    const float windowPhase = static_cast<float>(grain.grainPhase) / static_cast<float>(actualGrainSize);
                    const int windowIdx = static_cast<int>(windowPhase * static_cast<float>(GrainEngine::MAX_GRAIN_SIZE_SAMPLES));
                    const float windowValue = engine.hannWindow[windowIdx];

                    grainOutput += grainSample * windowValue;

                    // Advance grain playback (variable rate for pitch shifting)
                    grain.readPosition += pitchRatio;
                    grain.grainPhase++;

                    // Deactivate grain when complete
                    if (grain.grainPhase >= actualGrainSize)
                    {
                        grain.active = false;
                    }
                }

                // Normalize output by number of overlapping grains
                const float normalizationFactor = 1.0f / static_cast<float>(grainOverlap);
                channelData[sample] = grainOutput * normalizationFactor;
            }
        }

        // STAGE 2: Tape Delay (basic pass-through for Phase 2.1, no feedback yet)
        if (!bypassDelay)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* channelData = buffer.getWritePointer(ch);
                const float stereoWidthOutput = channelData[sample];

                // Push to delay line, pop immediately (0ms delay = pass-through)
                tapeDelay.pushSample(ch, stereoWidthOutput);
                tapeDelay.setDelay(0.0f);  // No delay yet (Phase 2.3 will add feedback)
                channelData[sample] = tapeDelay.popSample(ch);
            }
        }

        // STAGE 5: Master Output Gain
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            channelData[sample] *= masterGain;
        }
    }
}

juce::AudioProcessorEditor* RedShiftDistortionAudioProcessor::createEditor()
{
    return new RedShiftDistortionAudioProcessorEditor(*this);
}

void RedShiftDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RedShiftDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RedShiftDistortionAudioProcessor();
}
