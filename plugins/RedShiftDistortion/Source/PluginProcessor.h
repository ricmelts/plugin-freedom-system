#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

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
    bool acceptsMidi() const override { return false; }  // Audio effect, not instrument
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // TODO: Latency reporting temporarily disabled (compilation issue with override)
    // Total latency ~460ms (260ms stereo width + 200ms max grain size)
    // int getLatencySamples() override { return static_cast<int>(0.460 * currentSampleRate); }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS for parameter management
    juce::AudioProcessorValueTreeState parameters;

    // Level metering for UI
    std::atomic<float> outputLevelL{ 0.0f };
    std::atomic<float> outputLevelR{ 0.0f };

    float getOutputLevel(int channel) const {
        return channel == 0 ? outputLevelL.load() : outputLevelR.load();
    }

private:
    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components (Phase 2.1: Stereo Width + Basic Delay)
    juce::dsp::ProcessSpec spec;

    // Stereo width delay lines (L + R channels)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoWidthDelayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoWidthDelayR;

    // Tape delay lines (L + R channels) - 260ms base delay
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelayR;

    // Phase 2.2: Saturation + Filters + Feedback Loop

    // Cumulative saturation (tanh waveshaper)
    juce::dsp::WaveShaper<float> saturationL;
    juce::dsp::WaveShaper<float> saturationR;

    // Dual-band filtering (hi-pass + lo-pass)
    juce::dsp::IIR::Filter<float> hiPassFilterL;
    juce::dsp::IIR::Filter<float> hiPassFilterR;
    juce::dsp::IIR::Filter<float> loPassFilterL;
    juce::dsp::IIR::Filter<float> loPassFilterR;

    // Feedback state (previous output samples for feedback loop)
    float feedbackStateL = 0.0f;
    float feedbackStateR = 0.0f;

    // Phase 2.3: Granular Doppler Shift Components

    // Grain state structure (tracks individual grain playback)
    struct GrainState {
        float readPosition = 0.0f;  // Current read position in grain buffer (fractional samples)
        float grainAge = 0.0f;       // Age of grain in samples (for window calculation)
        bool isActive = false;       // Is this grain currently playing?
    };

    // Grain states for each channel (L + R) - max 4 grains for 4x overlap
    std::array<GrainState, 4> grainsL;
    std::array<GrainState, 4> grainsR;

    // Grain buffers (circular buffers for grain storage)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> grainBufferL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> grainBufferR;

    // Grain scheduling state
    int grainSpacingSamples = 0;      // How many samples between grain starts
    int samplesSinceLastGrain = 0;    // Counter for grain scheduling

    // Cached sample rate for delay time calculations
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
