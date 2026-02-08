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

    // dopplerShift - Float (-50.0 to 50.0 %) - TODO: Not yet implemented
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "dopplerShift", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // pitchEnable - Bool (On/Off) - TODO: Not yet implemented
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "pitchEnable", 1 },
        "Pitch Enable",
        true
    ));

    // delayTime - Float (0.0 to 2000.0 ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delayTime", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(0.0f, 2000.0f, 1.0f),
        250.0f,
        "ms"
    ));

    // tempoSync - Bool (On/Off)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "tempoSync", 1 },
        "Tempo Sync",
        false
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
    spec.numChannels = 2;  // Stereo only

    // Prepare main delay line - max 2 seconds
    const int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    mainDelayLine.setMaximumDelayInSamples(maxDelaySamples);
    mainDelayLine.prepare(spec);
    mainDelayLine.reset();
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Nothing to release (delay line managed by JUCE)
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
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    bool pitchEnabled = pitchEnableParam->load() > 0.5f;
    float delayTimeMs = delayTimeParam->load();
    bool tempoSyncEnabled = tempoSyncParam->load() > 0.5f;
    float masterOutputDb = masterOutputParam->load();

    // Convert parameters to usable values
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float masterOutputGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // Calculate delay time (tempo-synced or free)
    float actualDelayTimeMs = getTempoSyncedDelayTime(delayTimeMs, tempoSyncEnabled);
    float actualDelayTimeSamples = (actualDelayTimeMs / 1000.0f) * static_cast<float>(spec.sampleRate);
    actualDelayTimeSamples = juce::jlimit(0.0f, static_cast<float>(spec.sampleRate * 2.0), actualDelayTimeSamples);
    mainDelayLine.setDelay(actualDelayTimeSamples);

    const int numSamples = buffer.getNumSamples();

    // ═══════════════════════════════════════════════════════════════════════════
    // SERIES PROCESSING: Delay → (Doppler TODO) → Saturation → Master Output
    // ═══════════════════════════════════════════════════════════════════════════

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leftIn = buffer.getSample(0, sample);
        float rightIn = buffer.getSample(1, sample);

        // STAGE 1: Delay Line
        mainDelayLine.pushSample(0, leftIn);
        mainDelayLine.pushSample(1, rightIn);

        float leftOut = mainDelayLine.popSample(0);
        float rightOut = mainDelayLine.popSample(1);

        // STAGE 2: Doppler Pitch/Time Stretch (TODO - currently pass-through)
        // Future: Implement granular synthesis or pitch-shift based on dopplerShift and pitchEnabled
        juce::ignoreUnused(dopplerShift, pitchEnabled);

        // STAGE 3: Tube Saturation
        leftOut = std::tanh(saturationGain * leftOut);
        rightOut = std::tanh(saturationGain * rightOut);

        // STAGE 4: Master Output
        buffer.setSample(0, sample, leftOut * masterOutputGain);
        buffer.setSample(1, sample, rightOut * masterOutputGain);
    }
}

float RedShiftDistortionAudioProcessor::getTempoSyncedDelayTime(float delayTimeParam, bool tempoSyncEnabled)
{
    if (!tempoSyncEnabled)
        return delayTimeParam; // Free time mode - return milliseconds directly

    // Get host BPM
    auto playHead = getPlayHead();
    if (playHead == nullptr)
        return delayTimeParam;

    auto position = playHead->getPosition();
    if (!position.hasValue())
        return delayTimeParam;

    auto bpm = position->getBpm();
    if (!bpm.hasValue())
        bpm = 120.0; // Default BPM

    // Clamp BPM to reasonable range
    double currentBpm = juce::jlimit(20.0, 300.0, *bpm);

    // Convert to quarter note duration
    double quarterNoteDuration = (60000.0 / currentBpm); // 1/4 note in ms

    // Map parameter ranges to musical divisions
    // 0-250ms → 1/16 note (0.25 beats)
    // 250-500ms → 1/8 note (0.5 beats)
    // 500-1000ms → 1/4 note (1.0 beats)
    // 1000-1500ms → 1/2 note (2.0 beats)
    // 1500-2000ms → 1 bar (4.0 beats)
    if (delayTimeParam < 250.0f)
        return static_cast<float>(quarterNoteDuration * 0.25); // 1/16 note
    else if (delayTimeParam < 500.0f)
        return static_cast<float>(quarterNoteDuration * 0.5); // 1/8 note
    else if (delayTimeParam < 1000.0f)
        return static_cast<float>(quarterNoteDuration); // 1/4 note
    else if (delayTimeParam < 1500.0f)
        return static_cast<float>(quarterNoteDuration * 2.0); // 1/2 note
    else
        return static_cast<float>(quarterNoteDuration * 4.0); // 1 bar
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
