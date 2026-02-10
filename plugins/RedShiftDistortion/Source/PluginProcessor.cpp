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

    // Calculate target stereo control (-1.0 to +1.0)
    const float targetStereoControl = bypassStereoWidth ? 0.0f : (stereoWidthParam / 100.0f);

    // Calculate master output gain (dB to linear)
    const float masterGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // Get sample rate for delay time calculations
    const float sampleRate = static_cast<float>(getSampleRate());

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
