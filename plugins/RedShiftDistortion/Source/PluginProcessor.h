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

    // Stage 2: Tape Delay (with feedback loop - Phase 2.3)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelay;

    // Feedback buffer (stores filtered feedback signal for next iteration)
    juce::AudioBuffer<float> feedbackBuffer;

    // Stereo width smoothing (exponential smoothing, 10ms time constant)
    float smoothedStereoControl = 0.0f;
    float stereoSmoothingCoeff = 0.0f;  // Calculated in prepareToPlay

    // Stage 3: Granular Pitch Shifter (Phase 2.2 - isolated testing)
    struct GrainEngine
    {
        static constexpr int MAX_GRAIN_SIZE_SAMPLES = 9600;  // 200ms at 48kHz
        static constexpr int MAX_GRAINS = 4;  // 4x overlap maximum

        // Grain buffer (circular buffer for grain playback)
        std::array<float, MAX_GRAIN_SIZE_SAMPLES> grainBuffer;
        int grainBufferWritePos = 0;

        // Hann window lookup table (pre-calculated in prepareToPlay)
        std::array<float, MAX_GRAIN_SIZE_SAMPLES> hannWindow;

        // Active grains (each grain has independent playback position)
        struct Grain
        {
            float readPosition = 0.0f;  // Fractional sample position
            int grainPhase = 0;         // Current phase (0 to grainSize-1)
            bool active = false;
        };
        std::array<Grain, MAX_GRAINS> grains;

        // Grain spawning
        int samplesUntilNextGrain = 0;
        int grainAdvanceSamples = 0;  // Calculated from grainSize / grainOverlap
    };

    std::array<GrainEngine, 2> grainEngines;  // Separate engine per channel

    // Granular parameters (cached from APVTS)
    float currentSampleRate = 48000.0f;

    // Constants (from architecture.md)
    static constexpr float ITD_HALF_MS = 260.0f;  // Maximum L/R differential (ms)
    static constexpr float MAX_DELAY_MS = 300.0f;  // Maximum tape delay (ms)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
