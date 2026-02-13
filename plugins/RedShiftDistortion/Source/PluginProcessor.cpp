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

    // Phase 2.2: Prepare saturation waveshapers
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

    // Phase 2.2: Read feedback loop parameters
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* saturationParam = parameters.getRawParameterValue("saturation");
    auto* filterBandLowParam = parameters.getRawParameterValue("filterBandLow");
    auto* filterBandHighParam = parameters.getRawParameterValue("filterBandHigh");
    auto* bypassSaturationParam = parameters.getRawParameterValue("bypassSaturation");

    float stereoWidthValue = stereoWidthParam->load();
    bool bypassDelay = bypassDelayParam->load() > 0.5f;
    bool bypassStereoWidth = bypassStereoWidthParam->load() > 0.5f;
    float masterOutputDB = masterOutputParam->load();

    // Phase 2.2: Load feedback loop parameter values
    float feedbackValue = feedbackParam->load() / 100.0f;  // Convert 0-95% to 0.0-0.95
    float saturationDB = saturationParam->load();
    float filterBandLow = filterBandLowParam->load();
    float filterBandHigh = filterBandHighParam->load();
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;

    // Apply bypass stereo width (set to 0% = mono)
    if (bypassStereoWidth)
        stereoWidthValue = 0.0f;

    // Phase 2.2: Update filter coefficients if parameters changed
    // Edge case: If lo-cut > hi-cut, swap values to ensure valid bandpass range
    float actualFilterBandLow = filterBandLow;
    float actualFilterBandHigh = filterBandHigh;
    if (actualFilterBandLow > actualFilterBandHigh)
        std::swap(actualFilterBandLow, actualFilterBandHigh);

    // Update filter coefficients (clamp to 20Hz-20kHz to avoid Nyquist issues)
    actualFilterBandLow = juce::jlimit(20.0f, 20000.0f, actualFilterBandLow);
    actualFilterBandHigh = juce::jlimit(20.0f, 20000.0f, actualFilterBandHigh);

    *hiPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, actualFilterBandLow, 0.707);
    *hiPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, actualFilterBandLow, 0.707);
    *loPassFilterL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, actualFilterBandHigh, 0.707);
    *loPassFilterR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, actualFilterBandHigh, 0.707);

    // Phase 2.2: Calculate saturation gain
    // gain = pow(10.0, saturationDB / 20.0)
    // -12dB → 0.251x, 0dB → 1.0x, +24dB → 15.85x
    float saturationGain = std::pow(10.0f, saturationDB / 20.0f);

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
        // Phase 2.2: Stereo Width → Tape Delay + Feedback Loop (Saturation + Filters)

        // 1. Mix current input with feedback signal
        float inputWithFeedbackL = leftChannel[sample] + feedbackStateL;
        float inputWithFeedbackR = rightChannel[sample] + feedbackStateR;

        // 2. Push mixed input into stereo width delay
        stereoWidthDelayL.pushSample(0, inputWithFeedbackL);
        stereoWidthDelayR.pushSample(0, inputWithFeedbackR);

        // 3. Read stereo-widened signal
        float stereoWidenedL = stereoWidthDelayL.popSample(0);
        float stereoWidenedR = stereoWidthDelayR.popSample(0);

        // 4. Push stereo-widened signal into tape delay
        tapeDelayL.pushSample(0, stereoWidenedL);
        tapeDelayR.pushSample(0, stereoWidenedR);

        // 5. Read delayed signal (260ms delay)
        float delayedL = tapeDelayL.popSample(0);
        float delayedR = tapeDelayR.popSample(0);

        // === FEEDBACK LOOP START ===

        // 6. Apply saturation (if not bypassed)
        float saturatedL = delayedL;
        float saturatedR = delayedR;

        if (!bypassSaturation)
        {
            // Apply gain, then tanh waveshaping
            saturatedL = std::tanh(saturationGain * delayedL);
            saturatedR = std::tanh(saturationGain * delayedR);
        }

        // 7. Apply dual-band filtering (hi-pass → lo-pass)
        float filteredL = saturatedL;
        float filteredR = saturatedR;

        // Hi-pass filter (lo-cut)
        filteredL = hiPassFilterL.processSample(filteredL);
        filteredR = hiPassFilterR.processSample(filteredR);

        // Lo-pass filter (hi-cut)
        filteredL = loPassFilterL.processSample(filteredL);
        filteredR = loPassFilterR.processSample(filteredR);

        // 8. Scale by feedback gain and store for next iteration
        feedbackStateL = filteredL * feedbackValue;
        feedbackStateR = filteredR * feedbackValue;

        // === FEEDBACK LOOP END ===

        // 9. Output = delayed signal (100% wet, no dry mix)
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
