#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter layout creation (BEFORE constructor)
juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // MAIN CONTROLS (7 FloatParameters)

    // 1. STEREO_WIDTH - Stereo Width (-100% to +100%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "STEREO_WIDTH", 1 },
        "Stereo Width",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 2. FEEDBACK - Feedback (0% to 95%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FEEDBACK", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 3. FILTER_BAND_LOW - Lo-Cut (20Hz to 20000Hz, default: 100Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_LOW", 1 },
        "Lo-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        100.0f,
        "Hz"
    ));

    // 4. FILTER_BAND_HIGH - Hi-Cut (20Hz to 20000Hz, default: 8000Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "FILTER_BAND_HIGH", 1 },
        "Hi-Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skew 0.3 for log scale
        8000.0f,
        "Hz"
    ));

    // 5. DOPPLER_SHIFT - Doppler Shift (-50% to +50%, default: 0%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DOPPLER_SHIFT", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // 6. SATURATION - Saturation (-12dB to +24dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SATURATION", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // 7. MASTER_OUTPUT - Master Output (-60dB to +12dB, default: 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "MASTER_OUTPUT", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // BYPASS CONTROLS (2 BoolParameters)

    // 8. BYPASS_DOPPLER - Bypass Doppler (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_DOPPLER", 1 },
        "Bypass Doppler",
        false
    ));

    // 9. BYPASS_SATURATION - Bypass Saturation (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS_SATURATION", 1 },
        "Bypass Saturation",
        false
    ));

    return layout;
}

RedShiftDistortionAudioProcessor::RedShiftDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)   // Effect: stereo input
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)) // Effect: stereo output
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

RedShiftDistortionAudioProcessor::~RedShiftDistortionAudioProcessor()
{
}

void RedShiftDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Create ProcessSpec for juce::dsp components (Pattern #17)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;  // Mono per-channel processing

    // Calculate stereo width smoothing coefficient (10ms time constant)
    const float smoothingTimeMs = 10.0f;
    stereoSmoothingCoeff = 1.0f - std::exp(-1.0f / (smoothingTimeMs * 0.001f * static_cast<float>(sampleRate)));

    // Calculate maximum delay in samples
    const int maxStereoWidthDelaySamples = static_cast<int>(std::ceil((ITD_HALF_MS / 1000.0f) * sampleRate));
    const int maxTapeDelaySamples = static_cast<int>(std::ceil((MAX_DELAY_MS / 1000.0f) * sampleRate));

    // Initialize stereo width delay lines (Stage 1: Psychoacoustic Shift)
    stereoWidthDelayLeft.prepare(spec);
    stereoWidthDelayLeft.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayLeft.reset();

    stereoWidthDelayRight.prepare(spec);
    stereoWidthDelayRight.setMaximumDelayInSamples(maxStereoWidthDelaySamples);
    stereoWidthDelayRight.reset();

    // Initialize tape delay lines (Stage 2: Delay with Feedback)
    tapeDelayLeft.prepare(spec);
    tapeDelayLeft.setMaximumDelayInSamples(maxTapeDelaySamples);
    tapeDelayLeft.reset();

    tapeDelayRight.prepare(spec);
    tapeDelayRight.setMaximumDelayInSamples(maxTapeDelaySamples);
    tapeDelayRight.reset();

    // Initialize filters (Stage 3: Hi-Cut and Lo-Cut in feedback path)
    hiCutFilterLeft.prepare(spec);
    hiCutFilterLeft.reset();
    hiCutFilterRight.prepare(spec);
    hiCutFilterRight.reset();

    loCutFilterLeft.prepare(spec);
    loCutFilterLeft.reset();
    loCutFilterRight.prepare(spec);
    loCutFilterRight.reset();

    // Reset state variables
    smoothedStereoControl = 0.0f;
    feedbackHistoryLeft = 0.0f;
    feedbackHistoryRight = 0.0f;
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (Phase 2.1+)
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Early exit if no channels
    if (numChannels == 0 || numSamples == 0)
        return;

    // Read parameters (atomic, real-time safe)
    const float delayTimeMs = parameters.getRawParameterValue("STEREO_WIDTH")->load();  // UI: "DELAY"
    const float dopplerShiftPercent = parameters.getRawParameterValue("DOPPLER_SHIFT")->load();  // UI: "SHIFT"
    const float feedbackAmount = parameters.getRawParameterValue("FEEDBACK")->load() / 100.0f;  // 0-95% → 0.0-0.95
    const float saturationDb = parameters.getRawParameterValue("SATURATION")->load();
    const float filterLowHz = parameters.getRawParameterValue("FILTER_BAND_LOW")->load();  // Lo-cut (highpass)
    const float filterHighHz = parameters.getRawParameterValue("FILTER_BAND_HIGH")->load();  // Hi-cut (lowpass)
    const float masterOutputDb = parameters.getRawParameterValue("MASTER_OUTPUT")->load();
    const bool bypassDoppler = parameters.getRawParameterValue("BYPASS_DOPPLER")->load() > 0.5f;
    const bool bypassSaturation = parameters.getRawParameterValue("BYPASS_SATURATION")->load() > 0.5f;

    // Calculate target doppler control (-1.0 to +1.0) for psychoacoustic "shift" effect
    const float targetDopplerControl = bypassDoppler ? 0.0f : (dopplerShiftPercent / 100.0f);

    // Calculate gains
    const float saturationGain = bypassSaturation ? 1.0f : std::pow(10.0f, saturationDb / 20.0f);
    const float masterGain = std::pow(10.0f, masterOutputDb / 20.0f);

    // Get sample rate for delay time calculations
    const float sampleRate = static_cast<float>(getSampleRate());

    // Calculate delay times in samples
    const float delayTimeSamples = (delayTimeMs / 1000.0f) * sampleRate;

    // Update filter coefficients (do once per block for efficiency)
    auto loCutCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, filterLowHz);
    auto hiCutCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, filterHighHz);

    *loCutFilterLeft.coefficients = *loCutCoeffs;
    *loCutFilterRight.coefficients = *loCutCoeffs;
    *hiCutFilterLeft.coefficients = *hiCutCoeffs;
    *hiCutFilterRight.coefficients = *hiCutCoeffs;

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Smooth doppler control (exponential smoothing, 10ms time constant)
        smoothedStereoControl += (targetDopplerControl - smoothedStereoControl) * stereoSmoothingCoeff;

        // Calculate L/R ITD offsets for psychoacoustic shift (ITD_HALF = 260ms maximum)
        const float itdDelayTimeSamples = (ITD_HALF_MS / 1000.0f) * sampleRate * smoothedStereoControl;
        const float leftITDSamples = itdDelayTimeSamples;    // Positive offset
        const float rightITDSamples = -itdDelayTimeSamples;  // Negative offset (opposite)

        // Process left and right channels independently
        for (int ch = 0; ch < std::min(numChannels, 2); ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            const float input = channelData[sample];

            // STAGE 1: Psychoacoustic Shift (ITD-based delay for spatial effect)
            float shiftOutput = input;
            if (ch == 0)  // Left channel
            {
                stereoWidthDelayLeft.pushSample(0, input);
                stereoWidthDelayLeft.setDelay(std::abs(leftITDSamples));
                shiftOutput = stereoWidthDelayLeft.popSample(0);
            }
            else if (ch == 1)  // Right channel
            {
                stereoWidthDelayRight.pushSample(0, input);
                stereoWidthDelayRight.setDelay(std::abs(rightITDSamples));
                shiftOutput = stereoWidthDelayRight.popSample(0);
            }

            // STAGE 2: Tape Delay with Feedback Loop
            // Add feedback from previous iteration (filtered)
            float& feedbackHistory = (ch == 0) ? feedbackHistoryLeft : feedbackHistoryRight;
            const float delayInput = shiftOutput + (feedbackHistory * feedbackAmount);

            // Push to delay line
            if (ch == 0)
            {
                tapeDelayLeft.pushSample(0, delayInput);
                tapeDelayLeft.setDelay(delayTimeSamples);
            }
            else
            {
                tapeDelayRight.pushSample(0, delayInput);
                tapeDelayRight.setDelay(delayTimeSamples);
            }

            // Get delayed signal
            const float delayOutput = (ch == 0) ? tapeDelayLeft.popSample(0) : tapeDelayRight.popSample(0);

            // STAGE 3: Saturation (soft clipping with gain)
            float saturatedSignal = delayOutput * saturationGain;
            if (!bypassSaturation)
            {
                // Soft clip using tanh for smooth saturation
                saturatedSignal = std::tanh(saturatedSignal);
            }

            // STAGE 4: Feedback Path Filtering (lo-cut + hi-cut)
            float filteredFeedback = saturatedSignal;
            if (ch == 0)
            {
                filteredFeedback = loCutFilterLeft.processSample(filteredFeedback);
                filteredFeedback = hiCutFilterLeft.processSample(filteredFeedback);
            }
            else
            {
                filteredFeedback = loCutFilterRight.processSample(filteredFeedback);
                filteredFeedback = hiCutFilterRight.processSample(filteredFeedback);
            }

            // Store for next iteration
            feedbackHistory = filteredFeedback;

            // STAGE 5: Mix dry and wet, apply master output gain
            const float wetSignal = saturatedSignal;
            const float drySignal = input;
            const float mixedSignal = wetSignal;  // 100% wet for now (could add dry/wet mix later)

            channelData[sample] = mixedSignal * masterGain;
        }
    }
}

juce::AudioProcessorEditor* RedShiftDistortionAudioProcessor::createEditor()
{
    return new RedShiftDistortionAudioProcessorEditor(*this);
}

void RedShiftDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RedShiftDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RedShiftDistortionAudioProcessor();
}
