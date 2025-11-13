#pragma once
#include "PluginProcessor.h"

class MinimalKickAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MinimalKickAudioProcessorEditor(MinimalKickAudioProcessor&);
    ~MinimalKickAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MinimalKickAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimalKickAudioProcessorEditor)
};
