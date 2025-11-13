#include "PluginProcessor.h"
#include "PluginEditor.h"

MinimalKickAudioProcessor::MinimalKickAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

MinimalKickAudioProcessor::~MinimalKickAudioProcessor()
{
}

void MinimalKickAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialization will be added in Stage 4
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void MinimalKickAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 4
}

void MinimalKickAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Pass-through for Stage 2 (DSP added in Stage 4)
    // Instrument outputs silence until DSP implemented
    buffer.clear();
}

juce::AudioProcessorEditor* MinimalKickAudioProcessor::createEditor()
{
    return new MinimalKickAudioProcessorEditor(*this);
}

void MinimalKickAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // State management will be added in Stage 3
    juce::ignoreUnused(destData);
}

void MinimalKickAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // State management will be added in Stage 3
    juce::ignoreUnused(data, sizeInBytes);
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MinimalKickAudioProcessor();
}
