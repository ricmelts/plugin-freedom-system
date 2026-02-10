#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class RedShiftDistortionAudioProcessor : public juce::AudioProcessor
{
public:
    RedShiftDistortionAudioProcessor();
    ~RedShiftDistortionAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RedShiftDistortion"; }
    bool acceptsMidi() const override { return false; }  // Effect, not instrument
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public APVTS for UI access
    juce::AudioProcessorValueTreeState parameters;

private:
    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components (Phase 2.1: Stereo Width + Basic Delay)

    // Stage 1: Stereo Width Modulation
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoWidthDelayLeft;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoWidthDelayRight;

    // Stage 2: Tape Delay (basic pass-through for Phase 2.1, feedback added in Phase 2.3)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelay;

    // Stereo width smoothing (exponential smoothing, 10ms time constant)
    float smoothedStereoControl = 0.0f;
    float stereoSmoothingCoeff = 0.0f;  // Calculated in prepareToPlay

    // Constants (from architecture.md)
    static constexpr float ITD_HALF_MS = 260.0f;  // Maximum L/R differential (ms)
    static constexpr float MAX_DELAY_MS = 300.0f;  // Maximum tape delay (ms)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
