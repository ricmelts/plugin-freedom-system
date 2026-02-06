#include "PluginEditor.h"
#include "BinaryData.h"

RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // 1. Create relays FIRST (initialize with parameter IDs matching APVTS)
    saturationRelay = std::make_unique<juce::WebSliderRelay>("saturation");
    dopplerShiftRelay = std::make_unique<juce::WebSliderRelay>("dopplerShift");
    pitchEnableRelay = std::make_unique<juce::WebToggleButtonRelay>("pitchEnable");
    delayTimeRelay = std::make_unique<juce::WebSliderRelay>("delayTime");
    tempoSyncRelay = std::make_unique<juce::WebToggleButtonRelay>("tempoSync");
    delayLevelRelay = std::make_unique<juce::WebSliderRelay>("delayLevel");
    distortionLevelRelay = std::make_unique<juce::WebSliderRelay>("distortionLevel");
    masterOutputRelay = std::make_unique<juce::WebSliderRelay>("masterOutput");

    // 2. Create WebView with relay options
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](const auto& url) { return getResource(url); })
            .withOptionsFrom(*saturationRelay)
            .withOptionsFrom(*dopplerShiftRelay)
            .withOptionsFrom(*pitchEnableRelay)
            .withOptionsFrom(*delayTimeRelay)
            .withOptionsFrom(*tempoSyncRelay)
            .withOptionsFrom(*delayLevelRelay)
            .withOptionsFrom(*distortionLevelRelay)
            .withOptionsFrom(*masterOutputRelay)
    );

    // 3. Create attachments LAST (connect parameters to relays)
    saturationAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("saturation"), *saturationRelay, nullptr);

    dopplerShiftAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("dopplerShift"), *dopplerShiftRelay, nullptr);

    pitchEnableAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("pitchEnable"), *pitchEnableRelay, nullptr);

    delayTimeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("delayTime"), *delayTimeRelay, nullptr);

    tempoSyncAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *processorRef.parameters.getParameter("tempoSync"), *tempoSyncRelay, nullptr);

    delayLevelAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("delayLevel"), *delayLevelRelay, nullptr);

    distortionLevelAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("distortionLevel"), *distortionLevelRelay, nullptr);

    masterOutputAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("masterOutput"), *masterOutputRelay, nullptr);

    // Add WebView to editor
    addAndMakeVisible(*webView);

    // Navigate to UI
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // Set editor size (600x400 from mockup/design)
    setSize(600, 400);
}

RedShiftDistortionAudioProcessorEditor::~RedShiftDistortionAudioProcessorEditor()
{
    // Attachments destroyed first (reverse order of declaration)
    // Then WebView, then Relays - safe destruction order
}

void RedShiftDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView handles all painting
    juce::ignoreUnused(g);
}

void RedShiftDistortionAudioProcessorEditor::resized()
{
    // WebView fills entire editor
    webView->setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
RedShiftDistortionAudioProcessorEditor::getResource(const juce::String& url)
{
    // Helper: Convert binary data to vector<byte>
    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Explicit URL mapping (clear, debuggable, reliable)
    // Root "/" → index.html
    if (url == "/" || url == "/index.html") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_html, BinaryData::index_htmlSize),
            juce::String("text/html")
        };
    }

    // CSS
    if (url == "/css/styles.css") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::styles_css, BinaryData::styles_cssSize),
            juce::String("text/css")
        };
    }

    // JavaScript - JUCE bridge
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

    // JavaScript - Application
    if (url == "/js/app.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::app_js, BinaryData::app_jsSize),
            juce::String("text/javascript")
        };
    }

    // Resource not found
    juce::Logger::writeToLog("Resource not found: " + url);
    return std::nullopt;
}
