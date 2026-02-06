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
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Release large buffers to save memory when plugin not in use
    delayPathBuffer.setSize(0, 0);
    distortionPathBuffer.setSize(0, 0);
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
    auto* delayTimeParam = parameters.getRawParameterValue("delayTime");
    auto* delayLevelParam = parameters.getRawParameterValue("delayLevel");
    auto* distortionLevelParam = parameters.getRawParameterValue("distortionLevel");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    float saturationDb = saturationParam->load();
    float delayTimeMs = delayTimeParam->load();
    float delayLevelDb = delayLevelParam->load();
    float distortionLevelDb = distortionLevelParam->load();
    float masterOutputDb = masterOutputParam->load();

    // Convert dB to linear gain
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float delayLevelGain = std::pow(10.0f, delayLevelDb / 20.0f);
    float distortionLevelGain = std::pow(10.0f, distortionLevelDb / 20.0f);
    float masterOutputGain = std::pow(10.0f, masterOutputDb / 20.0f);

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
        // Convert delay time from ms to samples
        const float delaySamples = (delayTimeMs / 1000.0f) * static_cast<float>(spec.sampleRate);
        delayLine.setDelay(delaySamples);

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
