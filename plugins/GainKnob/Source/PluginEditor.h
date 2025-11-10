#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class GainKnobAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GainKnobAudioProcessorEditor(GainKnobAudioProcessor&);
    ~GainKnobAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GainKnobAudioProcessor& processorRef;

    // ⚠️ MEMBER DECLARATION ORDER IS CRITICAL ⚠️
    // Members are destroyed in REVERSE order of declaration
    // Declare dependencies AFTER what they depend on

    // 1️⃣ RELAY FIRST (no dependencies)
    juce::WebSliderRelay gainRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relay via withOptionsFrom)
    juce::WebBrowserComponent webView;

    // 3️⃣ ATTACHMENT LAST (depends on both relay and webView)
    juce::WebSliderParameterAttachment gainAttachment;

    // Helper for resource serving
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnobAudioProcessorEditor)
};
