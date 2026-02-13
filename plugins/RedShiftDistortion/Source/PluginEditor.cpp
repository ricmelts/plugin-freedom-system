#include "PluginEditor.h"

RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(600, 400);
}

RedShiftDistortionAudioProcessorEditor::~RedShiftDistortionAudioProcessorEditor()
{
}

void RedShiftDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("RedShiftDistortion - Stage 1", getLocalBounds(), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.drawFittedText("13 parameters implemented",
                     getLocalBounds().reduced(20).removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void RedShiftDistortionAudioProcessorEditor::resized()
{
    // Layout will be added in Stage 2 (GUI)
}
