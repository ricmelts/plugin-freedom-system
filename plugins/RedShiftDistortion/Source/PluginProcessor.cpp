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

    // delayTime - Float (0.0 to 16000.0 ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(0.0f, 16000.0f, 1.0f),
        250.0f,
        "ms"
    ));

    // tempoSync - Bool (default: true)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "tempoSync", 1 },
        "Tempo Sync",
        true
    ));

    // delayLevel - Float (-60.0 to 0.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delayLevel", 1 },
        "Delay Level",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -6.0f,
        "dB"
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
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Prepare delay line (maximum 20 seconds at 192kHz = 3,840,000 samples)
    const int maxDelaySamples = static_cast<int>(sampleRate * 20.0);
    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(spec);
    delayLine.reset();

    // Pre-allocate parallel path buffers (real-time safety)
    const int numChannels = getTotalNumOutputChannels();
    delayPathBuffer.setSize(numChannels, samplesPerBlock);
    distortionPathBuffer.setSize(numChannels, samplesPerBlock);

    // Clear buffers
    delayPathBuffer.clear();
    distortionPathBuffer.clear();

    // Reset LFO phase
    lfoPhase = 0.0f;

    // Initialize delay time smoothing
    currentDelayTimeMs = 250.0f;
    targetDelayTimeMs = 250.0f;
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Release large buffers to save memory when plugin not in use
    delayPathBuffer.setSize(0, 0);
    distortionPathBuffer.setSize(0, 0);
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
                return juce::jlimit(20.0, 300.0, *bpm);
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
    auto* delayLevelParam = parameters.getRawParameterValue("delayLevel");
    auto* distortionLevelParam = parameters.getRawParameterValue("distortionLevel");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    float delayTimeMs = delayTimeParam->load();
    bool tempoSync = tempoSyncParam->load() > 0.5f;
    float delayLevelDb = delayLevelParam->load();
    float distortionLevelDb = distortionLevelParam->load();
    float masterOutputDb = masterOutputParam->load();

    // Convert dB to linear gain
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float delayLevelGain = std::pow(10.0f, delayLevelDb / 20.0f);
    float distortionLevelGain = std::pow(10.0f, distortionLevelDb / 20.0f);
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
    // Exponential smoothing: fast enough to track tempo changes, slow enough to avoid clicks
    const float smoothingFactor = 0.05f;  // 5% per buffer (~10ms smoothing at 512 samples @ 48kHz)
    currentDelayTimeMs += (targetDelayTimeMs - currentDelayTimeMs) * smoothingFactor;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Ensure parallel path buffers are sized correctly
    if (delayPathBuffer.getNumSamples() < numSamples)
    {
        delayPathBuffer.setSize(numChannels, numSamples, false, false, true);
        distortionPathBuffer.setSize(numChannels, numSamples, false, false, true);
    }

    // PARALLEL ROUTING: Duplicate input to both paths
    for (int channel = 0; channel < numChannels; ++channel)
    {
        delayPathBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
        distortionPathBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
    }

    // DELAY PATH PROCESSING
    {
        // DELAY TIME MODULATOR: LFO-driven delay time variation (doppler simulation)
        // LFO frequency scales with doppler shift magnitude (0.5Hz to 2.0Hz)
        const float lfoFreq = 0.5f + (std::abs(dopplerShift) / 100.0f) * 1.5f;  // 0.5-2.0Hz range

        // Modulation depth scales with doppler shift magnitude (0-10% delay variation)
        const float modDepth = (std::abs(dopplerShift) / 100.0f) * 0.1f;  // 0-10% depth

        // Calculate LFO phase increment per sample
        const float phaseInc = (lfoFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(spec.sampleRate);

        // LFO output: sine wave (-1.0 to +1.0)
        const float lfoOut = std::sin(lfoPhase);

        // Modulated delay time: base delay * (1.0 + lfo * depth)
        // Example: 250ms base, 10% depth, lfo=+1.0 → 250ms * 1.1 = 275ms
        const float modulatedDelayTimeMs = currentDelayTimeMs * (1.0f + lfoOut * modDepth);

        // Convert delay time from ms to samples
        const float delaySamples = (modulatedDelayTimeMs / 1000.0f) * static_cast<float>(spec.sampleRate);
        delayLine.setDelay(delaySamples);

        // Advance LFO phase
        lfoPhase += phaseInc * static_cast<float>(numSamples);

        // Wrap phase to prevent overflow (explicit wrapping at 2π)
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        // Process delay line
        juce::dsp::AudioBlock<float> delayBlock(delayPathBuffer);
        juce::dsp::ProcessContextReplacing<float> delayContext(delayBlock);
        delayLine.process(delayContext);

        // Apply delay level gain
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* delayData = delayPathBuffer.getWritePointer(channel);
            juce::FloatVectorOperations::multiply(delayData, delayLevelGain, numSamples);
        }
    }

    // DISTORTION PATH PROCESSING
    {
        // Apply tube saturation (tanh waveshaping)
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* distData = distortionPathBuffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                // Tube saturation: output = tanh(gain * input)
                distData[sample] = std::tanh(saturationGain * distData[sample]);
            }
        }

        // Apply distortion level gain
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* distData = distortionPathBuffer.getWritePointer(channel);
            juce::FloatVectorOperations::multiply(distData, distortionLevelGain, numSamples);
        }
    }

    // OUTPUT MIXER: Sum both paths + apply master gain
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* outputData = buffer.getWritePointer(channel);
        const auto* delayData = delayPathBuffer.getReadPointer(channel);
        const auto* distData = distortionPathBuffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Parallel mix: sum both paths
            outputData[sample] = (delayData[sample] + distData[sample]) * masterOutputGain;
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
