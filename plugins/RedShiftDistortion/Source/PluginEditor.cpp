#include "PluginEditor.h"
#include "BinaryData.h"

RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ========================================================================
    // INITIALIZATION ORDER (matches declaration order in header)
    // 1. Create relays FIRST
    // 2. Create WebView with relay options
    // 3. Create attachments LAST
    // ========================================================================

    // 1️⃣ CREATE RELAYS (one per parameter)
    dopplerShiftRelay = std::make_unique<juce::WebSliderRelay>("DOPPLER_SHIFT");
    stereoWidthRelay = std::make_unique<juce::WebSliderRelay>("STEREO_WIDTH");
    feedbackRelay = std::make_unique<juce::WebSliderRelay>("FEEDBACK");
    masterOutputRelay = std::make_unique<juce::WebSliderRelay>("MASTER_OUTPUT");
    loCutFilterRelay = std::make_unique<juce::WebSliderRelay>("FILTER_BAND_LOW");
    saturationRelay = std::make_unique<juce::WebSliderRelay>("SATURATION");
    hiCutFilterRelay = std::make_unique<juce::WebSliderRelay>("FILTER_BAND_HIGH");
    bypassDopplerRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_DOPPLER");
    bypassSaturationRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_SATURATION");

    // 2️⃣ CREATE WEBVIEW with relay options
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](const juce::String& url) { return getResource(url); })
            .withOptionsFrom(*dopplerShiftRelay)
            .withOptionsFrom(*stereoWidthRelay)
            .withOptionsFrom(*feedbackRelay)
            .withOptionsFrom(*masterOutputRelay)
            .withOptionsFrom(*loCutFilterRelay)
            .withOptionsFrom(*saturationRelay)
            .withOptionsFrom(*hiCutFilterRelay)
            .withOptionsFrom(*bypassDopplerRelay)
            .withOptionsFrom(*bypassSaturationRelay)
    );

    // 3️⃣ CREATE ATTACHMENTS (JUCE 8 requires 3 parameters: parameter, relay, undoManager)
    dopplerShiftAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("DOPPLER_SHIFT"), *dopplerShiftRelay, nullptr);
    stereoWidthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("STEREO_WIDTH"), *stereoWidthRelay, nullptr);
    feedbackAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("FEEDBACK"), *feedbackRelay, nullptr);
    masterOutputAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("MASTER_OUTPUT"), *masterOutputRelay, nullptr);
    loCutFilterAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("FILTER_BAND_LOW"), *loCutFilterRelay, nullptr);
    saturationAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("SATURATION"), *saturationRelay, nullptr);
    hiCutFilterAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.parameters.getParameter("FILTER_BAND_HIGH"), *hiCutFilterRelay, nullptr);
    bypassDopplerAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.parameters.getParameter("BYPASS_DOPPLER"), *bypassDopplerRelay, nullptr);
    bypassSaturationAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.parameters.getParameter("BYPASS_SATURATION"), *bypassSaturationRelay, nullptr);

    // Add WebView to component and navigate to root
    addAndMakeVisible(*webView);
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // Set fixed window size (950x700px from v6 mockup spec)
    setSize(950, 700);
    setResizable(false, false);
}

RedShiftDistortionAudioProcessorEditor::~RedShiftDistortionAudioProcessorEditor()
{
    // Destruction happens automatically in REVERSE order:
    // 1. Attachments destroyed first (can safely use webView)
    // 2. WebView destroyed second
    // 3. Relays destroyed last
}

void RedShiftDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView fills entire component - no custom painting needed
    g.fillAll(juce::Colours::black);
}

void RedShiftDistortionAudioProcessorEditor::resized()
{
    // WebView fills entire editor window
    webView->setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource> RedShiftDistortionAudioProcessorEditor::getResource(const juce::String& url)
{
    // Helper lambda to convert BinaryData to vector<byte>
    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Explicit URL mapping (BinaryData flattens paths: "js/juce/index.js" → "index_js")
    // HTML requests original paths, so we map them explicitly

    // Main HTML page
    if (url == "/" || url == "/index.html") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_html, BinaryData::index_htmlSize),
            juce::String("text/html")
        };
    }

    // JUCE WebView bridge (REQUIRED)
    if (url == "/js/juce/index.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_js, BinaryData::index_jsSize),
            juce::String("application/javascript")
        };
    }

    // JUCE native interop check (REQUIRED for WebView initialization)
    if (url == "/js/juce/check_native_interop.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::check_native_interop_js, BinaryData::check_native_interop_jsSize),
            juce::String("application/javascript")
        };
    }

    // UI assets (images from v6 mockup)
    if (url == "/assets/00fa754f6e134fb61f5e452b7ba8415e529dd82a.png") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::_00fa754f6e134fb61f5e452b7ba8415e529dd82a_png,
                      BinaryData::_00fa754f6e134fb61f5e452b7ba8415e529dd82a_pngSize),
            juce::String("image/png")
        };
    }

    if (url == "/assets/4287f6bace032d3fb347fc04a01722012ff802a1.png") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::_4287f6bace032d3fb347fc04a01722012ff802a1_png,
                      BinaryData::_4287f6bace032d3fb347fc04a01722012ff802a1_pngSize),
            juce::String("image/png")
        };
    }

    if (url == "/assets/f26475b6e81b4d3a1423f46d75b9ae5c2611416a.png") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::f26475b6e81b4d3a1423f46d75b9ae5c2611416a_png,
                      BinaryData::f26475b6e81b4d3a1423f46d75b9ae5c2611416a_pngSize),
            juce::String("image/png")
        };
    }

    // v10: HISE filmstrip knobs
    if (url == "/assets/hise_knob_big.png") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::hise_knob_big_png, BinaryData::hise_knob_big_pngSize),
            juce::String("image/png")
        };
    }

    if (url == "/assets/hise_knob_small.png") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::hise_knob_small_png, BinaryData::hise_knob_small_pngSize),
            juce::String("image/png")
        };
    }

    // 404 - Resource not found
    return std::nullopt;
}
