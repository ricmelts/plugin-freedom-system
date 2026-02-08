#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RedShiftDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // saturation - Float (-12.0 to 24.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "saturation", 1 },
        "Saturation",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // dopplerShift - Float (-50.0 to 50.0 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "dopplerShift", 1 },
        "Doppler Shift",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
        0.0f,
        "%"
    ));

    // feedback - Float (0.0 to 95.0 %)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f,
        "%"
    ));

    // filterBandLow - Float (20.0 to 20000.0 Hz) highpass filter (low band cutoff)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "filterBandLow", 1 },
        "Filter Low Band",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skewed for better low-frequency control
        100.0f,
        "Hz"
    ));

    // filterBandHigh - Float (20.0 to 20000.0 Hz) lowpass filter (high band cutoff)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "filterBandHigh", 1 },
        "Filter High Band",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skewed for better low-frequency control
        8000.0f,
        "Hz"
    ));

    // distortionLevel - Float (-60.0 to 0.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "distortionLevel", 1 },
        "Distortion Level",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -6.0f,
        "dB"
    ));

    // masterOutput - Float (-60.0 to 12.0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "masterOutput", 1 },
        "Master Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));

    // Bypass toggles for troubleshooting
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassSaturation", 1 },
        "Bypass Saturation",
        false
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypassDoppler", 1 },
        "Bypass Doppler",
        false
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "reverse", 1 },
        "Reverse",
        false
    ));

    // lfoRate - Float (0.1 to 10.0 Hz in free mode, 1/16 to 8 bars in sync mode)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "lfoRate", 1 },
        "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f),
        1.0f,
        "Hz"
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "lfoTempoSync", 1 },
        "LFO Tempo Sync",
        false
    ));

    // lfoDepth - Float (0.0 to 100.0 %) controls wow & flutter intensity
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "lfoDepth", 1 },
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f,  // 20% default for moderate tape-style modulation
        "%"
    ));

    return layout;
}

RedShiftDistortionAudioProcessor::RedShiftDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

RedShiftDistortionAudioProcessor::~RedShiftDistortionAudioProcessor()
{
}

void RedShiftDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare DSP spec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;  // Stereo only

    // Prepare mono delay lines (one for left, one for right)
    const int maxDelaySamples = static_cast<int>(sampleRate * 0.30);  // 300ms max (allows 260ms doppler + margin)
    delayLineLeft.setMaximumDelayInSamples(maxDelaySamples);
    delayLineRight.setMaximumDelayInSamples(maxDelaySamples);

    // Each delay line is mono (1 channel)
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    delayLineLeft.prepare(monoSpec);
    delayLineRight.prepare(monoSpec);
    delayLineLeft.reset();
    delayLineRight.reset();

    // Pre-allocate feedback buffer (real-time safety)
    const int numChannels = 2;  // Force stereo
    feedbackBuffer.setSize(numChannels, samplesPerBlock);
    feedbackBuffer.clear();

    // Prepare tape delay feedback filters
    hiCutFilter.prepare(spec);
    loCutFilter.prepare(spec);
    hiCutFilter.reset();
    loCutFilter.reset();

    // Initialize doppler control smoothing
    currentDopplerControl = 0.0f;

    // Initialize LFO phase for delay time modulation
    lfoPhase = 0.0f;

    // Initialize professional reverse delay with grain-based windowing
    reverseDelay.prepare(sampleRate, 300);  // 300ms max delay
}

void RedShiftDistortionAudioProcessor::releaseResources()
{
    // Release feedback buffer to save memory when plugin not in use
    feedbackBuffer.setSize(0, 0);
}

void RedShiftDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters (atomic, real-time safe)
    auto* saturationParam = parameters.getRawParameterValue("saturation");
    auto* dopplerShiftParam = parameters.getRawParameterValue("dopplerShift");
    auto* feedbackParam = parameters.getRawParameterValue("feedback");
    auto* filterBandLowParam = parameters.getRawParameterValue("filterBandLow");
    auto* filterBandHighParam = parameters.getRawParameterValue("filterBandHigh");
    auto* masterOutputParam = parameters.getRawParameterValue("masterOutput");

    // Bypass parameters
    auto* bypassSaturationParam = parameters.getRawParameterValue("bypassSaturation");
    auto* bypassDopplerParam = parameters.getRawParameterValue("bypassDoppler");
    auto* reverseParam = parameters.getRawParameterValue("reverse");
    auto* lfoRateParam = parameters.getRawParameterValue("lfoRate");
    auto* lfoTempoSyncParam = parameters.getRawParameterValue("lfoTempoSync");
    auto* lfoDepthParam = parameters.getRawParameterValue("lfoDepth");

    float saturationDb = saturationParam->load();
    float dopplerShift = dopplerShiftParam->load();
    float feedbackPercent = feedbackParam->load();
    float filterBandLow = filterBandLowParam->load();   // Highpass cutoff (20-20000 Hz)
    float filterBandHigh = filterBandHighParam->load();  // Lowpass cutoff (20-20000 Hz)
    float masterOutputDb = masterOutputParam->load();
    float lfoRate = lfoRateParam->load();
    float lfoDepthPercent = lfoDepthParam->load();  // 0-100%

    // Bypass states
    bool bypassSaturation = bypassSaturationParam->load() > 0.5f;
    bool bypassDoppler = bypassDopplerParam->load() > 0.5f;
    bool reverse = reverseParam->load() > 0.5f;
    bool lfoTempoSync = lfoTempoSyncParam->load() > 0.5f;

    // Convert parameters to usable values
    float saturationGain = std::pow(10.0f, saturationDb / 20.0f);
    float feedbackGain = feedbackPercent / 100.0f;  // 0-95% → 0.0-0.95
    float masterOutputGain = std::pow(10.0f, masterOutputDb / 20.0f);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Ensure feedback buffer matches current block size
    if (feedbackBuffer.getNumSamples() != numSamples)
    {
        feedbackBuffer.setSize(2, numSamples, false, false, true);
        feedbackBuffer.clear();
    }

    // Update feedback filter coefficients (dual-band filter covering full spectrum)
    *hiCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, filterBandHigh);
    *loCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, filterBandLow);

    // ═══════════════════════════════════════════════════════════════════════════
    // SERIES PROCESSING CHAIN: Input → Per-Sample Doppler Delay → Saturation → Output
    //
    // Doppler modulation creates stereo width via differential L/R delay times:
    //   Left  channel: d0 + (ITD_HALF * dopplerControl)
    //   Right channel: d0 - (ITD_HALF * dopplerControl)
    //
    // At max doppler (+50%): 260ms L/R differential creates wide stereo imaging
    // ═══════════════════════════════════════════════════════════════════════════

    // Creative doppler depth (half of maximum L/R differential)
    const float ITD_HALF = 0.260f;  // 260ms stereo width control
    const float BASE_DELAY_MS = 0.0f;  // 0ms neutral point (asymmetric modulation)

    // Calculate base delay (d0) in samples
    const float d0Samples = (BASE_DELAY_MS / 1000.0f) * static_cast<float>(spec.sampleRate);

    // Exponential smoothing coefficient: alpha = 1 - exp(-1 / (tau * fs))
    const float tau = 0.01f;  // 10ms time constant for smooth parameter changes
    const float alpha = 1.0f - std::exp(-1.0f / (tau * static_cast<float>(spec.sampleRate)));

    // Target doppler control (normalized to [-1, 1] range)
    const float targetDopplerControl = bypassDoppler ? 0.0f : (dopplerShift / 50.0f);

    // Process each sample through the series chain
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // ─────────────────────────────────────────────────────────────────────
        // PER-SAMPLE DOPPLER CONTROL SMOOTHING
        // ─────────────────────────────────────────────────────────────────────

        // Exponential smoothing: x(n) = x(n-1) + alpha * (target - x(n-1))
        currentDopplerControl += alpha * (targetDopplerControl - currentDopplerControl);

        // Calculate differential delay: deltaDelay = ITD_HALF * x(t)  where x ∈ [-1, 1]
        const float deltaDelaySamples = ITD_HALF * currentDopplerControl * static_cast<float>(spec.sampleRate);

        // Compute L/R delay times with opposite differential:
        // delayLeft(t)  = d0 + deltaDelay(t)
        // delayRight(t) = d0 - deltaDelay(t)
        // Safety clamping ensures delays never go below 1 sample
        const float leftDelaySamples = std::max(1.0f, d0Samples + deltaDelaySamples);
        const float rightDelaySamples = std::max(1.0f, d0Samples - deltaDelaySamples);

        // ─────────────────────────────────────────────────────────────────────
        // LFO MODULATION (Tape Wow & Flutter Simulation)
        // ─────────────────────────────────────────────────────────────────────

        // Calculate LFO frequency (tempo-synced or free running)
        float lfoFreq = lfoRate;  // Default to free mode (Hz)

        if (lfoTempoSync)
        {
            // Tempo-synced mode: Calculate frequency from host BPM
            auto posInfo = getPlayHead();
            if (posInfo != nullptr)
            {
                auto position = posInfo->getPosition();
                if (position.hasValue() && position->getBpm().hasValue())
                {
                    double bpm = *position->getBpm();
                    bpm = juce::jlimit(20.0, 300.0, bpm);  // Clamp to valid range

                    // Map lfoRate (0.1-10Hz) to musical divisions (proper tempo sync)
                    // lfoRate: 0.125 = 8 bars, 0.25 = 4 bars, 0.5 = 2 bars, 1.0 = 1 bar,
                    //          2.0 = 1/2 note, 4.0 = 1/4 note, 8.0 = 1/8 note, 10.0 = 1/16 note

                    // Calculate beats per second from BPM
                    float beatsPerSecond = static_cast<float>(bpm / 60.0);

                    // Map lfoRate to divisions
                    float division;
                    if (lfoRate <= 0.25f)      division = 16.0f;  // 4 bars (16 beats)
                    else if (lfoRate <= 0.5f)  division = 8.0f;   // 2 bars (8 beats)
                    else if (lfoRate <= 1.0f)  division = 4.0f;   // 1 bar (4 beats)
                    else if (lfoRate <= 2.0f)  division = 2.0f;   // Half note (2 beats)
                    else if (lfoRate <= 4.0f)  division = 1.0f;   // Quarter note (1 beat)
                    else if (lfoRate <= 6.0f)  division = 0.5f;   // Eighth note
                    else                       division = 0.25f;  // Sixteenth note

                    // Calculate LFO frequency: cycles per second = (beats per second) / (beats per cycle)
                    lfoFreq = beatsPerSecond / division;
                }
            }
        }

        // LFO Depth: User-controllable wow & flutter intensity (0-100% → 0-20% delay modulation)
        // Higher percentages = more aggressive pitch artifacts (tape-style)
        const float modDepth = (lfoDepthPercent / 100.0f) * 0.20f;  // Scale to 0-20% delay variation

        // Calculate LFO output (sine wave, range -1 to +1)
        const float lfoValue = std::sin(lfoPhase);

        // Apply LFO modulation to delay times (creates continuous pitch artifacts)
        const float leftDelayModulated = leftDelaySamples * (1.0f + lfoValue * modDepth);
        const float rightDelayModulated = rightDelaySamples * (1.0f + lfoValue * modDepth);

        // Update LFO phase for next sample
        const float phaseInc = (lfoFreq * 2.0f * juce::MathConstants<float>::pi) / static_cast<float>(spec.sampleRate);
        lfoPhase += phaseInc;
        if (lfoPhase >= 2.0f * juce::MathConstants<float>::pi)
            lfoPhase -= 2.0f * juce::MathConstants<float>::pi;

        // Set per-sample delay amounts with LFO modulation (fractional delay with linear interpolation)
        if (!bypassDoppler)
        {
            delayLineLeft.setDelay(std::max(1.0f, leftDelayModulated));
            delayLineRight.setDelay(std::max(1.0f, rightDelayModulated));
        }

        // ─────────────────────────────────────────────────────────────────────
        // STAGE 1: DOPPLER DELAY (with feedback)
        // ─────────────────────────────────────────────────────────────────────

        float leftIn = buffer.getSample(0, sample);
        float rightIn = buffer.getSample(1, sample);

        if (!bypassDoppler)
        {
            // Add filtered feedback to input
            float feedbackL = feedbackBuffer.getSample(0, sample) * feedbackGain;
            float feedbackR = feedbackBuffer.getSample(1, sample) * feedbackGain;
            leftIn += feedbackL;
            rightIn += feedbackR;

            // Push samples INTO delay line, then pop delayed samples OUT
            // CRITICAL: Push happens first, then pop (FIFO behavior)
            delayLineLeft.pushSample(0, leftIn);
            delayLineRight.pushSample(0, rightIn);

            // Pop delayed samples (uses delay set by setDelay() above)
            float delayedL = delayLineLeft.popSample(0);
            float delayedR = delayLineRight.popSample(0);

            // ─────────────────────────────────────────────────────────────────────
            // REVERSE PLAYBACK (Professional Grain-Based Implementation)
            // ─────────────────────────────────────────────────────────────────────

            if (reverse)
            {
                // Process through professional reverse delay with double-buffering and crossfading
                delayedL = reverseDelay.processSample(delayedL, 0);
                delayedR = reverseDelay.processSample(delayedR, 1);
            }

            leftIn = delayedL;
            rightIn = delayedR;

            // Store delayed signal for feedback (will be filtered below)
            feedbackBuffer.setSample(0, sample, leftIn);
            feedbackBuffer.setSample(1, sample, rightIn);
        }

        // ─────────────────────────────────────────────────────────────────────
        // STAGE 2: SATURATION
        // ─────────────────────────────────────────────────────────────────────

        if (!bypassSaturation)
        {
            // Tube saturation: output = tanh(gain * input)
            leftIn = std::tanh(saturationGain * leftIn);
            rightIn = std::tanh(saturationGain * rightIn);
        }

        // ─────────────────────────────────────────────────────────────────────
        // STAGE 3: MASTER OUTPUT
        // ─────────────────────────────────────────────────────────────────────

        buffer.setSample(0, sample, leftIn * masterOutputGain);
        buffer.setSample(1, sample, rightIn * masterOutputGain);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // POST-PROCESSING: Filter feedback buffer for next iteration
    // ─────────────────────────────────────────────────────────────────────────

    if (!bypassDoppler && feedbackGain > 0.001f)
    {
        juce::dsp::AudioBlock<float> feedbackBlock(feedbackBuffer);
        juce::dsp::ProcessContextReplacing<float> feedbackContext(feedbackBlock);

        // Apply lo-cut (high-pass) then hi-cut (low-pass) to feedback signal
        loCutFilter.process(feedbackContext);
        hiCutFilter.process(feedbackContext);
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
