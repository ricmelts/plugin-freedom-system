#include "PluginProcessor.h"
#include "PluginEditor.h"

OrganicHatsAudioProcessor::OrganicHatsAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OrganicHatsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Closed Hi-Hat parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "CLOSED_TONE", 1 },
        "Closed Tone",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
        50.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "CLOSED_DECAY", 1 },
        "Closed Decay",
        juce::NormalisableRange<float>(20.0f, 200.0f, 0.1f),
        80.0f,
        "ms"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "CLOSED_NOISE_COLOR", 1 },
        "Closed Noise Color",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
        50.0f,
        "%"
    ));

    // Open Hi-Hat parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "OPEN_TONE", 1 },
        "Open Tone",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
        50.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "OPEN_RELEASE", 1 },
        "Open Release",
        juce::NormalisableRange<float>(100.0f, 1000.0f, 0.1f),
        400.0f,
        "ms"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "OPEN_NOISE_COLOR", 1 },
        "Open Noise Color",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f),
        50.0f,
        "%"
    ));

    return layout;
}

OrganicHatsAudioProcessor::~OrganicHatsAudioProcessor()
{
}

void OrganicHatsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialization will be added in Stage 4
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void OrganicHatsAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 4
}

void OrganicHatsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access example (for Stage 4 DSP implementation):
    // auto* closedToneParam = parameters.getRawParameterValue("CLOSED_TONE");
    // float closedTone = closedToneParam->load();  // Atomic read (real-time safe)
    //
    // All 6 parameters available:
    // - CLOSED_TONE, CLOSED_DECAY, CLOSED_NOISE_COLOR
    // - OPEN_TONE, OPEN_RELEASE, OPEN_NOISE_COLOR

    // Clear output buffer (silence for Stage 3 - DSP added in Stage 4)
    buffer.clear();
}

juce::AudioProcessorEditor* OrganicHatsAudioProcessor::createEditor()
{
    return new OrganicHatsAudioProcessorEditor(*this);
}

void OrganicHatsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OrganicHatsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OrganicHatsAudioProcessor();
}
