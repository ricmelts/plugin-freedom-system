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
    grainOutputBuffer.setSize(numChannels, samplesPerBlock);
    feedbackBuffer.setSize(numChannels, samplesPerBlock);

    // Clear buffers
    delayPathBuffer.clear();
    distortionPathBuffer.clear();
    grainOutputBuffer.clear();
    feedbackBuffer.clear();

    // Reset LFO phase
    lfoPhase = 0.0f;

    // Initialize delay time smoothing
    currentDelayTimeMs = 250.0f;
    targetDelayTimeMs = 250.0f;

    // Initialize granular synthesis engine
    initializeGranularEngine();
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

void RedShiftDistortionAudioProcessor::initializeGranularEngine()
{
    // Calculate grain size in samples (100ms at current sample rate)
    grainSizeSamples = static_cast<int>((GRAIN_SIZE_MS / 1000.0) * spec.sampleRate);

    // Grain advance is 25% of grain size (4 overlapping grains = 75% overlap)
    grainAdvanceSamples = grainSizeSamples / 4;

    // Allocate grain buffer (needs to hold at least 2x grain size for overlap)
    grainBufferSize = grainSizeSamples * 2;
    grainBuffer.setSize(static_cast<int>(spec.numChannels), grainBufferSize);
    grainBuffer.clear();
    grainBufferWritePos = 0;

    // Pre-calculate Hann window (smooth grain envelope)
    hannWindow.resize(grainSizeSamples);
    for (int i = 0; i < grainSizeSamples; ++i)
    {
        // Hann window formula: 0.5 * (1.0 - cos(2π * i / grainSize))
        float phase = static_cast<float>(i) / static_cast<float>(grainSizeSamples);
        hannWindow[i] = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * phase));
    }

    // Reset grain states
    for (auto& grain : grains)
    {
        grain.readPosition = 0;
        grain.playbackRate = 1.0f;
        grain.grainPhase = 0.0f;
        grain.isActive = false;
    }

    samplesSinceLastGrainSpawn = 0;
}

void RedShiftDistortionAudioProcessor::spawnGrain(float pitchShiftRate)
{
    // Find an inactive grain slot
    for (auto& grain : grains)
    {
        if (!grain.isActive)
        {
            // Activate grain at current write position
            grain.readPosition = grainBufferWritePos;
            grain.playbackRate = pitchShiftRate;
            grain.grainPhase = 0.0f;
            grain.isActive = true;
            return;
        }
    }

    // If all grains active, replace oldest grain (grain 0)
    // This shouldn't happen with proper 4-grain overlap, but provides safety
    grains[0].readPosition = grainBufferWritePos;
    grains[0].playbackRate = pitchShiftRate;
    grains[0].grainPhase = 0.0f;
    grains[0].isActive = true;
}

