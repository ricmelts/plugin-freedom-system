#pragma once
#include "PluginProcessor.h"

class FlutterVerbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit FlutterVerbAudioProcessorEditor(FlutterVerbAudioProcessor&);
    ~FlutterVerbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FlutterVerbAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlutterVerbAudioProcessorEditor)
};
