#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

//==============================================================================
// Professional Reverse Delay with Double-Buffering and Crossfading
// Uses state machine to accumulate grains, then play back in reverse
//==============================================================================
class ReverseDelay
{
public:
    enum class State { FILLING, PLAYING, CROSSFADING };

    ReverseDelay() = default;

    void prepare(double sampleRate, int maxDelayMs)
    {
        fs = static_cast<float>(sampleRate);
        bufferSize = static_cast<int>((sampleRate * maxDelayMs) / 1000.0);

        // Dual buffers for seamless crossfading
        bufferA.setSize(2, bufferSize);
        bufferB.setSize(2, bufferSize);
        bufferA.clear();
        bufferB.clear();

        // Grain parameters
        grainSize = 4096;  // ~93ms at 44.1kHz (larger grain = smoother reverse)
        crossfadeLength = 64;  // ~1.5ms crossfade (research suggests 32-64 samples)

        // Pre-calculate Hann window for grain envelope
        hannWindow.resize(static_cast<size_t>(grainSize));
        for (int i = 0; i < grainSize; ++i)
        {
            float phase = static_cast<float>(i) / static_cast<float>(grainSize - 1);
            hannWindow[static_cast<size_t>(i)] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));
        }

        reset();
    }

    void reset()
    {
        state = State::FILLING;
        fillBuffer = &bufferA;
        playBuffer = &bufferB;

        fillPos = 0;
        playPos = 0;
        crossfadePos = 0;

        bufferA.clear();
        bufferB.clear();
    }

    float processSample(float input, int channel)
    {
        float output = 0.0f;

        // Always write to fill buffer
        fillBuffer->setSample(channel, fillPos, input);

        switch (state)
        {
            case State::FILLING:
            {
                // Accumulation phase - output silence or last sample
                output = (playPos > 0) ? playBuffer->getSample(channel, playPos) : 0.0f;

                fillPos++;
                if (fillPos >= grainSize)
                {
                    // Grain full - switch to playback
                    state = State::PLAYING;
                    playPos = grainSize - 1;  // Start reading from end
                    fillPos = 0;  // Reset fill position for next grain
                }
                break;
            }

            case State::PLAYING:
            {
                // Read backwards with Hann windowing
                float window = hannWindow[static_cast<size_t>(playPos)];
                output = playBuffer->getSample(channel, playPos) * window;

                fillPos++;
                playPos--;

                // Check if we need to start crossfading
                if (playPos < crossfadeLength && fillPos >= grainSize - crossfadeLength)
                {
                    state = State::CROSSFADING;
                    crossfadePos = 0;
                }
                // Or if playback finished without crossfade
                else if (playPos < 0)
                {
                    // Swap buffers
                    std::swap(fillBuffer, playBuffer);
                    state = State::FILLING;
                    fillPos = 0;
                    playPos = 0;
                }
                break;
            }

            case State::CROSSFADING:
            {
                // Crossfade between ending playback and new grain
                float oldWindow = hannWindow[static_cast<size_t>(std::max(0, playPos))];
                float oldSample = (playPos >= 0) ? playBuffer->getSample(channel, playPos) * oldWindow : 0.0f;

                // New grain plays from the end backwards
                int newPlayPos = grainSize - 1 - crossfadePos;
                float newWindow = hannWindow[static_cast<size_t>(newPlayPos)];
                float newSample = fillBuffer->getSample(channel, newPlayPos) * newWindow;

                // Constant-power crossfade
                float crossfade = static_cast<float>(crossfadePos) / static_cast<float>(crossfadeLength);
                crossfade = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * crossfade));

                output = oldSample * (1.0f - crossfade) + newSample * crossfade;

                fillPos++;
                playPos--;
                crossfadePos++;

                if (crossfadePos >= crossfadeLength)
                {
                    // Crossfade complete - swap buffers
                    std::swap(fillBuffer, playBuffer);
                    state = State::PLAYING;
                    playPos = grainSize - 1 - crossfadeLength;  // Continue from crossfade end
                    fillPos = 0;
                }
                break;
            }
        }

        return output;
    }

private:
    State state = State::FILLING;

    juce::AudioBuffer<float> bufferA, bufferB;
    juce::AudioBuffer<float>* fillBuffer = nullptr;   // Currently filling
    juce::AudioBuffer<float>* playBuffer = nullptr;   // Currently playing

    std::vector<float> hannWindow;

    float fs = 44100.0f;
    int bufferSize = 0;
    int grainSize = 4096;
    int crossfadeLength = 64;

    int fillPos = 0;      // Write position in fill buffer
    int playPos = 0;      // Read position in play buffer (counts backwards)
    int crossfadePos = 0; // Position within crossfade
};

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

    // Delay components - Stereo delay lines for L/R doppler effect
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineLeft;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineRight;

    // Feedback buffer (stores delayed signal for feedback loop)
    juce::AudioBuffer<float> feedbackBuffer;

    // Tape delay feedback filters (hi-cut and lo-cut)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> hiCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> loCutFilter;

    // Doppler control smoothing (avoid clicks)
    float currentDopplerControl { 0.0f };

    // LFO phase for continuous delay time modulation (tape wow & flutter)
    float lfoPhase { 0.0f };

    // Professional reverse delay with grain-based windowing
    ReverseDelay reverseDelay;

    // Helper methods (none needed)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RedShiftDistortionAudioProcessor)
};
