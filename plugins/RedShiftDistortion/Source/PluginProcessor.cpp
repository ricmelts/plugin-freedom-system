#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // saturation - Float (-12.0 to 24.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "saturation", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // dopplerShift - Float (-50.0 to 50.0 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "dopplerShift", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // feedback - Float (0.0 to 95.0 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f,
        "%"
    ));

    // hiCut - Float (2000.0 to 10000.0 Hz) for tape delay feedback filtering
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "hiCut", 1 },
        "Hi Cut",
        juce::NormalisableRange<float>(2000.0f, 10000.0f, 10.0f),
        8000.0f,
        "Hz"
    ));

    // loCut - Float (50.0 to 1000.0 Hz) for tape delay feedback filtering
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "loCut", 1 },
        "Lo Cut",
        juce::NormalisableRange<float>(50.0f, 1000.0f, 1.0f),
        100.0f,
        "Hz"
    ));

    // distortionLevel - Float (-60.0 to 0.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "distortionLevel", 1 },
        "Distortion Level",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -6.0f,
        "dB"
    ));

    // masterOutput - Float (-60.0 to 12.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "masterOutput", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // Bypass toggles for troubleshooting
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassSaturation", 1 },
        "Bypass Saturation",
        false
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassDoppler", 1 },
        "Bypass Doppler",
        false
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
    // Prepare DSP spec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;  // Stereo only

    // Prepare stereo delay lines (only need ~1ms for base Doppler delay)
    const int maxDelaySamples = static_cast<int>(sampleRate * 0.01);  // 10ms max
    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);
    delayLineLeft.reset();
    delayLineRight.reset();

    // Pre-allocate feedback buffer (real-time safety)
    const int numChannels = 2;  // Force stereo
    feedbackBuffer.setSize(numChannels, samplesPerBlock);
    feedbackBuffer.clear();

    // Prepare tape delay feedback filters
    hiCutFilter.prepare(spec);
    loCutFilter.prepare(spec);
    hiCutFilter.reset();
    loCutFilter.reset();

    // Initialize doppler control smoothing
    currentDopplerControl = 0.0f;
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Release feedback buffer to save memory when plugin not in use
    feedbackBuffer.setSize(0, 0);
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters (atomic, real-time safe)
    auto* saturationParam = parameters.getRawParameterValue("saturation");
    auto* dopplerShiftParam = parameters.getRawParameterValue("dopplerShift");
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* hiCutParam = parameters.getRawParameterValue("hiCut");
    auto* loCutParam = parameters.getRawParameterValue("loCut");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    // Bypass parameters
    auto* bypassSaturationParam = parameters.getRawParameterValue("bypassSaturation");
    auto* bypassDopplerParam = parameters.getRawParameterValue("bypassDoppler");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    float feedbackPercent = feedbackParam->load();
    float hiCutFreq = hiCutParam->load();
    float loCutFreq = loCutParam->load();
    float masterOutputDb = masterOutputParam->load();

    // Bypass states
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;
    bool bypassDoppler = bypassDopplerParam->load() > 0.5f;

    // Convert parameters to usable values
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float feedbackGain = feedbackPercent / 100.0f;  // 0-95% → 0.0-0.95
    float masterOutputGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // SMOOTH DOPPLER CONTROL: Map from -50% to +50% → -1 to +1
    const float targetDopplerControl = bypassDoppler ? 0.0f : (dopplerShift / 50.0f);
    const float dopplerSmoothingFactor = 0.01f;  // Smooth to avoid clicks
    currentDopplerControl += (targetDopplerControl - currentDopplerControl) * dopplerSmoothingFactor;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Ensure feedback buffer matches current block size
    if (feedbackBuffer.getNumSamples() != numSamples)
    {
        feedbackBuffer.setSize(2, numSamples, false, false, true);
        feedbackBuffer.clear();
    }

    // Update feedback filter coefficients (only when they change significantly)
    *hiCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, hiCutFreq);
    *loCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, loCutFreq);

    // ═══════════════════════════════════════════════════════════════════════════
    // SERIES PROCESSING CHAIN: Input → Doppler Delay → Saturation → Output
    // ═══════════════════════════════════════════════════════════════════════════

    // Psychoacoustic Doppler Delay Constants
    const float ITD_MAX = 0.00052f;  // 520 microseconds (ear separation)
    const float BASE_DELAY_MS = 1.0f;  // 1ms stability offset (d0)

    // Calculate base delay (d0) in samples
    const float d0Samples = (BASE_DELAY_MS / 1000.0f) * static_cast<float>(spec.sampleRate);

    // Calculate differential delay from psychoacoustic ITD
    // deltaDelay(t) = (ITD_MAX / 2) * x(t)   where x(t) ∈ [-1, 1]
    const float deltaDelaySamples = (ITD_MAX / 2.0f) * currentDopplerControl * static_cast<float>(spec.sampleRate);

    // Compute L/R delay times with opposite differential
    // delayLeft(t)  = d0 + deltaDelay(t)
    // delayRight(t) = d0 - deltaDelay(t)
    const float leftDelaySamples = d0Samples + deltaDelaySamples;
    const float rightDelaySamples = d0Samples - deltaDelaySamples;

    // Process each sample through the series chain
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // ─────────────────────────────────────────────────────────────────────
        // STAGE 1: DOPPLER DELAY (with feedback)
        // ─────────────────────────────────────────────────────────────────────

        float leftIn = buffer.getSample(0, sample);
        float rightIn = buffer.getSample(1, sample);

        if (!bypassDoppler)
        {
            // Add filtered feedback to input
            leftIn += feedbackBuffer.getSample(0, sample) * feedbackGain;
            rightIn += feedbackBuffer.getSample(1, sample) * feedbackGain;

            // Apply psychoacoustic doppler delay (fractional delay with linear interpolation)
            delayLineLeft.setDelay(leftDelaySamples);
            delayLineRight.setDelay(rightDelaySamples);

            leftIn = delayLineLeft.popSample(0, leftIn);
            rightIn = delayLineRight.popSample(0, rightIn);

            // Store delayed signal for feedback (will be filtered below)
            feedbackBuffer.setSample(0, sample, leftIn);
            feedbackBuffer.setSample(1, sample, rightIn);
        }

        // ─────────────────────────────────────────────────────────────────────
        // STAGE 2: SATURATION
        // ─────────────────────────────────────────────────────────────────────

        if (!bypassSaturation)
        {
            // Tube saturation: output = tanh(gain * input)
            leftIn = std::tanh(saturationGain * leftIn);
            rightIn = std::tanh(saturationGain * rightIn);
        }

        // ─────────────────────────────────────────────────────────────────────
        // STAGE 3: MASTER OUTPUT
        // ─────────────────────────────────────────────────────────────────────

        buffer.setSample(0, sample, leftIn * masterOutputGain);
        buffer.setSample(1, sample, rightIn * masterOutputGain);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // POST-PROCESSING: Filter feedback buffer for next iteration
    // ─────────────────────────────────────────────────────────────────────────

    if (!bypassDoppler && feedbackGain > 0.001f)
    {
        juce::dsp::AudioBlock<float> feedbackBlock(feedbackBuffer);
        juce::dsp::ProcessContextReplacing<float> feedbackContext(feedbackBlock);

        // Apply lo-cut (high-pass) then hi-cut (low-pass) to feedback signal
        loCutFilter.process(feedbackContext);
        hiCutFilter.process(feedbackContext);
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
