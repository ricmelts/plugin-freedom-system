#include "PluginProcessor.h"
#include "PluginEditor.h"

FlutterVerbAudioProcessor::FlutterVerbAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
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

    // Pass-through for Stage 2 (DSP added in Stage 4)
    // Audio routing is already handled by JUCE bus configuration
}

juce::AudioProcessorEditor* FlutterVerbAudioProcessor::createEditor()
{
    return new FlutterVerbAudioProcessorEditor(*this);
}

void FlutterVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // State management will be added in Stage 3
    juce::ignoreUnused(destData);
}

void FlutterVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // State management will be added in Stage 3
    juce::ignoreUnused(data, sizeInBytes);
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlutterVerbAudioProcessor();
}
