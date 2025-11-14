#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class LushPadAudioProcessor : public juce::AudioProcessor
{
public:
    LushPadAudioProcessor();
    ~LushPadAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "LushPad"; }
    bool acceptsMidi() const override { return true; }  // Synth accepts MIDI
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

    juce::AudioProcessorValueTreeState parameters;

private:
    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Voice structure for polyphonic synthesis
    struct SynthVoice
    {
        bool active = false;
        int currentNote = -1;
        float currentVelocity = 0.0f;
        uint64_t timestamp = 0;  // For oldest-note-stealing

        // 3 oscillator phases (detuned ±7 cents)
        float phase1 = 0.0f;  // Base frequency
        float phase2 = 0.0f;  // +7 cents
        float phase3 = 0.0f;  // -7 cents

        // FM feedback memory (1-sample delay per oscillator)
        float previousOutput1 = 0.0f;
        float previousOutput2 = 0.0f;
        float previousOutput3 = 0.0f;

        // Low-pass filter per voice
        juce::dsp::IIR::Filter<float> filter;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        void reset()
        {
            active = false;
            currentNote = -1;
            currentVelocity = 0.0f;
            phase1 = phase2 = phase3 = 0.0f;
            previousOutput1 = previousOutput2 = previousOutput3 = 0.0f;
            filter.reset();
            adsr.reset();
        }
    };

    // Voice management
    static constexpr int maxVoices = 8;
    SynthVoice voices[maxVoices];
    uint64_t voiceCounter = 0;  // Incrementing timestamp for oldest-note-stealing
    double currentSampleRate = 44100.0;

    // Helper methods for voice allocation
    void allocateVoice(int note, float velocity);
    void releaseVoice(int note);
    void startVoice(SynthVoice& voice, int note, float velocity);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LushPadAudioProcessor)
};
