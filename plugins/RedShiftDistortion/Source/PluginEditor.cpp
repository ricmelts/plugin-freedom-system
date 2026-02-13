#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
// FilmstripLookAndFeel Implementation
//==============================================================================

FilmstripLookAndFeel::FilmstripLookAndFeel()
{
    // Use default JUCE V4 styling as base
}

void FilmstripLookAndFeel::setFilmstripImage(juce::Image image, int frameCount, int frameWidth, int frameHeight)
{
    filmstripImage = image;
    numFrames = frameCount;
    filmstripFrameWidth = frameWidth;
    filmstripFrameHeight = frameHeight;
}

void FilmstripLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                           int x, int y,
                                           int width, int height,
                                           float sliderPosProportional,
                                           float rotaryStartAngle,
                                           float rotaryEndAngle,
                                           juce::Slider& slider)
{
    juce::ignoreUnused(rotaryStartAngle, rotaryEndAngle);

    if (filmstripImage.isValid() && numFrames > 0)
    {
        // Calculate which frame to display based on slider value
        const int frameIndex = juce::jlimit(0, numFrames - 1,
            static_cast<int>(sliderPosProportional * (numFrames - 1)));

        // Calculate source rectangle for this frame (vertical filmstrip)
        const int sourceY = frameIndex * filmstripFrameHeight;

        // Draw the frame centered in the component bounds
        g.drawImage(filmstripImage,
                   x, y, width, height,
                   0, sourceY, filmstripFrameWidth, filmstripFrameHeight,
                   false);
    }
    else
    {
        // Fallback: Draw default JUCE rotary slider
        LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                                        sliderPosProportional,
                                        rotaryStartAngle, rotaryEndAngle, slider);
    }
}

//==============================================================================
// MediumFilmstripLookAndFeel Implementation (for sub-knobs)
//==============================================================================

MediumFilmstripLookAndFeel::MediumFilmstripLookAndFeel()
{
    // Use default JUCE V4 styling as base
}

void MediumFilmstripLookAndFeel::setFilmstripImage(juce::Image image, int frameCount, int frameWidth, int frameHeight)
{
    filmstripImage = image;
    numFrames = frameCount;
    filmstripFrameWidth = frameWidth;
    filmstripFrameHeight = frameHeight;
}

void MediumFilmstripLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                                  int x, int y,
                                                  int width, int height,
                                                  float sliderPosProportional,
                                                  float rotaryStartAngle,
                                                  float rotaryEndAngle,
                                                  juce::Slider& slider)
{
    juce::ignoreUnused(rotaryStartAngle, rotaryEndAngle, slider);

    if (filmstripImage.isValid() && numFrames > 0)
    {
        // Calculate which frame to display based on slider value
        const int frameIndex = juce::jlimit(0, numFrames - 1,
            static_cast<int>(sliderPosProportional * (numFrames - 1)));

        // Calculate source rectangle for this frame (vertical filmstrip)
        const int sourceY = frameIndex * filmstripFrameHeight;

        // Draw the frame centered and scaled to fit the component bounds
        g.drawImage(filmstripImage,
                   x, y, width, height,
                   0, sourceY, filmstripFrameWidth, filmstripFrameHeight,
                   false);
    }
    else
    {
        // Fallback: Draw default JUCE rotary slider
        LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                                        sliderPosProportional,
                                        rotaryStartAngle, rotaryEndAngle, slider);
    }
}

//==============================================================================
// RedShiftDistortionAudioProcessorEditor Implementation
//==============================================================================

