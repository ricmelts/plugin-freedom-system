#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class RedShiftDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor&);
    ~RedShiftDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RedShiftDistortionAudioProcessor& processorRef;

    // ⚠️ MEMBER DECLARATION ORDER IS CRITICAL ⚠️
    // Members destroyed in REVERSE order of declaration
    // Declare dependencies AFTER what they depend on

    // 1️⃣ RELAYS FIRST (no dependencies)
    std::unique_ptr<juce::WebSliderRelay> saturationRelay;
    std::unique_ptr<juce::WebSliderRelay> dopplerShiftRelay;
    std::unique_ptr<juce::WebSliderRelay> feedbackRelay;
    std::unique_ptr<juce::WebSliderRelay> hiCutRelay;
    std::unique_ptr<juce::WebSliderRelay> loCutRelay;
    std::unique_ptr<juce::WebSliderRelay> masterOutputRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassSaturationRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDopplerRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relays via withOptionsFrom)
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3️⃣ ATTACHMENTS LAST (depend on both relays and webView)
    std::unique_ptr<juce::WebSliderParameterAttachment> saturationAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> dopplerShiftAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> feedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> hiCutAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> loCutAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> masterOutputAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassSaturationAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDopplerAttachment;

    // Helper for resource serving
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
