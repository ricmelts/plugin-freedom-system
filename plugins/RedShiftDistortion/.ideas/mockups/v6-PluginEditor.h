#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

/**
 * RedShiftDistortion Audio Processor Editor (WebView UI)
 *
 * UI Mockup: v6 (950x700px, industrial metal aesthetic)
 * Generated: 2026-02-08
 *
 * CRITICAL: Member declaration order (prevents release build crashes)
 * Order: Relays → WebView → Attachments
 * Destruction order is REVERSE of declaration
 *
 * WebView Requirements:
 * - juce::juce_gui_extra module linked in CMakeLists.txt
 * - JUCE_WEB_BROWSER=1 compile definition
 * - UI assets bundled via juce_add_binary_data
 */
class RedShiftDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor&);
    ~RedShiftDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    /**
     * Resource provider for WebView
     * Maps URLs to BinaryData resources with correct MIME types
     */
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    // Reference to processor
    RedShiftDistortionAudioProcessor& audioProcessor;

    // ========================================================================
    // ⚠️ CRITICAL MEMBER DECLARATION ORDER ⚠️
    // Order: Relays → WebView → Attachments
    // Members destroyed in REVERSE order (attachments first, relays last)
    // Prevents WebView use-after-free in release builds
    // ========================================================================

    // 1️⃣ RELAYS FIRST (no dependencies)
    std::unique_ptr<juce::WebSliderRelay> dopplerShiftRelay;
    std::unique_ptr<juce::WebSliderRelay> stereoWidthRelay;
    std::unique_ptr<juce::WebSliderRelay> feedbackRelay;
    std::unique_ptr<juce::WebSliderRelay> masterOutputRelay;
    std::unique_ptr<juce::WebSliderRelay> loCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> saturationRelay;
    std::unique_ptr<juce::WebSliderRelay> hiCutFilterRelay;
    std::unique_ptr<juce::WebSliderRelay> grainSizeRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDopplerRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassSaturationRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassStereoWidthRelay;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassDelayRelay;
    std::unique_ptr<juce::WebComboBoxRelay> grainOverlapRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relays)
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3️⃣ ATTACHMENTS LAST (depend on relays and webView)
    std::unique_ptr<juce::WebSliderParameterAttachment> dopplerShiftAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> stereoWidthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> feedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> masterOutputAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> loCutFilterAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> saturationAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> hiCutFilterAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> grainSizeAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDopplerAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassSaturationAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassStereoWidthAttachment;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassDelayAttachment;
    std::unique_ptr<juce::WebComboBoxParameterAttachment> grainOverlapAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
