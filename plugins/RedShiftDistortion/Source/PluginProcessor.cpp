#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter layout creation (BEFORE constructor)
juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Main Controls (7 float parameters)

    // stereoWidth - Spatial positioning via L/R delay differential
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "stereoWidth", 1 },
        "Stereo Width",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f, 1.0f),
        0.0f,
        "%"
    ));

    // feedback - Delay feedback amount (0-95%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f, 1.0f),
        0.0f,
        "%"
    ));

    // filterBandLow - Highpass cutoff in feedback loop
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "filterBandLow", 1 },
        "Lo-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f),  // Logarithmic skew
        100.0f,
        "Hz"
    ));

    // filterBandHigh - Lowpass cutoff in feedback loop
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "filterBandHigh", 1 },
        "Hi-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f),  // Logarithmic skew
        8000.0f,
        "Hz"
    ));

    // dopplerShift - Pitch shift per repeat (±12 semitones, CUMULATIVE)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "dopplerShift", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 1.0f),
        0.0f,
        "%"
    ));

    // saturation - Tube saturation drive (CUMULATIVE in feedback loop)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "saturation", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f, 1.0f),
        0.0f,
        "dB"
    ));

    // masterOutput - Final output level control
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "masterOutput", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f, 2.0f),  // Logarithmic skew
        0.0f,
        "dB"
    ));

    // Bypass Controls (4 bool parameters)

    // bypassStereoWidth - Bypass stereo width modulation (mono output)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassStereoWidth", 1 },
        "Bypass Stereo Width",
        false
    ));

    // bypassDelay - Bypass entire delay + feedback loop
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassDelay", 1 },
        "Bypass Delay",
        false
    ));

    // bypassDoppler - Bypass granular pitch shifting
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassDoppler", 1 },
        "Bypass Doppler",
        false
    ));

    // bypassSaturation - Bypass tube saturation stage
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassSaturation", 1 },
        "Bypass Saturation",
        false
    ));

    // Advanced Settings (2 parameters)

    // grainSize - Grain buffer size for granular pitch shifting
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "grainSize", 1 },
        "Grain Size",
        juce::NormalisableRange<float>(25.0f, 200.0f, 1.0f, 1.0f),
        100.0f,
        "ms"
    ));

    // grainOverlap - Simultaneous grain count (2x or 4x)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "grainOverlap", 1 },
        "Grain Overlap",
        juce::StringArray { "2x", "4x" },
        1  // Default: "4x" (index 1)
    ));

    return layout;
}

RedShiftDistortionAudioProcessor::RedShiftDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

RedShiftDistortionAudioProcessor::~RedShiftDistortionAudioProcessor()
{
}

void RedShiftDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Store sample rate for delay time calculations
    currentSampleRate = sampleRate;

    // Prepare DSP spec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Calculate max delay time in samples
    // Stereo width: ±260ms max differential + 260ms base = 520ms total
    // Tape delay: 260ms base
    // Total max: 520ms at 192kHz = 99,840 samples
    const int maxStereoWidthDelaySamples = static_cast<int>(0.520 * sampleRate);  // 520ms
    const int maxTapeDelaySamples = static_cast<int>(0.260 * sampleRate);        // 260ms

    // Prepare stereo width delay lines
    stereoWidthDelayL.prepare(spec);
    stereoWidthDelayL.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayL.reset();

    stereoWidthDelayR.prepare(spec);
    stereoWidthDelayR.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayR.reset();

    // Prepare tape delay lines
    tapeDelayL.prepare(spec);
    tapeDelayL.setMaximumDelayInSamples(maxTapeDelaySamples);
    tapeDelayL.reset();

    tapeDelayR.prepare(spec);
    tapeDelayR.setMaximumDelayInSamples(maxTapeDelaySamples);
    tapeDelayR.reset();
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Optional: Release delay buffers to save memory when plugin not in use
    // DelayLine components handle their own cleanup automatically
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters (atomic, real-time safe)
    auto* stereoWidthParam = parameters.getRawParameterValue("stereoWidth");
    auto* bypassDelayParam = parameters.getRawParameterValue("bypassDelay");
    auto* bypassStereoWidthParam = parameters.getRawParameterValue("bypassStereoWidth");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    float stereoWidthValue = stereoWidthParam->load();
    bool bypassDelay = bypassDelayParam->load() > 0.5f;
    bool bypassStereoWidth = bypassStereoWidthParam->load() > 0.5f;
    float masterOutputDB = masterOutputParam->load();

    // Apply bypass stereo width (set to 0% = mono)
    if (bypassStereoWidth)
        stereoWidthValue = 0.0f;

    // If delay bypassed, only apply master output gain and return
    if (bypassDelay)
    {
        // Apply master output gain
        float masterGain = std::pow(10.0f, masterOutputDB / 20.0f);
        buffer.applyGain(masterGain);
        return;
    }

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Only process stereo (L + R)
    if (numChannels < 2)
        return;

    // Calculate stereo width delay times
    // Base delay: 260ms (fixed)
    // Differential: (stereoWidth / 100.0) * 260ms
    // L channel delay: 260ms + (stereoWidth < 0 ? differential : 0)
    // R channel delay: 260ms + (stereoWidth > 0 ? differential : 0)
    const float baseDelayMs = 260.0f;
    const float differentialMs = (stereoWidthValue / 100.0f) * 260.0f;

    float lChannelDelayMs = baseDelayMs;
    float rChannelDelayMs = baseDelayMs;

    if (stereoWidthValue < 0.0f)
    {
        // Negative width: L channel delayed more
        lChannelDelayMs += std::abs(differentialMs);
    }
    else if (stereoWidthValue > 0.0f)
    {
        // Positive width: R channel delayed more
        rChannelDelayMs += differentialMs;
    }
    // At stereoWidth = 0: Both channels equal (mono)

    // Convert delay times to samples
    const float lChannelDelaySamples = (lChannelDelayMs / 1000.0f) * static_cast<float>(currentSampleRate);
    const float rChannelDelaySamples = (rChannelDelayMs / 1000.0f) * static_cast<float>(currentSampleRate);

    // Set stereo width delay times
    stereoWidthDelayL.setDelay(lChannelDelaySamples);
    stereoWidthDelayR.setDelay(rChannelDelaySamples);

    // Fixed tape delay time: 260ms
    const float tapeDelaySamples = (baseDelayMs / 1000.0f) * static_cast<float>(currentSampleRate);
    tapeDelayL.setDelay(tapeDelaySamples);
    tapeDelayR.setDelay(tapeDelaySamples);

    // Get channel pointers
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Phase 2.1: Stereo Width → Tape Delay (no feedback yet)

        // 1. Push current input into stereo width delay
        stereoWidthDelayL.pushSample(0, leftChannel[sample]);
        stereoWidthDelayR.pushSample(0, rightChannel[sample]);

        // 2. Read stereo-widened signal
        float stereoWidenedL = stereoWidthDelayL.popSample(0);
        float stereoWidenedR = stereoWidthDelayR.popSample(0);

        // 3. Push stereo-widened signal into tape delay
        tapeDelayL.pushSample(0, stereoWidenedL);
        tapeDelayR.pushSample(0, stereoWidenedR);

        // 4. Read delayed signal (260ms delay)
        float delayedL = tapeDelayL.popSample(0);
        float delayedR = tapeDelayR.popSample(0);

        // 5. Output = delayed signal (100% wet, no dry mix in Phase 2.1)
        leftChannel[sample] = delayedL;
        rightChannel[sample] = delayedR;
    }

    // Apply master output gain
    float masterGain = std::pow(10.0f, masterOutputDB / 20.0f);
    buffer.applyGain(masterGain);
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
