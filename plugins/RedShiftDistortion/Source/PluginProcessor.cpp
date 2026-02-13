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
    // Initialization will be added in Stage 2 (DSP)
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (DSP)
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access example (for Stage 2 DSP implementation):
    // auto* stereoWidthParam = parameters.getRawParameterValue("stereoWidth");
    // float stereoWidthValue = stereoWidthParam->load();  // Atomic read (real-time safe)

    // Pass-through for Stage 1 (DSP implementation happens in Stage 2)
    // Audio routing is already handled by JUCE (input → output)
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
