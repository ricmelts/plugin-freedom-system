#include "PluginEditor.h"

FlutterVerbAudioProcessorEditor::FlutterVerbAudioProcessorEditor(FlutterVerbAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // WebView mockup is 600x640px (v6 mockup dimensions)
    // Using 700x400px for Stage 2 placeholder, will adjust in Stage 5
    setSize(700, 400);
}

FlutterVerbAudioProcessorEditor::~FlutterVerbAudioProcessorEditor()
{
}

void FlutterVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("FlutterVerb - Stage 3", getLocalBounds(), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.drawFittedText("7 parameters implemented",
                     getLocalBounds().reduced(20).removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void FlutterVerbAudioProcessorEditor::resized()
{
    // WebView layout will be added in Stage 5
}