RedShiftDistortionAudioProcessorEditor::RedShiftDistortionAudioProcessorEditor(RedShiftDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Set editor size to match mockup (950x700)
    setSize(950, 700);

    // Load background textures from BinaryData
    diamondPlateTexture = juce::ImageCache::getFromMemory(
        BinaryData::_00fa754f6e134fb61f5e452b7ba8415e529dd82a_png,
        BinaryData::_00fa754f6e134fb61f5e452b7ba8415e529dd82a_pngSize
    );

    screwImage = juce::ImageCache::getFromMemory(
        BinaryData::_4287f6bace032d3fb347fc04a01722012ff802a1_png,
        BinaryData::_4287f6bace032d3fb347fc04a01722012ff802a1_pngSize
    );

    // Load Jacquard 12 custom font
    jacquardTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Jacquard12Regular_ttf,
        BinaryData::Jacquard12Regular_ttfSize
    );

    // Load Handjet custom font (static version for JUCE compatibility)
    handjetTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::HandjetStatic_ttf,
        BinaryData::HandjetStatic_ttfSize
    );

    // Debug: Check if fonts loaded
    DBG("Jacquard typeface loaded: " << (jacquardTypeface != nullptr ? "YES" : "NO"));
    DBG("Handjet typeface loaded: " << (handjetTypeface != nullptr ? "YES" : "NO"));

    // Load HISE filmstrip knob (128 frames, 180x180 per frame)
    juce::Image knobFilmstrip = juce::ImageCache::getFromMemory(
        BinaryData::hise_knob_big_png,
        BinaryData::hise_knob_big_pngSize
    );

    filmstripLookAndFeel.setFilmstripImage(knobFilmstrip, 128, 180, 180);

    // Load medium filmstrip knob for sub-knobs (128 frames, 100x100 per frame)
    juce::Image mediumKnobFilmstrip = juce::ImageCache::getFromMemory(
        BinaryData::hise_Knob_medium_png,
        BinaryData::hise_Knob_medium_pngSize
    );

    mediumFilmstripLookAndFeel.setFilmstripImage(mediumKnobFilmstrip, 128, 100, 100);

    // Setup main doppler shift knob (168px diameter, centered at x:391, y:266)
    dopplerKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dopplerKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dopplerKnob.setRange(-50.0, 50.0, 0.1);
    dopplerKnob.setValue(0.0);
    dopplerKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,  // -135 degrees
                                    juce::MathConstants<float>::pi * 2.75f,  // +135 degrees
                                    true);  // Stop at end
    dopplerKnob.setLookAndFeel(&filmstripLookAndFeel);
    addAndMakeVisible(dopplerKnob);

    // Setup label below knob ("delay time")
    dopplerLabel.setText("delay time", juce::dontSendNotification);
    dopplerLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        dopplerLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        dopplerLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::plain));
    dopplerLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    dopplerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(dopplerLabel);

    // Setup value display label (shows current delay time)
    dopplerValueLabel.setText("260ms", juce::dontSendNotification);
    dopplerValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        dopplerValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(20.0f));
    else
        dopplerValueLabel.setFont(juce::FontOptions("sans-serif", 20.0f, juce::Font::bold));
    dopplerValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    dopplerValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(dopplerValueLabel);

    // Attach to delayTime parameter (repurposed from dopplerShift)
    auto* delayTimeParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("delayTime")
    );
    if (delayTimeParam)
    {
        dopplerAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *delayTimeParam,
            dopplerKnob
        );

        // Update value label with ms/s formatting
        dopplerKnob.onValueChange = [this]()
        {
            float ms = dopplerKnob.getValue();
            if (ms >= 1000.0f)
            {
                // Display in seconds for values >= 1000ms
                dopplerValueLabel.setText(
                    juce::String(ms / 1000.0f, 1) + "s",
                    juce::dontSendNotification
                );
            }
            else
            {
                // Display in milliseconds
                dopplerValueLabel.setText(
                    juce::String(static_cast<int>(ms)) + "ms",
                    juce::dontSendNotification
                );
            }
        };
    }

    // ========================================================================
    // Setup 6 surrounding knobs (130px diameter each, radial arrangement)
    // ========================================================================

    // 1. Stereo Width (top-left, 10 o'clock) - position: x:173, y:147
    stereoWidthKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stereoWidthKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    stereoWidthKnob.setRange(-100.0, 100.0, 0.1);
    stereoWidthKnob.setValue(0.0);
    stereoWidthKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
    stereoWidthKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(stereoWidthKnob);

    stereoWidthLabel.setText("ping-pong", juce::dontSendNotification);
    stereoWidthLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        stereoWidthLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        stereoWidthLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    stereoWidthLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    stereoWidthLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(stereoWidthLabel);

    stereoWidthValueLabel.setText("0%", juce::dontSendNotification);
    stereoWidthValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        stereoWidthValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        stereoWidthValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    stereoWidthValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    stereoWidthValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(stereoWidthValueLabel);

    auto* pingPongAmountParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("pingPongAmount")
    );
    if (pingPongAmountParam)
    {
        stereoWidthAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *pingPongAmountParam,
            stereoWidthKnob
        );
        stereoWidthKnob.onValueChange = [this]() {
            stereoWidthValueLabel.setText(juce::String(stereoWidthKnob.getValue(), 0) + "%", juce::dontSendNotification);
        };
    }

    // 2. Feedback (top-center, 12 o'clock) - position: x:410, y:123
    feedbackKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    feedbackKnob.setRange(0.0, 95.0, 0.1);
    feedbackKnob.setValue(0.0);
    feedbackKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                     juce::MathConstants<float>::pi * 2.75f,
                                     true);
    feedbackKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(feedbackKnob);

    feedbackLabel.setText("feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        feedbackLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        feedbackLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    feedbackLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    feedbackLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(feedbackLabel);

    feedbackValueLabel.setText("0%", juce::dontSendNotification);
    feedbackValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        feedbackValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        feedbackValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    feedbackValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    feedbackValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(feedbackValueLabel);

    auto* feedbackParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("feedback")
    );
    if (feedbackParam)
    {
        feedbackAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *feedbackParam,
            feedbackKnob
        );
        feedbackKnob.onValueChange = [this]() {
            feedbackValueLabel.setText(juce::String(feedbackKnob.getValue(), 0) + "%", juce::dontSendNotification);
        };
    }

    // 3. Master Output (top-right, 2 o'clock) - position: x:647, y:147
    masterOutputKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterOutputKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterOutputKnob.setRange(-60.0, 12.0, 0.1);
    masterOutputKnob.setValue(0.0);
    masterOutputKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                         juce::MathConstants<float>::pi * 2.75f,
                                         true);
    masterOutputKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(masterOutputKnob);

    masterOutputLabel.setText("master", juce::dontSendNotification);
    masterOutputLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        masterOutputLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        masterOutputLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    masterOutputLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    masterOutputLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(masterOutputLabel);

    masterOutputValueLabel.setText("0dB", juce::dontSendNotification);
    masterOutputValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        masterOutputValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        masterOutputValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    masterOutputValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    masterOutputValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(masterOutputValueLabel);

    auto* masterOutputParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("masterOutput")
    );
    if (masterOutputParam)
    {
        masterOutputAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *masterOutputParam,
            masterOutputKnob
        );
        masterOutputKnob.onValueChange = [this]() {
            masterOutputValueLabel.setText(juce::String(masterOutputKnob.getValue(), 1) + "dB", juce::dontSendNotification);
        };
    }

    // 4. Lo-Cut Filter (bottom-left, 8 o'clock) - position: x:173, y:438
    loCutKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    loCutKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    loCutKnob.setRange(20.0, 20000.0, 1.0);
    loCutKnob.setValue(100.0);
    loCutKnob.setSkewFactorFromMidPoint(1000.0);  // Logarithmic scaling
    loCutKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                  juce::MathConstants<float>::pi * 2.75f,
                                  true);
    loCutKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(loCutKnob);

    loCutLabel.setText("lo-cut", juce::dontSendNotification);
    loCutLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        loCutLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        loCutLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    loCutLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    loCutLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(loCutLabel);

    loCutValueLabel.setText("100Hz", juce::dontSendNotification);
    loCutValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        loCutValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        loCutValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    loCutValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    loCutValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(loCutValueLabel);

    auto* loCutParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("filterBandLow")
    );
    if (loCutParam)
    {
        loCutAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *loCutParam,
            loCutKnob
        );
        loCutKnob.onValueChange = [this]() {
            float val = loCutKnob.getValue();
            juce::String text = val >= 1000.0f ? juce::String(val / 1000.0f, 1) + "kHz" : juce::String(val, 0) + "Hz";
            loCutValueLabel.setText(text, juce::dontSendNotification);
        };
    }

    // 5. Saturation (bottom-center, 6 o'clock) - position: x:410, y:462
    saturationKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturationKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    saturationKnob.setRange(-12.0, 24.0, 0.1);
    saturationKnob.setValue(0.0);
    saturationKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                       juce::MathConstants<float>::pi * 2.75f,
                                       true);
    saturationKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(saturationKnob);

    saturationLabel.setText("saturation", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        saturationLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        saturationLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    saturationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    saturationLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(saturationLabel);

    saturationValueLabel.setText("0dB", juce::dontSendNotification);
    saturationValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        saturationValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        saturationValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    saturationValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    saturationValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(saturationValueLabel);

    auto* saturationParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("saturation")
    );
    if (saturationParam)
    {
        saturationAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *saturationParam,
            saturationKnob
        );
        saturationKnob.onValueChange = [this]() {
            saturationValueLabel.setText(juce::String(saturationKnob.getValue(), 1) + "dB", juce::dontSendNotification);
        };
    }

    // 6. Hi-Cut Filter (bottom-right, 4 o'clock) - position: x:647, y:438
    hiCutKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    hiCutKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    hiCutKnob.setRange(20.0, 20000.0, 1.0);
    hiCutKnob.setValue(8000.0);
    hiCutKnob.setSkewFactorFromMidPoint(1000.0);  // Logarithmic scaling
    hiCutKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                  juce::MathConstants<float>::pi * 2.75f,
                                  true);
    hiCutKnob.setLookAndFeel(&mediumFilmstripLookAndFeel);
    addAndMakeVisible(hiCutKnob);

    hiCutLabel.setText("hi-cut", juce::dontSendNotification);
    hiCutLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        hiCutLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(14.0f));
    else
        hiCutLabel.setFont(juce::FontOptions("sans-serif", 14.0f, juce::Font::plain));
    hiCutLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    hiCutLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(hiCutLabel);

    hiCutValueLabel.setText("8kHz", juce::dontSendNotification);
    hiCutValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        hiCutValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(16.0f));
    else
        hiCutValueLabel.setFont(juce::FontOptions("sans-serif", 16.0f, juce::Font::bold));
    hiCutValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    hiCutValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(hiCutValueLabel);

    auto* hiCutParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("filterBandHigh")
    );
    if (hiCutParam)
    {
        hiCutAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *hiCutParam,
            hiCutKnob
        );
        hiCutKnob.onValueChange = [this]() {
            float val = hiCutKnob.getValue();
            juce::String text = val >= 1000.0f ? juce::String(val / 1000.0f, 1) + "kHz" : juce::String(val, 0) + "Hz";
            hiCutValueLabel.setText(text, juce::dontSendNotification);
        };
    }

    // Setup VU meters
    addAndMakeVisible(leftVUMeter);
    addAndMakeVisible(rightVUMeter);

    // Setup VU meter labels
    leftVULabel.setText("L", juce::dontSendNotification);
    leftVULabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        leftVULabel.setFont(juce::FontOptions(handjetTypeface).withHeight(18.0f));
    else
        leftVULabel.setFont(juce::FontOptions("sans-serif", 18.0f, juce::Font::bold));
    leftVULabel.setColour(juce::Label::textColourId, juce::Colours::white);
    leftVULabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(leftVULabel);

    rightVULabel.setText("R", juce::dontSendNotification);
    rightVULabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        rightVULabel.setFont(juce::FontOptions(handjetTypeface).withHeight(18.0f));
    else
        rightVULabel.setFont(juce::FontOptions("sans-serif", 18.0f, juce::Font::bold));
    rightVULabel.setColour(juce::Label::textColourId, juce::Colours::white);
    rightVULabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(rightVULabel);

    // ========================================================================
    // Setup advanced settings (grain size slider + grain overlap dropdown)
    // ========================================================================

    // Grain Size Slider (horizontal slider, x:815, y:630, 90x23)
    grainSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    grainSizeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    grainSizeSlider.setRange(25.0, 200.0, 1.0);
    grainSizeSlider.setValue(100.0);
    addAndMakeVisible(grainSizeSlider);

    grainSizeLabel.setText("GRAIN SIZE", juce::dontSendNotification);
    grainSizeLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        grainSizeLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(12.0f));
    else
        grainSizeLabel.setFont(juce::FontOptions("sans-serif", 12.0f, juce::Font::plain));
    grainSizeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    grainSizeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(grainSizeLabel);

    grainSizeValueLabel.setText("100ms", juce::dontSendNotification);
    grainSizeValueLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        grainSizeValueLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(11.0f));
    else
        grainSizeValueLabel.setFont(juce::FontOptions("sans-serif", 11.0f, juce::Font::plain));
    grainSizeValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    grainSizeValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(grainSizeValueLabel);

    auto* grainSizeParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("grainSize")
    );
    if (grainSizeParam)
    {
        grainSizeAttachment = std::make_unique<juce::SliderParameterAttachment>(
            *grainSizeParam,
            grainSizeSlider
        );
        grainSizeSlider.onValueChange = [this]() {
            grainSizeValueLabel.setText(juce::String(static_cast<int>(grainSizeSlider.getValue())) + "ms", juce::dontSendNotification);
        };
    }

    // Grain Overlap Dropdown (x:815, y:665, 90x26)
    grainOverlapCombo.addItem("2x", 1);
    grainOverlapCombo.addItem("4x", 2);
    grainOverlapCombo.setSelectedId(2);  // Default to 4x
    addAndMakeVisible(grainOverlapCombo);

    grainOverlapLabel.setText("OVERLAP", juce::dontSendNotification);
    grainOverlapLabel.setJustificationType(juce::Justification::centred);
    if (handjetTypeface != nullptr)
        grainOverlapLabel.setFont(juce::FontOptions(handjetTypeface).withHeight(12.0f));
    else
        grainOverlapLabel.setFont(juce::FontOptions("sans-serif", 12.0f, juce::Font::plain));
    grainOverlapLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    grainOverlapLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(grainOverlapLabel);

    auto* grainOverlapParam = dynamic_cast<juce::RangedAudioParameter*>(
        processorRef.parameters.getParameter("grainOverlap")
    );
    if (grainOverlapParam)
    {
        grainOverlapAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(
            *grainOverlapParam,
            grainOverlapCombo
        );
    }

    // Start timer for VU meter updates (30 FPS)
    startTimerHz(30);
}

