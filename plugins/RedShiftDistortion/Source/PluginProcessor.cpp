#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter layout creation (BEFORE constructor)
juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // MAIN CONTROLS (7 FloatParameters)

    // 1. STEREO_WIDTH - Stereo Width (-100% to +100%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "STEREO_WIDTH", 1 },
        "Stereo Width",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 2. FEEDBACK - Feedback (0% to 95%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FEEDBACK", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 3. FILTER_BAND_LOW - Lo-Cut (20Hz to 20000Hz, default: 100Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_LOW", 1 },
        "Lo-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        100.0f,
        "Hz"
    ));

    // 4. FILTER_BAND_HIGH - Hi-Cut (20Hz to 20000Hz, default: 8000Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_HIGH", 1 },
        "Hi-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        8000.0f,
        "Hz"
    ));

    // 5. DOPPLER_SHIFT - Doppler Shift (-50% to +50%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DOPPLER_SHIFT", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 6. SATURATION - Saturation (-12dB to +24dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SATURATION", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // 7. MASTER_OUTPUT - Master Output (-60dB to +12dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "MASTER_OUTPUT", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // BYPASS CONTROLS (4 BoolParameters)

    // 8. BYPASS_STEREO_WIDTH - Bypass Stereo Width (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_STEREO_WIDTH", 1 },
        "Bypass Stereo Width",
        false
    ));

    // 9. BYPASS_DELAY - Bypass Delay (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_DELAY", 1 },
        "Bypass Delay",
        false
    ));

    // 10. BYPASS_DOPPLER - Bypass Doppler (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_DOPPLER", 1 },
        "Bypass Doppler",
        false
    ));

    // 11. BYPASS_SATURATION - Bypass Saturation (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_SATURATION", 1 },
        "Bypass Saturation",
        false
    ));

    // ADVANCED SETTINGS (2 parameters)

    // 12. GRAIN_SIZE - Grain Size (25ms to 200ms, default: 100ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "GRAIN_SIZE", 1 },
        "Grain Size",
        juce::NormalisableRange<float>(25.0f, 200.0f, 1.0f),
        100.0f,
        "ms"
    ));

    // 13. GRAIN_OVERLAP - Grain Overlap (2x or 4x, default: 4x = index 1)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "GRAIN_OVERLAP", 1 },
        "Grain Overlap",
        juce::StringArray { "2x", "4x" },
        1  // Default: 4x (index 1)
    ));

    return layout;
}

RedShiftDistortionAudioProcessor::RedShiftDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)   // Effect: stereo input
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)) // Effect: stereo output
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

RedShiftDistortionAudioProcessor::~RedShiftDistortionAudioProcessor()
{
}

void RedShiftDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // DSP initialization will be added in Stage 2 (Phase 2.1+)
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (Phase 2.1+)
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access example (for future Stage 2 DSP implementation):
    // auto* stereoWidthParam = parameters.getRawParameterValue("STEREO_WIDTH");
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
