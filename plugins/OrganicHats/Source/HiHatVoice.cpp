#include "HiHatVoice.h"

HiHatVoice::HiHatVoice(juce::AudioProcessorValueTreeState& apvts)
    : parameters(apvts)
{
}

bool HiHatVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<HiHatSound*>(sound) != nullptr;
}

void HiHatVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int)
{
    // Determine if this is a closed or open hi-hat
    isClosed = (midiNoteNumber == 36);  // C1 = closed, D1 (38) = open

    // Store velocity as linear gain (0.0-1.0)
    velocityGain = velocity;

    // Configure ADSR based on note type
    if (isClosed)
    {
        // Closed hi-hat: Short decay, no sustain
        // Read CLOSED_DECAY parameter (20-200ms)
        auto* decayParam = parameters.getRawParameterValue("CLOSED_DECAY");
        float decayMs = decayParam->load();

        juce::ADSR::Parameters adsrParams;
        adsrParams.attack = 0.0001f;   // 0.1ms attack
        adsrParams.decay = decayMs / 1000.0f;  // Convert ms to seconds
        adsrParams.sustain = 0.0f;     // No sustain
        adsrParams.release = 0.005f;   // 5ms release

        envelope.setParameters(adsrParams);
    }
    else
    {
        // Open hi-hat: No decay, full sustain, long release
        // Read OPEN_RELEASE parameter (100-1000ms)
        auto* releaseParam = parameters.getRawParameterValue("OPEN_RELEASE");
        float releaseMs = releaseParam->load();

        juce::ADSR::Parameters adsrParams;
        adsrParams.attack = 0.0001f;   // 0.1ms attack
        adsrParams.decay = 0.0f;       // No decay
        adsrParams.sustain = 1.0f;     // Full sustain
        adsrParams.release = releaseMs / 1000.0f;  // Convert ms to seconds

        envelope.setParameters(adsrParams);
    }

    // Trigger envelope
    envelope.noteOn();
}

void HiHatVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        envelope.noteOff();
    }
    else
    {
        // Immediate cutoff
        clearCurrentNote();
        envelope.reset();
    }
}

void HiHatVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Generate white noise: range [-1.0, 1.0]
        float noiseSample = (noiseGenerator.nextFloat() * 2.0f) - 1.0f;

        // Apply envelope
        float envelopeSample = envelope.getNextSample();

        // Apply velocity scaling
        float outputSample = noiseSample * envelopeSample * velocityGain;

        // Add to output buffer (don't replace - multiple voices may be active)
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addSample(channel, startSample + sample, outputSample);
        }

        // Stop voice if envelope finished
        if (!envelope.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}
