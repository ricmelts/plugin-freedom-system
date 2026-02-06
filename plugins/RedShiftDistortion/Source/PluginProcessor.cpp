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

    // Pre-allocate processing buffers (real-time safety)
    const int numChannels = 2;  // Force stereo
    delayPathBuffer.setSize(numChannels, samplesPerBlock);
    distortionPathBuffer.setSize(numChannels, samplesPerBlock);
    feedbackBuffer.setSize(numChannels, samplesPerBlock);

    // Clear buffers
    delayPathBuffer.clear();
    distortionPathBuffer.clear();
    feedbackBuffer.clear();

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
    auto* distortionLevelParam = parameters.getRawParameterValue("distortionLevel");
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
    float distortionLevelDb = distortionLevelParam->load();
    float masterOutputDb = masterOutputParam->load();

    // Bypass states
    bool bypassDelay = bypassDelayParam->load() > 0.5f;
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;
    bool bypassDoppler = bypassDopplerParam->load() > 0.5f;

    // Convert parameters to usable values
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float feedbackGain = feedbackPercent / 100.0f;  // 0-95% → 0.0-0.95
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
    if (!bypassDelay)
    {
        // STEP 1: Add feedback from previous iteration
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* delayInput = delayPathBuffer.getWritePointer(channel);
            const auto* feedbackData = feedbackBuffer.getReadPointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                delayInput[sample] += feedbackData[sample] * feedbackGain;
            }
        }

        // STEP 2: Calculate base delay time
        const float baseDelaySamples = (currentDelayTimeMs / 1000.0f) * static_cast<float>(spec.sampleRate);

        // STEP 3: Apply doppler effect as L/R distance shift (if not bypassed)
        float leftDelaySamples = baseDelaySamples;
        float rightDelaySamples = baseDelaySamples;

        if (!bypassDoppler && std::abs(dopplerShift) > 0.01f)
        {
            // Doppler shift percentage controls L/R distance offset
            // Physics-based: delay_offset = (distance_shift / speed_of_sound) * sampleRate
            // Simplified: percentage directly controls additional delay in samples

            // Maximum offset: 50ms (at ±50% doppler shift)
            const float maxOffsetMs = 50.0f;
            const float offsetMs = (dopplerShift / 100.0f) * maxOffsetMs;  // -50% to +50% → -50ms to +50ms
            const float offsetSamples = (offsetMs / 1000.0f) * static_cast<float>(spec.sampleRate);

            // Apply asymmetric delay to create stereo width
            // Positive shift: right channel delayed (source appears left)
            // Negative shift: left channel delayed (source appears right)
            if (dopplerShift > 0.0f)
            {
                rightDelaySamples += offsetSamples;  // Right ear delayed
            }
            else
            {
                leftDelaySamples += std::abs(offsetSamples);  // Left ear delayed
            }
        }

        // STEP 4: Process stereo delay lines independently
        delayLineLeft.setDelay(leftDelaySamples);
        delayLineRight.setDelay(rightDelaySamples);

        // Process left channel
        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto* leftData = delayPathBuffer.getWritePointer(0);
            leftData[sample] = delayLineLeft.popSample(0, leftData[sample]);
        }

        // Process right channel
        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto* rightData = delayPathBuffer.getWritePointer(1);
            rightData[sample] = delayLineRight.popSample(0, rightData[sample]);
        }


        // STEP 5: Store delayed output in feedback buffer for next iteration
        for (int channel = 0; channel < numChannels; ++channel)
        {
            feedbackBuffer.copyFrom(channel, 0, delayPathBuffer, channel, 0, numSamples);
        }
    }
    // If delay is bypassed, delayPathBuffer remains as clean input copy

    // DISTORTION PATH PROCESSING
    if (!bypassSaturation)
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
    // If saturation is bypassed, distortionPathBuffer remains as clean input copy

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
