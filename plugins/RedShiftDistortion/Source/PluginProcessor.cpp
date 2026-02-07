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

    // pitchEnable - Bool (default: true)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "pitchEnable", 1 },
        "Pitch Enable",
        true
    ));

    // delayTime - Float (0.0 to 2500.0 ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(0.0f, 2500.0f, 1.0f),
        250.0f,
        "ms"
    ));

    // tempoSync - Bool (default: true)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "tempoSync", 1 },
        "Tempo Sync",
        true
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
        juce::ParameterID { "bypassDelay", 1 },
        "Bypass Delay",
        false
    ));

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

    // Prepare stereo delay lines (maximum 3 seconds at 192kHz)
    const int maxDelaySamples = static_cast<int>(sampleRate * 3.0);
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

    // Initialize delay time smoothing
    currentDelayTimeMs = 250.0f;
    targetDelayTimeMs = 250.0f;

    // Initialize doppler control smoothing
    currentDopplerControl = 0.0f;
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Release feedback buffer to save memory when plugin not in use
    feedbackBuffer.setSize(0, 0);
}

float RedShiftDistortionAudioProcessor::getHostBpm()
{
    // Query host BPM via AudioPlayHead (real-time safe)
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (auto bpm = positionInfo->getBpm())
            {
                // Clamp BPM to valid range (20-300 BPM)
                return static_cast<float>(juce::jlimit(20.0, 300.0, *bpm));
            }
        }
    }

    // Fallback to 120 BPM if host doesn't provide tempo
    return 120.0;
}

float RedShiftDistortionAudioProcessor::quantizeDelayTimeToTempo(float hostBpm, float delayTimeMs)
{
    // Note divisions in beats (4/4 time signature)
    const float divisions[] = {
        0.25f,  // 1/16 note
        0.5f,   // 1/8 note
        1.0f,   // 1/4 note
        2.0f,   // 1/2 note
        4.0f,   // 1 bar
        8.0f,   // 2 bars
        16.0f,  // 4 bars
        32.0f   // 8 bars
    };

    // Convert current delayTimeMs to beats
    const float beatsFromMs = (delayTimeMs * hostBpm) / 60000.0f;

    // Find nearest note division
    float nearestDivision = divisions[0];
    float minDistance = std::abs(beatsFromMs - nearestDivision);

    for (float division : divisions)
    {
        float distance = std::abs(beatsFromMs - division);
        if (distance < minDistance)
        {
            minDistance = distance;
            nearestDivision = division;
        }
    }

    // Convert nearest division back to milliseconds
    // Formula: ms = (60000 / bpm) * beats
    return (60000.0f / hostBpm) * nearestDivision;
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
    auto* delayTimeParam = parameters.getRawParameterValue("delayTime");
    auto* tempoSyncParam = parameters.getRawParameterValue("tempoSync");
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* hiCutParam = parameters.getRawParameterValue("hiCut");
    auto* loCutParam = parameters.getRawParameterValue("loCut");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    // Bypass parameters
    auto* bypassDelayParam = parameters.getRawParameterValue("bypassDelay");
    auto* bypassSaturationParam = parameters.getRawParameterValue("bypassSaturation");
    auto* bypassDopplerParam = parameters.getRawParameterValue("bypassDoppler");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    float delayTimeMs = delayTimeParam->load();
    bool tempoSync = tempoSyncParam->load() > 0.5f;
    float feedbackPercent = feedbackParam->load();
    float hiCutFreq = hiCutParam->load();
    float loCutFreq = loCutParam->load();
    float masterOutputDb = masterOutputParam->load();

    // Bypass states
    bool bypassDelay = bypassDelayParam->load() > 0.5f;
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;
    bool bypassDoppler = bypassDopplerParam->load() > 0.5f;

    // Convert parameters to usable values
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float feedbackGain = feedbackPercent / 100.0f;  // 0-95% → 0.0-0.95
    float masterOutputGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // TEMPO SYNC: Calculate target delay time
    if (tempoSync)
    {
        // Get host BPM and quantize delay time to nearest note division
        float hostBpm = getHostBpm();
        targetDelayTimeMs = quantizeDelayTimeToTempo(hostBpm, delayTimeMs);
    }
    else
    {
        // Free time mode: use parameter value directly
        targetDelayTimeMs = delayTimeMs;
    }

    // SMOOTH DELAY TIME TRANSITIONS: Prevent clicks when switching modes or tempo changes
    const float smoothingFactor = 0.05f;  // 5% per buffer
    currentDelayTimeMs += (targetDelayTimeMs - currentDelayTimeMs) * smoothingFactor;

    // SMOOTH DOPPLER CONTROL: Map from -50% to +50% → -1 to +1
    const float targetDopplerControl = bypassDoppler ? 0.0f : (dopplerShift / 50.0f);
    const float dopplerSmoothingFactor = 0.01f;  // Slower smoothing for doppler (avoid artifacts)
    currentDopplerControl += (targetDopplerControl - currentDopplerControl) * dopplerSmoothingFactor;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Update feedback filter coefficients (only when they change significantly)
    *hiCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, hiCutFreq);
    *loCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, loCutFreq);

    // ═══════════════════════════════════════════════════════════════════════════
    // SERIES PROCESSING CHAIN: Input → Doppler Delay → Saturation → Output
    // ═══════════════════════════════════════════════════════════════════════════

    // Psychoacoustic Doppler Delay Constants
    const float ITD_MAX = 0.00052f;  // 520 microseconds (ear separation)
    const float BASE_DELAY_MS = 1.0f;  // 1ms stability offset

    // Calculate base delay (d0) in samples
    const float d0Samples = ((currentDelayTimeMs + BASE_DELAY_MS) / 1000.0f) * static_cast<float>(spec.sampleRate);

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

        if (!bypassDelay)
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

    if (!bypassDelay && feedbackGain > 0.001f)
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
