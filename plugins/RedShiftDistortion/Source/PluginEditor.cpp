#include "PluginEditor.h"
#include "BinaryData.h"

RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // 1️⃣ Create relays FIRST (Pattern #11)
    stereoWidthRelay = std::make_unique<juce::WebSliderRelay>("STEREO_WIDTH");
    feedbackRelay = std::make_unique<juce::WebSliderRelay>("FEEDBACK");
    filterBandLowRelay = std::make_unique<juce::WebSliderRelay>("FILTER_BAND_LOW");
    filterBandHighRelay = std::make_unique<juce::WebSliderRelay>("FILTER_BAND_HIGH");
    dopplerShiftRelay = std::make_unique<juce::WebSliderRelay>("DOPPLER_SHIFT");
    saturationRelay = std::make_unique<juce::WebSliderRelay>("SATURATION");
    masterOutputRelay = std::make_unique<juce::WebSliderRelay>("MASTER_OUTPUT");

    bypassStereoWidthRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_STEREO_WIDTH");
    bypassDelayRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_DELAY");
    bypassDopplerRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_DOPPLER");
    bypassSaturationRelay = std::make_unique<juce::WebToggleButtonRelay>("BYPASS_SATURATION");

    grainSizeRelay = std::make_unique<juce::WebSliderRelay>("GRAIN_SIZE");
    grainOverlapRelay = std::make_unique<juce::WebSliderRelay>("GRAIN_OVERLAP");

    // 2️⃣ Create WebView with relay options
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](auto& url) { return getResource(url); })
            .withOptionsFrom(*stereoWidthRelay)
            .withOptionsFrom(*feedbackRelay)
            .withOptionsFrom(*filterBandLowRelay)
            .withOptionsFrom(*filterBandHighRelay)
            .withOptionsFrom(*dopplerShiftRelay)
            .withOptionsFrom(*saturationRelay)
            .withOptionsFrom(*masterOutputRelay)
            .withOptionsFrom(*bypassStereoWidthRelay)
            .withOptionsFrom(*bypassDelayRelay)
            .withOptionsFrom(*bypassDopplerRelay)
            .withOptionsFrom(*bypassSaturationRelay)
            .withOptionsFrom(*grainSizeRelay)
            .withOptionsFrom(*grainOverlapRelay)
    );

    // 3️⃣ Create attachments LAST (Pattern #12: 3 params)
    stereoWidthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("STEREO_WIDTH"), *stereoWidthRelay, nullptr);
    feedbackAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("FEEDBACK"), *feedbackRelay, nullptr);
    filterBandLowAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("FILTER_BAND_LOW"), *filterBandLowRelay, nullptr);
    filterBandHighAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("FILTER_BAND_HIGH"), *filterBandHighRelay, nullptr);
    dopplerShiftAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("DOPPLER_SHIFT"), *dopplerShiftRelay, nullptr);
    saturationAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("SATURATION"), *saturationRelay, nullptr);
    masterOutputAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("MASTER_OUTPUT"), *masterOutputRelay, nullptr);

    bypassStereoWidthAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("BYPASS_STEREO_WIDTH"), *bypassStereoWidthRelay, nullptr);
    bypassDelayAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("BYPASS_DELAY"), *bypassDelayRelay, nullptr);
    bypassDopplerAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("BYPASS_DOPPLER"), *bypassDopplerRelay, nullptr);
    bypassSaturationAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("BYPASS_SATURATION"), *bypassSaturationRelay, nullptr);

    grainSizeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("GRAIN_SIZE"), *grainSizeRelay, nullptr);
    grainOverlapAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("GRAIN_OVERLAP"), *grainOverlapRelay, nullptr);

    addAndMakeVisible(*webView);
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    setSize(800, 600);
}

RedShiftDistortionAudioProcessorEditor::~RedShiftDistortionAudioProcessorEditor() = default;

void RedShiftDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView handles all painting
    juce::ignoreUnused(g);
}

void RedShiftDistortionAudioProcessorEditor::resized()
{
    webView->setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
RedShiftDistortionAudioProcessorEditor::getResource(const juce::String& url)
{
    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Explicit URL mapping (Pattern #8: ALWAYS REQUIRED)
    if (url == "/" || url == "/index.html") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_html, BinaryData::index_htmlSize),
            juce::String("text/html")
        };
    }

    if (url == "/js/juce/index.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_js, BinaryData::index_jsSize),
            juce::String("text/javascript")
        };
    }

    if (url == "/js/juce/check_native_interop.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::check_native_interop_js, BinaryData::check_native_interop_jsSize),
            juce::String("text/javascript")
        };
    }

    return std::nullopt;
}
