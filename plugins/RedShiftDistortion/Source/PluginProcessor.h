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
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Report latency from granular pitch shifter
    int getLatencySamples() const { return grainSizeSamples / 2; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components (BEFORE parameters for initialization order)
    juce::dsp::ProcessSpec spec;

    // Delay path components
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine;

    // Delay time modulator (LFO for doppler simulation)
    float lfoPhase { 0.0f };
    float currentDelayTimeMs { 250.0f };  // Smoothed delay time (prevent clicks)
    float targetDelayTimeMs { 250.0f };   // Target delay time (from parameter or tempo sync)

    // Granular synthesis components (pitch shifting + time stretching)
    struct GrainState
    {
        float readPosition { 0.0f };      // Current read position in grain buffer (float for fractional samples)
        float playbackRate { 1.0f };      // Pitch shift rate (1.0 = no shift)
        float grainPhase { 0.0f };        // Current phase in grain (0.0 to 1.0)
        bool isActive { false };          // Is this grain currently playing?
    };

    static constexpr int NUM_GRAINS = 4;  // 4 overlapping grains (75% overlap)
    static constexpr int GRAIN_SIZE_MS = 100;  // 100ms grain size

    std::array<GrainState, NUM_GRAINS> grains;
    juce::AudioBuffer<float> grainBuffer;  // Circular buffer for grain playback (delay line output)
    int grainBufferWritePos { 0 };
    int grainBufferSize { 0 };
    int grainSizeSamples { 0 };
    int grainAdvanceSamples { 0 };  // 25% grain advance (4 overlapping grains)
    int samplesSinceLastGrainSpawn { 0 };

    std::vector<float> hannWindow;  // Hann window for grain envelope

    // Parallel path buffers (pre-allocated in prepareToPlay)
    juce::AudioBuffer<float> delayPathBuffer;
    juce::AudioBuffer<float> distortionPathBuffer;
    juce::AudioBuffer<float> grainOutputBuffer;  // Granular synthesis output (pitch-shifted)

    // Helper methods
    float quantizeDelayTimeToTempo(float hostBpm, float delayTimeMs);
    float getHostBpm();
    void initializeGranularEngine();
    void spawnGrain(float pitchShiftRate);
    float getHannWindowValue(float phase);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
