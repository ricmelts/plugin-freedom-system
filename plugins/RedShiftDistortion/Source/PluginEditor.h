#pragma once
#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

class RedShiftDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor&);
    ~RedShiftDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RedShiftDistortionAudioProcessor& audioProcessor;

    // WebView components (Pattern #11: std::unique_ptr for initialization order)
    // Order: Relays → WebView → Attachments (prevents 90% of release build crashes)

    // 1️⃣ RELAYS FIRST (no dependencies)
    // Main controls (7 relays)
    std::unique_ptr<juce::WebSliderRelay> stereoWidthRelay;
    std::unique_ptr<juce::WebSliderRelay> feedbackRelay;
    std::unique_ptr<juce::WebSliderRelay> loCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> hiCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> dopplerShiftRelay;
    std::unique_ptr<juce::WebSliderRelay> saturationRelay;
    std::unique_ptr<juce::WebSliderRelay> masterOutputRelay;

    // Bypass controls (4 toggle relays)
    std::unique_ptr<juce::WebToggleButtonRelay> bypassStereoWidthRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDelayRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDopplerRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassSaturationRelay;

    // Advanced settings (2 relays)
    std::unique_ptr<juce::WebSliderRelay> grainSizeRelay;
    std::unique_ptr<juce::WebComboBoxRelay> grainOverlapRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relays via withOptionsFrom)
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3️⃣ ATTACHMENTS LAST (depend on both relays and webView)
    // Main controls attachments
    std::unique_ptr<juce::WebSliderParameterAttachment> stereoWidthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> feedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> loCutFilterAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> hiCutFilterAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> dopplerShiftAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> saturationAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> masterOutputAttachment;

    // Bypass controls attachments
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassStereoWidthAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDelayAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDopplerAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassSaturationAttachment;

    // Advanced settings attachments
    std::unique_ptr<juce::WebSliderParameterAttachment> grainSizeAttachment;
    std::unique_ptr<juce::WebComboBoxParameterAttachment> grainOverlapAttachment;

    // Resource provider
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
