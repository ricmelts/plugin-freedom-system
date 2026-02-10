#pragma once
#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

class RedShiftDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor&);
    ~RedShiftDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RedShiftDistortionAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
