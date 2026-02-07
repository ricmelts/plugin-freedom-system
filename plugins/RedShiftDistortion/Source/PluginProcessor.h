#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

//==============================================================================
// Professional Reverse Delay with Grain-Based Windowing
// Eliminates clicks and digital artifacts through dual-buffer crossfading
//==============================================================================
class ReverseDelay
{
public:
    ReverseDelay() = default;

    void prepare(double sampleRate, int maxDelayMs)
    {
        bufferSize = static_cast<int>((sampleRate * maxDelayMs) / 1000.0);

        // Dual buffers for seamless crossfading
        bufferA.setSize(2, bufferSize);
        bufferB.setSize(2, bufferSize);
        bufferA.clear();
        bufferB.clear();

        // Pre-calculate Hann window for grain envelope
        grainSize = 2048;  // ~46ms at 44.1kHz
        crossfadeLength = 256;  // ~5.8ms crossfade
        hannWindow.resize(grainSize);

        for (int i = 0; i < grainSize; ++i)
        {
            float phase = static_cast<float>(i) / static_cast<float>(grainSize - 1);
            hannWindow[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));
        }

        reset();
    }

    void reset()
    {
        writePos = 0;
        readPosA = 0;
        readPosB = 0;
        useBufferA = true;
        crossfadePhase = 0.0f;
        bufferA.clear();
        bufferB.clear();
    }

    float processSample(float input, int channel, int delaySamples)
    {
        // Write to both buffers continuously
        bufferA.setSample(channel, writePos, input);
        bufferB.setSample(channel, writePos, input);

        // Read from active buffer backwards with Hann windowing
        float outputA = readWithWindow(bufferA, channel, readPosA);
        float outputB = readWithWindow(bufferB, channel, readPosB);

        // Crossfade between buffers for seamless operation
        float crossfade = calculateCrossfade();
        float output = outputA * (1.0f - crossfade) + outputB * crossfade;

        // Update positions
        updatePositions(delaySamples);

        return output;
    }

private:
    juce::AudioBuffer<float> bufferA, bufferB;
    std::vector<float> hannWindow;
    int bufferSize = 0;
    int grainSize = 2048;
    int crossfadeLength = 256;

    int writePos = 0;
    int readPosA = 0;
    int readPosB = 0;
    bool useBufferA = true;
    float crossfadePhase = 0.0f;

    float readWithWindow(const juce::AudioBuffer<float>& buffer, int channel, int readPos)
    {
        if (readPos < 0 || readPos >= bufferSize)
            return 0.0f;

        // Calculate grain position (wrapping)
        int grainPos = (writePos - readPos + bufferSize) % grainSize;

        // Apply Hann window envelope
        float window = (grainPos >= 0 && grainPos < grainSize) ? hannWindow[grainPos] : 0.0f;

        return buffer.getSample(channel, readPos) * window;
    }

    float calculateCrossfade()
    {
        if (crossfadePhase > 0.0f && crossfadePhase < 1.0f)
        {
            // Raised cosine crossfade for constant power
            return 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * crossfadePhase));
        }
        return useBufferA ? 0.0f : 1.0f;
    }

    void updatePositions(int delaySamples)
    {
        // Advance write position forward
        writePos = (writePos + 1) % bufferSize;

        // Move read positions backward (reverse playback)
        readPosA--;
        readPosB--;

        // Wrap negative positions
        if (readPosA < 0) readPosA += bufferSize;
        if (readPosB < 0) readPosB += bufferSize;

        // Update crossfade if active
        if (crossfadePhase > 0.0f && crossfadePhase < 1.0f)
        {
            crossfadePhase += 1.0f / static_cast<float>(crossfadeLength);

            if (crossfadePhase >= 1.0f)
            {
                crossfadePhase = 0.0f;
                useBufferA = !useBufferA;
            }
        }

        // Check for boundary and initiate crossfade
        int distanceToOrigin = std::min(readPosA, readPosB);
        if (distanceToOrigin < crossfadeLength && crossfadePhase == 0.0f)
        {
            // Start crossfade
            crossfadePhase = 0.01f;

            // Reset inactive buffer's read position
            if (useBufferA)
            {
                readPosB = writePos - delaySamples;
                if (readPosB < 0) readPosB += bufferSize;
            }
            else
            {
                readPosA = writePos - delaySamples;
                if (readPosA < 0) readPosA += bufferSize;
            }
        }
    }
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
