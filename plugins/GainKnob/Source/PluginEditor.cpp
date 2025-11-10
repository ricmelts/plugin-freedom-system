#include "PluginEditor.h"
#include "BinaryData.h"

GainKnobAudioProcessorEditor::GainKnobAudioProcessorEditor(GainKnobAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)

    // Initialize relay with parameter ID (MUST match APVTS ID exactly)
    , gainRelay("GAIN")

    // Initialize WebView with options
    , webView(juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled()  // CRITICAL: Enables JUCE JavaScript library
        .withResourceProvider([this](const auto& url) { return getResource(url); })
        .withOptionsFrom(gainRelay)      // Register relay
    )

    // Initialize attachment (connect parameter to relay)
    , gainAttachment(*processorRef.apvts.getParameter("GAIN"), gainRelay)
{
    // Add WebView to editor
    addAndMakeVisible(webView);

    // Navigate to UI (root of resource provider)
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // Set editor size (from UI mockup dimensions)
    setSize(400, 400);
}

GainKnobAudioProcessorEditor::~GainKnobAudioProcessorEditor()
{
    // Destructor - members destroyed in reverse declaration order
    // This order ensures attachments stop using webView before it's destroyed
}

void GainKnobAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView handles all painting
    juce::ignoreUnused(g);
}

void GainKnobAudioProcessorEditor::resized()
{
    // WebView fills entire editor
    webView.setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
GainKnobAudioProcessorEditor::getResource(const juce::String& url)
{
    // Map URLs to embedded resources
    auto resource = url.replaceCharacter('\\', '/');

    // Root "/" → index.html
    if (resource == "/" || resource.isEmpty())
        resource = "/index.html";

    // Remove leading slash for BinaryData lookup
    auto path = resource.substring(1);

    // Find in binary data (files embedded from ui/public/)
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        if (path == BinaryData::namedResourceList[i])
        {
            int dataSize = 0;
            const char* data = BinaryData::getNamedResource(
                BinaryData::namedResourceList[i], dataSize);

            // Determine MIME type
            juce::String mimeType = "text/html";
            if (path.endsWith(".css")) mimeType = "text/css";
            if (path.endsWith(".js")) mimeType = "application/javascript";
            if (path.endsWith(".png")) mimeType = "image/png";
            if (path.endsWith(".jpg") || path.endsWith(".jpeg")) mimeType = "image/jpeg";
            if (path.endsWith(".svg")) mimeType = "image/svg+xml";

            // Convert raw pointer to vector<std::byte> for JUCE 8
            std::vector<std::byte> dataVector;
            dataVector.reserve(static_cast<size_t>(dataSize));
            for (int j = 0; j < dataSize; ++j)
                dataVector.push_back(static_cast<std::byte>(data[j]));

            return juce::WebBrowserComponent::Resource{
                std::move(dataVector), mimeType
            };
        }
    }

    // Resource not found
    juce::Logger::writeToLog("Resource not found: " + url);
    return std::nullopt;
}
