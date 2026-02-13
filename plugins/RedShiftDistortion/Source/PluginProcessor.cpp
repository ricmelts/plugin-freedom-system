#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter layout creation (BEFORE constructor)
juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Main Controls (renamed/repurposed for tape delay architecture)

    // delayTime - Tape delay time (user-controllable, replaces dopplerShift)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 0.1f, 0.4f),  // Logarithmic skew
        260.0f,  // Default matches old base delay
        "ms"
    ));

    // pingPongAmount - Ping-pong feedback amount (replaces stereoWidth)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "pingPongAmount", 1 },
        "Ping Pong",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        0.0f,  // Default: no ping-pong
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

    // saturation - Tube saturation drive (in feedback loop)
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

    // Bypass Controls (4 bool parameters for tape delay architecture)

    // bypassPingPong - Bypass ping-pong mode (replaces bypassStereoWidth)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassPingPong", 1 },
        "Bypass Ping Pong",
        false
    ));

    // bypassDelay - Bypass entire delay + feedback loop
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassDelay", 1 },
        "Bypass Delay",
        false
    ));

    // bypassSaturation - Bypass tube saturation stage
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassSaturation", 1 },
        "Bypass Saturation",
        false
    ));

    // bypassFilters - Bypass hi-cut and lo-cut filters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassFilters", 1 },
        "Bypass Filters",
        false
    ));

    // bypassFeedback - Bypass feedback loop
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassFeedback", 1 },
        "Bypass Feedback",
        false
    ));

    // Note: Granular-specific parameters (grainSize, grainOverlap) removed for tape delay architecture

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

    // Calculate max delay time: 2000ms at 192kHz = 384,000 samples
    const int maxDelaySamples = static_cast<int>(2.0 * sampleRate);  // 2000ms max

    // Prepare tape delay lines (increased buffer size for user-controllable delay time)
    tapeDelayL.prepare(spec);
    tapeDelayL.setMaximumDelayInSamples(maxDelaySamples);
    tapeDelayL.reset();

    tapeDelayR.prepare(spec);
    tapeDelayR.setMaximumDelayInSamples(maxDelaySamples);
    tapeDelayR.reset();

    // Prepare saturation waveshapers
    saturationL.prepare(spec);
    saturationL.functionToUse = [](float x) { return std::tanh(x); };
    saturationL.reset();

    saturationR.prepare(spec);
    saturationR.functionToUse = [](float x) { return std::tanh(x); };
    saturationR.reset();

    // Phase 2.2: Prepare dual-band filters
    hiPassFilterL.prepare(spec);
    hiPassFilterL.reset();

    hiPassFilterR.prepare(spec);
    hiPassFilterR.reset();

    loPassFilterL.prepare(spec);
    loPassFilterL.reset();

    loPassFilterR.prepare(spec);
    loPassFilterR.reset();

    // Initialize filter coefficients (will be updated in processBlock when parameters change)
    *hiPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 100.0, 0.707);
    *hiPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 100.0, 0.707);
    *loPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0, 0.707);
    *loPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0, 0.707);

    // Reset feedback state
    feedbackStateL = 0.0f;
    feedbackStateR = 0.0f;

    // Reset ping-pong buffers
    pingPongBufferL = 0.0f;
    pingPongBufferR = 0.0f;
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

    // Read parameters for one-sided tape delay architecture
    auto* delayTimeParam = parameters.getRawParameterValue("delayTime");
    auto* pingPongAmountParam = parameters.getRawParameterValue("pingPongAmount");
    auto* bypassDelayParam = parameters.getRawParameterValue("bypassDelay");
    auto* bypassPingPongParam = parameters.getRawParameterValue("bypassPingPong");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    // Feedback loop parameters (unchanged from granular version)
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* saturationParam = parameters.getRawParameterValue("saturation");
    auto* filterBandLowParam = parameters.getRawParameterValue("filterBandLow");
    auto* filterBandHighParam = parameters.getRawParameterValue("filterBandHigh");
    auto* bypassSaturationParam = parameters.getRawParameterValue("bypassSaturation");
    auto* bypassFiltersParam = parameters.getRawParameterValue("bypassFilters");
    auto* bypassFeedbackParam = parameters.getRawParameterValue("bypassFeedback");

    // Load parameter values
    float delayTimeMs = delayTimeParam->load();
    float pingPongAmount = pingPongAmountParam->load() / 100.0f;  // 0.0 to 1.0
    bool bypassDelay = bypassDelayParam->load() > 0.5f;
    bool bypassPingPong = bypassPingPongParam->load() > 0.5f;
    float masterOutputDB = masterOutputParam->load();

    float feedbackValue = feedbackParam->load() / 100.0f;
    float saturationDB = saturationParam->load();
    float filterBandLow = filterBandLowParam->load();
    float filterBandHigh = filterBandHighParam->load();
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;
    bool bypassFilters = bypassFiltersParam->load() > 0.5f;
    bool bypassFeedback = bypassFeedbackParam->load() > 0.5f;

    // If delay bypassed, only apply master output gain
    if (bypassDelay)
    {
        float masterGain = std::pow(10.0f, masterOutputDB / 20.0f);
        buffer.applyGain(masterGain);
        return;
    }

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Only process stereo (L + R)
    if (numChannels < 2)
        return;

    // Calculate delay time in samples
    const float delaySamples = (delayTimeMs / 1000.0f) * static_cast<float>(currentSampleRate);
    tapeDelayR.setDelay(delaySamples);  // Right channel is always wet

    // If ping-pong is active, also set left channel delay
    if (!bypassPingPong && pingPongAmount > 0.0f)
        tapeDelayL.setDelay(delaySamples);

    // Update filter coefficients (swap if inverted)
    float actualFilterBandLow = filterBandLow;
    float actualFilterBandHigh = filterBandHigh;
    if (actualFilterBandLow > actualFilterBandHigh)
        std::swap(actualFilterBandLow, actualFilterBandHigh);

    actualFilterBandLow = juce::jlimit(20.0f, 20000.0f, actualFilterBandLow);
    actualFilterBandHigh = juce::jlimit(20.0f, 20000.0f, actualFilterBandHigh);

    *hiPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, actualFilterBandLow, 0.707);
    *hiPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, actualFilterBandLow, 0.707);
    *loPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, actualFilterBandHigh, 0.707);
    *loPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, actualFilterBandHigh, 0.707);

    // Calculate saturation gain
    float saturationGain = std::pow(10.0f, saturationDB / 20.0f);

    // Get channel pointers
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Tube saturation function (asymmetrical)
    auto tubeSaturation = [](float input, float gain) -> float {
        float driven = input * gain;
        const float asymmetry = 0.15f;

        if (driven > 0.0f)
        {
            float positiveDrive = driven * (1.0f + asymmetry);
            float saturated = std::tanh(positiveDrive);
            float secondHarmonic = driven * driven * 0.1f;
            return saturated + secondHarmonic * (1.0f - saturated);
        }
        else
        {
            float negativeDrive = driven * (1.0f - asymmetry);
            float saturated = std::tanh(negativeDrive);
            return saturated * 0.95f;
        }
    };

    // Process each sample - ONE-SIDED TAPE DELAY ARCHITECTURE
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // === LEFT CHANNEL: DRY PASSTHROUGH (unless ping-pong active) ===
        float dryL = leftChannel[sample];

        // === RIGHT CHANNEL: WET PROCESSING ===
        float inputR = rightChannel[sample];

        // Mix input with feedback
        float inputWithFeedbackR = inputR + feedbackStateR;

        // Push to delay line
        tapeDelayR.pushSample(0, inputWithFeedbackR);

        // Read delayed signal
        float delayedR = tapeDelayR.popSample(0);

        // === FEEDBACK LOOP (Right Channel) ===

        // Apply saturation
        float saturatedR = delayedR;
        if (!bypassSaturation)
            saturatedR = tubeSaturation(delayedR, saturationGain);

        // Apply filters
        float filteredR = saturatedR;
        if (!bypassFilters)
        {
            filteredR = hiPassFilterR.processSample(filteredR);
            filteredR = loPassFilterR.processSample(filteredR);
        }

        // === PING-PONG PROCESSING ===

        if (!bypassPingPong && pingPongAmount > 0.0f)
        {
            // Cross-feedback: R → L and L → R
            float crossFeedbackL = filteredR * feedbackValue * pingPongAmount;
            float crossFeedbackR = pingPongBufferL * feedbackValue * pingPongAmount;

            // Regular feedback (same channel)
            float regularFeedbackR = filteredR * feedbackValue * (1.0f - pingPongAmount);

            // Apply feedback
            if (!bypassFeedback)
            {
                feedbackStateR = regularFeedbackR + crossFeedbackR;
                pingPongBufferL = crossFeedbackL;
            }
            else
            {
                feedbackStateR = 0.0f;
                pingPongBufferL = 0.0f;
            }

            // Process left channel delay when ping-pong active
            float inputWithFeedbackL = dryL + pingPongBufferL;
            tapeDelayL.pushSample(0, inputWithFeedbackL);
            float delayedL = tapeDelayL.popSample(0);

            // Apply processing to left delay
            float saturatedL = delayedL;
            if (!bypassSaturation)
                saturatedL = tubeSaturation(delayedL, saturationGain);

            float filteredL = saturatedL;
            if (!bypassFilters)
            {
                filteredL = hiPassFilterL.processSample(filteredL);
                filteredL = loPassFilterL.processSample(filteredL);
            }

            // Update cross-feedback buffer for next iteration
            pingPongBufferL = filteredL;

            // Left channel output: mix dry + ping-pong delayed
            leftChannel[sample] = dryL * (1.0f - pingPongAmount) + filteredL * pingPongAmount;
        }
        else
        {
            // No ping-pong: standard feedback on right channel only
            if (!bypassFeedback)
                feedbackStateR = filteredR * feedbackValue;
            else
                feedbackStateR = 0.0f;

            // Left channel: pure dry
            leftChannel[sample] = dryL;
        }

        // Right channel: always wet
        rightChannel[sample] = delayedR;
    }

    // Calculate output levels for VU meters
    float maxLevelL = 0.0f;
    float maxLevelR = 0.0f;
    for (int sample = 0; sample < numSamples; ++sample)
    {
        maxLevelL = std::max(maxLevelL, std::abs(leftChannel[sample]));
        maxLevelR = std::max(maxLevelR, std::abs(rightChannel[sample]));
    }
    outputLevelL.store(maxLevelL);
    outputLevelR.store(maxLevelR);

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
