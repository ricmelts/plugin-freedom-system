#include "PluginEditor.h"

MinimalKickAudioProcessorEditor::MinimalKickAudioProcessorEditor(MinimalKickAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(730, 280);
}

MinimalKickAudioProcessorEditor::~MinimalKickAudioProcessorEditor()
{
}

void MinimalKickAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("MinimalKick - Stage 2", getLocalBounds(), juce::Justification::centred, 1);
}

void MinimalKickAudioProcessorEditor::resized()
{
    // Layout will be added in Stage 5
}