RedShiftDistortionAudioProcessorEditor::~RedShiftDistortionAudioProcessorEditor()
{
    // Reset LookAndFeel before destruction
    dopplerKnob.setLookAndFeel(nullptr);
    stereoWidthKnob.setLookAndFeel(nullptr);
    feedbackKnob.setLookAndFeel(nullptr);
    masterOutputKnob.setLookAndFeel(nullptr);
    loCutKnob.setLookAndFeel(nullptr);
    saturationKnob.setLookAndFeel(nullptr);
    hiCutKnob.setLookAndFeel(nullptr);
}

void RedShiftDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Draw diamond plate texture background (cover entire 950x700 area)
    if (diamondPlateTexture.isValid())
    {
        g.drawImage(diamondPlateTexture, getLocalBounds().toFloat(),
                   juce::RectanglePlacement::fillDestination, false);
    }
    else
    {
        // Fallback: solid background color
        g.fillAll(juce::Colour(0xff2a2a2a));
    }

    // Draw corner screws (38px size at 4 corners with 20px padding)
    if (screwImage.isValid())
    {
        const int screwSize = 38;
        const int padding = 20;

        // Top-left
        g.drawImage(screwImage, padding, padding, screwSize, screwSize,
                   0, 0, screwImage.getWidth(), screwImage.getHeight(), false);

        // Top-right
        g.drawImage(screwImage, getWidth() - padding - screwSize, padding, screwSize, screwSize,
                   0, 0, screwImage.getWidth(), screwImage.getHeight(), false);

        // Bottom-left
        g.drawImage(screwImage, padding, getHeight() - padding - screwSize, screwSize, screwSize,
                   0, 0, screwImage.getWidth(), screwImage.getHeight(), false);

        // Bottom-right
        g.drawImage(screwImage, getWidth() - padding - screwSize, getHeight() - padding - screwSize,
                   screwSize, screwSize,
                   0, 0, screwImage.getWidth(), screwImage.getHeight(), false);
    }

    // ========================================================================
    // RED GLOW LAYERS - Draw behind all knobs
    // ========================================================================

    // Helper lambda to draw radial glow effect
    auto drawRadialGlow = [&g](float centerX, float centerY, float radius,
                                juce::Colour color, int blurSteps) {
        for (int i = blurSteps; i > 0; --i)
        {
            float alpha = color.getFloatAlpha() * (static_cast<float>(i) / blurSteps);
            float currentRadius = radius * (1.0f + (blurSteps - i) * 0.15f);
            g.setColour(color.withAlpha(alpha * 0.3f));
            g.fillEllipse(centerX - currentRadius, centerY - currentRadius,
                         currentRadius * 2, currentRadius * 2);
        }
    };

    // Define glow colors (from mockup spec)
    juce::Colour outerGlowColor(242, 26, 29);  // #F21A1D
    juce::Colour innerGlowColor(189, 0, 0);    // #BD0000

    // Main doppler knob glow (center: 391 + 84 = 475, y: 266 + 84 = 350)
    // Note: Glow reduced by 30% (0.8 * 0.7 = 0.56, 0.6 * 0.7 = 0.42)
    const float mainKnobCenterX = 391 + 84.0f;  // 168px / 2
    const float mainKnobCenterY = 266 + 84.0f;
    drawRadialGlow(mainKnobCenterX, mainKnobCenterY, 315.0f / 2, outerGlowColor.withAlpha(0.56f), 20);
    drawRadialGlow(mainKnobCenterX, mainKnobCenterY, 231.0f / 2, innerGlowColor.withAlpha(0.42f), 15);

    // Sub-knob centers (104px / 2 = 52, centered at original positions)
    const float subKnobRadius = 52.0f;

    // 1. Stereo Width (top-left: 186 + 52, 160 + 52)
    drawRadialGlow(186 + subKnobRadius, 160 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(186 + subKnobRadius, 160 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // 2. Feedback (top-center: 423 + 52, 136 + 52)
    drawRadialGlow(423 + subKnobRadius, 136 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(423 + subKnobRadius, 136 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // 3. Master Output (top-right: 660 + 52, 160 + 52)
    drawRadialGlow(660 + subKnobRadius, 160 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(660 + subKnobRadius, 160 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // 4. Lo-Cut Filter (bottom-left: 186 + 52, 451 + 52)
    drawRadialGlow(186 + subKnobRadius, 451 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(186 + subKnobRadius, 451 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // 5. Saturation (bottom-center: 423 + 52, 475 + 52)
    drawRadialGlow(423 + subKnobRadius, 475 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(423 + subKnobRadius, 475 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // 6. Hi-Cut Filter (bottom-right: 660 + 52, 451 + 52)
    drawRadialGlow(660 + subKnobRadius, 451 + subKnobRadius, 100.0f / 2, outerGlowColor.withAlpha(0.56f), 10);
    drawRadialGlow(660 + subKnobRadius, 451 + subKnobRadius, 75.0f / 2, innerGlowColor.withAlpha(0.42f), 8);

    // ========================================================================
    // LABEL BACKGROUNDS - Black rounded rectangles with red glow
    // ========================================================================

    auto drawLabelBackground = [&g, &innerGlowColor](const juce::Label& label) {
        if (!label.isVisible()) return;

        auto bounds = label.getBounds().toFloat();

        // Draw red glow (simulate text-shadow: 0px -7px 50px #bd0000)
        for (int i = 0; i < 8; ++i)
        {
            float offset = i * 2.0f;
            float alpha = 0.15f * (1.0f - i / 8.0f);
            g.setColour(innerGlowColor.withAlpha(alpha));
            g.fillRoundedRectangle(bounds.expanded(offset), 6.0f + offset * 0.5f);
        }

        // Draw black background rectangle
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 6.0f);
    };

    // Draw backgrounds for all labels
    drawLabelBackground(dopplerLabel);
    drawLabelBackground(dopplerValueLabel);
    drawLabelBackground(stereoWidthLabel);
    drawLabelBackground(stereoWidthValueLabel);
    drawLabelBackground(feedbackLabel);
    drawLabelBackground(feedbackValueLabel);
    drawLabelBackground(masterOutputLabel);
    drawLabelBackground(masterOutputValueLabel);
    drawLabelBackground(loCutLabel);
    drawLabelBackground(loCutValueLabel);
    drawLabelBackground(saturationLabel);
    drawLabelBackground(saturationValueLabel);
    drawLabelBackground(hiCutLabel);
    drawLabelBackground(hiCutValueLabel);

    // Draw title "RED SHIFT" with red glow effect (position: x:475, y:60, size: 60px)
    // Using Jacquard 12 custom font (loaded from embedded TTF)
    if (jacquardTypeface != nullptr)
    {
        g.setFont(juce::FontOptions(jacquardTypeface).withHeight(60.0f));
    }
    else
    {
        // Fallback to monospace if font fails to load
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 60.0f, juce::Font::bold));
    }

    // Red glow shadow effect (multiple passes for glow intensity)
    g.setColour(juce::Colour(0xffbd0000).withAlpha(0.3f));
    for (int offset = 10; offset > 0; offset -= 2)
    {
        g.drawText("RED SHIFT", 0, 60 - offset, getWidth(), 80,
                  juce::Justification::centred, false);
        g.drawText("RED SHIFT", 0, 60 + offset, getWidth(), 80,
                  juce::Justification::centred, false);
    }

    // Main title text (black)
    g.setColour(juce::Colour(0xff000000));
    g.drawText("RED SHIFT", 0, 60, getWidth(), 80,
              juce::Justification::centred, false);

    // Draw subtitle "digital delay" at bottom (position: x:475, y:660, size: 30px)
    // Using Handjet custom font (loaded from embedded TTF)
    if (handjetTypeface != nullptr)
    {
        g.setFont(juce::FontOptions(handjetTypeface).withHeight(30.0f));
    }
    else
    {
        // Fallback to sans-serif if font fails to load
        g.setFont(juce::FontOptions("sans-serif", 30.0f, juce::Font::plain));
    }

    // Measure text width for background rectangle
    juce::String subtitleText = "digital delay";
    int textWidth = g.getCurrentFont().getStringWidth(subtitleText);
    int padding = 15;

    // Draw black rounded rectangle background
    g.setColour(juce::Colours::black);
    juce::Rectangle<float> bgRect((getWidth() - textWidth) / 2.0f - padding, 660.0f,
                                  textWidth + padding * 2, 40.0f);
    g.fillRoundedRectangle(bgRect, 8.0f);

    // Subtitle text (white)
    g.setColour(juce::Colours::white);
    g.drawText(subtitleText, 0, 660, getWidth(), 40,
              juce::Justification::centred, false);
}

void RedShiftDistortionAudioProcessorEditor::resized()
{
    // Main doppler knob: 168px diameter, position from mockup (x:391, y:266)
    const int knobSize = 168;
    const int knobX = 391;
    const int knobY = 266;

    dopplerKnob.setBounds(knobX, knobY, knobSize, knobSize);

    // Label below the knob
    dopplerLabel.setBounds(knobX - 50, knobY + knobSize + 5, knobSize + 100, 30);

    // Value display in center of knob
    dopplerValueLabel.setBounds(knobX, knobY + knobSize / 2 - 15, knobSize, 30);

    // Sub-knobs: 104px diameter (0.8 scale of original 130px)
    // Positions adjusted by +13px offset to keep knobs centered at original positions
    const int subKnobSize = 104;

    // 1. Stereo Width (top-left, x:186, y:160 - centered at original 173+65, 147+65)
    stereoWidthKnob.setBounds(186, 160, subKnobSize, subKnobSize);
    stereoWidthLabel.setBounds(186 - 20, 160 + subKnobSize + 3, subKnobSize + 40, 25);
    stereoWidthValueLabel.setBounds(186, 160 + subKnobSize / 2 - 12, subKnobSize, 24);

    // 2. Feedback (top-center, x:423, y:136 - centered at original 410+65, 123+65)
    feedbackKnob.setBounds(423, 136, subKnobSize, subKnobSize);
    feedbackLabel.setBounds(423 - 20, 136 + subKnobSize + 3, subKnobSize + 40, 25);
    feedbackValueLabel.setBounds(423, 136 + subKnobSize / 2 - 12, subKnobSize, 24);

    // 3. Master Output (top-right, x:660, y:160 - centered at original 647+65, 147+65)
    masterOutputKnob.setBounds(660, 160, subKnobSize, subKnobSize);
    masterOutputLabel.setBounds(660 - 20, 160 + subKnobSize + 3, subKnobSize + 40, 25);
    masterOutputValueLabel.setBounds(660, 160 + subKnobSize / 2 - 12, subKnobSize, 24);

    // 4. Lo-Cut Filter (bottom-left, x:186, y:451 - centered at original 173+65, 438+65)
    loCutKnob.setBounds(186, 451, subKnobSize, subKnobSize);
    loCutLabel.setBounds(186 - 20, 451 + subKnobSize + 3, subKnobSize + 40, 25);
    loCutValueLabel.setBounds(186, 451 + subKnobSize / 2 - 12, subKnobSize, 24);

    // 5. Saturation (bottom-center, x:423, y:475 - centered at original 410+65, 462+65)
    saturationKnob.setBounds(423, 475, subKnobSize, subKnobSize);
    saturationLabel.setBounds(423 - 20, 475 + subKnobSize + 3, subKnobSize + 40, 25);
    saturationValueLabel.setBounds(423, 475 + subKnobSize / 2 - 12, subKnobSize, 24);

    // 6. Hi-Cut Filter (bottom-right, x:660, y:451 - centered at original 647+65, 438+65)
    hiCutKnob.setBounds(660, 451, subKnobSize, subKnobSize);
    hiCutLabel.setBounds(660 - 20, 451 + subKnobSize + 3, subKnobSize + 40, 25);
    hiCutValueLabel.setBounds(660, 451 + subKnobSize / 2 - 12, subKnobSize, 24);

    // VU meters (right side, x:820, y:175, 40x350px each, 10px gap)
    const int vuX = 820;
    const int vuY = 175;
    const int vuWidth = 40;
    const int vuHeight = 350;
    const int vuGap = 10;

    leftVUMeter.setBounds(vuX, vuY, vuWidth, vuHeight);
    rightVUMeter.setBounds(vuX + vuWidth + vuGap, vuY, vuWidth, vuHeight);

    // VU meter labels (above meters)
    leftVULabel.setBounds(vuX, vuY - 25, vuWidth, 20);
    rightVULabel.setBounds(vuX + vuWidth + vuGap, vuY - 25, vuWidth, 20);

    // Advanced settings (bottom-right)
    const int advancedX = 815;

    // Grain Size Slider (x:815, y:630, 90x23)
    grainSizeLabel.setBounds(advancedX, 607, 90, 18);
    grainSizeSlider.setBounds(advancedX, 630, 90, 23);
    grainSizeValueLabel.setBounds(advancedX + 25, 607, 40, 18);  // Right-aligned value next to label

    // Grain Overlap Dropdown (x:815, y:665)
    grainOverlapLabel.setBounds(advancedX, 658, 90, 18);
    grainOverlapCombo.setBounds(advancedX, 678, 90, 26);
}

void RedShiftDistortionAudioProcessorEditor::timerCallback()
{
    // Update VU meters with levels from processor
    float levelL = processorRef.getOutputLevel(0);
    float levelR = processorRef.getOutputLevel(1);

    leftVUMeter.setLevel(levelL);
    rightVUMeter.setLevel(levelR);
}
