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

    // DSP Components (One-Sided Tape Delay Architecture)
    juce::dsp::ProcessSpec spec;

    // Tape delay lines (L + R channels) - user-controllable delay time (10-2000ms)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelayL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> tapeDelayR;

    // Saturation + Filters + Feedback Loop

    // Tube saturation (asymmetrical tanh waveshaper)
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

    // Ping-pong cross-feedback buffers
    float pingPongBufferL = 0.0f;
    float pingPongBufferR = 0.0f;

    // Cached sample rate for delay time calculations
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
