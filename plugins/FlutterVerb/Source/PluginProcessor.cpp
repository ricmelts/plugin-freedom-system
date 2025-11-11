#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout FlutterVerbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // SIZE - Room dimensions
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SIZE", 1 },
        "Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    // DECAY - Reverb tail length
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DECAY", 1 },
        "Decay",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 1.0f),
        2.5f,
        "s"
    ));

    // MIX - Dry/wet blend
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "MIX", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        25.0f,
        "%"
    ));

    // AGE - Tape character intensity
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "AGE", 1 },
        "Age",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        20.0f,
        "%"
    ));

    // DRIVE - Tape saturation
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DRIVE", 1 },
        "Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        20.0f,
        "%"
    ));

    // TONE - DJ-style filter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "TONE", 1 },
        "Tone",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f, 1.0f),
        0.0f,
        ""
    ));

    // MOD_MODE - Modulation routing
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "MOD_MODE", 1 },
        "Mod Mode",
        false  // Default: WET ONLY (0)
    ));

    return layout;
}

FlutterVerbAudioProcessor::FlutterVerbAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

FlutterVerbAudioProcessor::~FlutterVerbAudioProcessor()
{
}

void FlutterVerbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // DSP initialization will be added in Stage 4
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void FlutterVerbAudioProcessor::releaseResources()
{
    // DSP cleanup will be added in Stage 4
}

void FlutterVerbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access example (for Stage 4 DSP implementation):
    // auto* sizeParam = parameters.getRawParameterValue("SIZE");
    // auto* decayParam = parameters.getRawParameterValue("DECAY");
    // auto* mixParam = parameters.getRawParameterValue("MIX");
    // auto* ageParam = parameters.getRawParameterValue("AGE");
    // auto* driveParam = parameters.getRawParameterValue("DRIVE");
    // auto* toneParam = parameters.getRawParameterValue("TONE");
    // auto* modModeParam = parameters.getRawParameterValue("MOD_MODE");
    //
    // float size = sizeParam->load();      // Atomic read (real-time safe)
    // float decay = decayParam->load();
    // float mix = mixParam->load();
    // float age = ageParam->load();
    // float drive = driveParam->load();
    // float tone = toneParam->load();
    // bool modMode = modModeParam->load() > 0.5f;

    // Pass-through for Stage 3 (DSP implementation happens in Stage 4)
    // Audio routing is already handled by JUCE bus configuration
}

juce::AudioProcessorEditor* FlutterVerbAudioProcessor::createEditor()
{
    return new FlutterVerbAudioProcessorEditor(*this);
}

void FlutterVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FlutterVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlutterVerbAudioProcessor();
}
