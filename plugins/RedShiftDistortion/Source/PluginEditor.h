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
    std::unique_ptr<juce::WebSliderRelay> filterBandHighRelay;
    std::unique_ptr<juce::WebSliderRelay> filterBandLowRelay;
    std::unique_ptr<juce::WebSliderRelay> lfoRateRelay;
    std::unique_ptr<juce::WebSliderRelay> lfoDepthRelay;
    std::unique_ptr<juce::WebSliderRelay> distortionLevelRelay;
    std::unique_ptr<juce::WebSliderRelay> masterOutputRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> reverseRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassSaturationRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDopplerRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> lfoTempoSyncRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relays via withOptionsFrom)
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3️⃣ ATTACHMENTS LAST (depend on both relays and webView)
    std::unique_ptr<juce::WebSliderParameterAttachment> saturationAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> dopplerShiftAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> feedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterBandHighAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterBandLowAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> lfoRateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> lfoDepthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> distortionLevelAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> masterOutputAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> reverseAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassSaturationAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDopplerAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> lfoTempoSyncAttachment;

    // Helper for resource serving
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
