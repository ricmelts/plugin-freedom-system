#pragma once
#include "PluginProcessor.h"
#include <JuceHeader.h>

// Custom LookAndFeel for filmstrip knob rendering
class FilmstripLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FilmstripLookAndFeel();

    void drawRotarySlider(juce::Graphics& g,
                         int x, int y,
                         int width, int height,
                         float sliderPosProportional,
                         float rotaryStartAngle,
                         float rotaryEndAngle,
                         juce::Slider& slider) override;

    void setFilmstripImage(juce::Image image, int frameCount, int frameWidth, int frameHeight);

private:
    juce::Image filmstripImage;
    int numFrames = 0;
    int filmstripFrameWidth = 0;
    int filmstripFrameHeight = 0;
};

// Custom LookAndFeel for medium filmstrip knobs (sub-knobs) - reuse filmstrip logic
class MediumFilmstripLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MediumFilmstripLookAndFeel();

    void drawRotarySlider(juce::Graphics& g,
                         int x, int y,
                         int width, int height,
                         float sliderPosProportional,
                         float rotaryStartAngle,
                         float rotaryEndAngle,
                         juce::Slider& slider) override;

    void setFilmstripImage(juce::Image image, int frameCount, int frameWidth, int frameHeight);

private:
    juce::Image filmstripImage;
    int numFrames = 0;
    int filmstripFrameWidth = 0;
    int filmstripFrameHeight = 0;
};

// Custom VU Meter component for real-time audio level display
class VUMeter : public juce::Component, private juce::Timer
{
public:
    VUMeter()
    {
        startTimerHz(30); // 30 FPS refresh rate
    }

    void setLevel(float newLevel)
    {
        // Smooth level changes with decay
        targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw dark background
        g.setColour(juce::Colour(0xff1a0a00)); // Dark brown
        g.fillRoundedRectangle(bounds, 4.0f);

        // Draw 2px black border
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds, 4.0f, 2.0f);

        // Draw red LED bar from bottom up
        if (currentLevel > 0.0f)
        {
            float barHeight = bounds.getHeight() * currentLevel;
            juce::Rectangle<float> levelBar(
                bounds.getX() + 2.0f,
                bounds.getBottom() - barHeight - 2.0f,
                bounds.getWidth() - 4.0f,
                barHeight
            );

            g.setColour(juce::Colour(0xffF21A1D)); // Red #F21A1D
            g.fillRoundedRectangle(levelBar, 2.0f);
        }
    }

private:
    void timerCallback() override
    {
        // Smooth decay for level display
        const float decayRate = 0.95f;
        if (currentLevel > targetLevel)
            currentLevel *= decayRate;
        else
            currentLevel = targetLevel;

        repaint();
    }

    float currentLevel = 0.0f;
    float targetLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

class RedShiftDistortionAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor&);
    ~RedShiftDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RedShiftDistortionAudioProcessor& processorRef;

    // Background images
    juce::Image diamondPlateTexture;
    juce::Image screwImage;

    // Custom fonts
    juce::Typeface::Ptr jacquardTypeface;
    juce::Typeface::Ptr handjetTypeface;

    // Custom LookAndFeel for main filmstrip knob (big)
    FilmstripLookAndFeel filmstripLookAndFeel;

    // Custom LookAndFeel for sub-knobs filmstrip (medium)
    MediumFilmstripLookAndFeel mediumFilmstripLookAndFeel;

    // Main doppler shift knob (168px diameter, centered)
    juce::Slider dopplerKnob;
    juce::Label dopplerLabel;
    juce::Label dopplerValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> dopplerAttachment;

    // 6 surrounding knobs (130px diameter each)
    juce::Slider stereoWidthKnob;
    juce::Label stereoWidthLabel;
    juce::Label stereoWidthValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> stereoWidthAttachment;

    juce::Slider feedbackKnob;
    juce::Label feedbackLabel;
    juce::Label feedbackValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> feedbackAttachment;

    juce::Slider masterOutputKnob;
    juce::Label masterOutputLabel;
    juce::Label masterOutputValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> masterOutputAttachment;

    juce::Slider loCutKnob;
    juce::Label loCutLabel;
    juce::Label loCutValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> loCutAttachment;

    juce::Slider saturationKnob;
    juce::Label saturationLabel;
    juce::Label saturationValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> saturationAttachment;

    juce::Slider hiCutKnob;
    juce::Label hiCutLabel;
    juce::Label hiCutValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> hiCutAttachment;

    // VU Meters (stereo)
    VUMeter leftVUMeter;
    VUMeter rightVUMeter;
    juce::Label leftVULabel;
    juce::Label rightVULabel;

    // Advanced settings (bottom-right)
    juce::Slider grainSizeSlider;
    juce::Label grainSizeLabel;
    juce::Label grainSizeValueLabel;
    std::unique_ptr<juce::SliderParameterAttachment> grainSizeAttachment;

    juce::ComboBox grainOverlapCombo;
    juce::Label grainOverlapLabel;
    std::unique_ptr<juce::ComboBoxParameterAttachment> grainOverlapAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessorEditor)
};
