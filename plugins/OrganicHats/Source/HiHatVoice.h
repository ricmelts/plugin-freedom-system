#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "HiHatSound.h"

class HiHatVoice : public juce::SynthesiserVoice
{
public:
    HiHatVoice(juce::AudioProcessorValueTreeState& apvts);

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int currentPitchWheelPosition) override;

    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                        int startSample, int numSamples) override;

private:
    juce::AudioProcessorValueTreeState& parameters;

    // Noise generation
    juce::Random noiseGenerator;

    // Envelope shaping
    juce::ADSR envelope;

    // Voice state
    bool isClosed = true;  // C1 = closed, D1 = open
    float velocityGain = 1.0f;
};