float RedShiftDistortionAudioProcessor::getHannWindowValue(float phase)
{
    // Clamp phase to 0.0-1.0
    phase = juce::jlimit(0.0f, 1.0f, phase);

    // Convert phase to window index
    int index = static_cast<int>(phase * static_cast<float>(grainSizeSamples - 1));
    return hannWindow[index];
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
    auto* pitchEnableParam = parameters.getRawParameterValue("pitchEnable");
    auto* delayTimeParam = parameters.getRawParameterValue("delayTime");
    auto* tempoSyncParam = parameters.getRawParameterValue("tempoSync");
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* distortionLevelParam = parameters.getRawParameterValue("distortionLevel");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    bool pitchEnable = pitchEnableParam->load() > 0.5f;
    float delayTimeMs = delayTimeParam->load();
    bool tempoSync = tempoSyncParam->load() > 0.5f;
    float feedbackPercent = feedbackParam->load();
    float distortionLevelDb = distortionLevelParam->load();
    float masterOutputDb = masterOutputParam->load();

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

        // STEP 2: Calculate delay time (with optional modulation for doppler)
        // Only modulate if doppler is non-zero
        float finalDelayTimeMs = currentDelayTimeMs;

        if (std::abs(dopplerShift) > 0.01f)  // Doppler enabled
        {
            // LFO frequency scales with doppler shift magnitude
            const float lfoFreq = 0.5f + (std::abs(dopplerShift) / 100.0f) * 1.5f;

            // Modulation depth scales with doppler shift magnitude (0-10% delay variation)
            const float modDepth = (std::abs(dopplerShift) / 100.0f) * 0.1f;

            // Calculate LFO phase increment per sample
            const float phaseInc = (lfoFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(spec.sampleRate);

            // LFO output: sine wave (-1.0 to +1.0)
            const float lfoOut = std::sin(lfoPhase);

            // Modulate delay time
            finalDelayTimeMs = currentDelayTimeMs * (1.0f + lfoOut * modDepth);

            // Advance LFO phase
            lfoPhase += phaseInc * static_cast<float>(numSamples);
            if (lfoPhase >= juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;
        }

        // STEP 3: Set delay time and process delay line
        const float delaySamples = (finalDelayTimeMs / 1000.0f) * static_cast<float>(spec.sampleRate);
        delayLine.setDelay(delaySamples);

        juce::dsp::AudioBlock<float> delayBlock(delayPathBuffer);
        juce::dsp::ProcessContextReplacing<float> delayContext(delayBlock);
        delayLine.process(delayContext);

        // STEP 4: Optional pitch shifting (only if doppler is significant AND pitchEnable is true)
        // Doppler creates natural pitch artifacts from delay time modulation
        // Additional pitch shifting is optional for extreme effects
        bool applyPitchShift = pitchEnable && (std::abs(dopplerShift) > 10.0f);  // Only above 10%

        if (applyPitchShift)
        {
            // Calculate pitch shift rate
            const float semitones = (dopplerShift / 100.0f) * 12.0f;  // ±50% → ±12 semitones
            const float pitchShiftRate = std::pow(2.0f, semitones / 12.0f);

            // Clear grain output buffer
            grainOutputBuffer.clear();

            // Process granular pitch shifting
            for (int sample = 0; sample < numSamples; ++sample)
            {
                // Write delayed signal to grain buffer
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    const auto* delayData = delayPathBuffer.getReadPointer(channel);
                    auto* grainBufData = grainBuffer.getWritePointer(channel);
                    grainBufData[grainBufferWritePos] = delayData[sample];
                }

                grainBufferWritePos = (grainBufferWritePos + 1) % grainBufferSize;

                // Spawn grains
                samplesSinceLastGrainSpawn++;
                if (samplesSinceLastGrainSpawn >= grainAdvanceSamples)
                {
                    spawnGrain(pitchShiftRate);
                    samplesSinceLastGrainSpawn = 0;
                }

                // Process active grains
                for (auto& grain : grains)
                {
                    if (!grain.isActive || grain.grainPhase >= 1.0f)
                    {
                        grain.isActive = false;
                        continue;
                    }

                    const int readPos = static_cast<int>(grain.readPosition) % grainBufferSize;
                    const float windowValue = getHannWindowValue(grain.grainPhase);

                    for (int channel = 0; channel < numChannels; ++channel)
                    {
                        const auto* grainBufData = grainBuffer.getReadPointer(channel);
                        auto* outputData = grainOutputBuffer.getWritePointer(channel);
                        outputData[sample] += grainBufData[readPos] * windowValue;
                    }

                    grain.readPosition += grain.playbackRate;
                    grain.grainPhase += 1.0f / static_cast<float>(grainSizeSamples);
                }
            }

            // Blend pitched output with delayed signal (50/50 mix for smoother result)
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* delayData = delayPathBuffer.getWritePointer(channel);
                const auto* grainData = grainOutputBuffer.getReadPointer(channel);

                for (int sample = 0; sample < numSamples; ++sample)
                {
                    delayData[sample] = delayData[sample] * 0.5f + grainData[sample] * 0.5f;
                }
            }
        }

        // STEP 5: Store delayed output in feedback buffer for next iteration
        for (int channel = 0; channel < numChannels; ++channel)
        {
            feedbackBuffer.copyFrom(channel, 0, delayPathBuffer, channel, 0, numSamples);
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
