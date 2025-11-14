#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout LushPadAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // timbre - Float (0.0 to 1.0, default: 0.35, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "timbre", 1 },
        "Timbre",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.35f
    ));

    // filter_cutoff - Float (20.0 to 20000.0 Hz, default: 2000.0, skew: 0.3 logarithmic)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "filter_cutoff", 1 },
        "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f),
        2000.0f,
        "Hz"
    ));

    // reverb_amount - Float (0.0 to 1.0, default: 0.4, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "reverb_amount", 1 },
        "Reverb Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.4f
    ));

    return layout;
}

LushPadAudioProcessor::LushPadAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

LushPadAudioProcessor::~LushPadAudioProcessor()
{
}

void LushPadAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Prepare DSP spec for filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;  // Mono per-voice filtering

    // Initialize all voices with filter preparation
    for (auto& voice : voices)
    {
        voice.adsr.setSampleRate(sampleRate);
        voice.filter.prepare(spec);
        voice.reset();
    }
}

void LushPadAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 3 (DSP)
}

void LushPadAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output buffer
    buffer.clear();

    // Handle MIDI events (sample-accurate timing)
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            float velocity = message.getVelocity() / 127.0f;
            allocateVoice(note, velocity);
        }
        else if (message.isNoteOff())
        {
            int note = message.getNoteNumber();
            releaseVoice(note);
        }
    }

    // Read parameters (atomic, done once per buffer for efficiency)
    float timbreValue = parameters.getRawParameterValue("timbre")->load();
    float filterCutoffValue = parameters.getRawParameterValue("filter_cutoff")->load();

    // Map timbre to FM feedback depth (0.0-0.4 range for musicality)
    float feedbackDepth = timbreValue * 0.4f;

    // Map timbre to saturation gain (1.0-3.0 range)
    float saturationGain = 1.0f + (timbreValue * 2.0f);

    // Generate audio per-sample
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mixL = 0.0f;
        float mixR = 0.0f;

        // Process all active voices
        for (auto& voice : voices)
        {
            if (!voice.active)
                continue;

            // Calculate base frequency for this MIDI note
            // f = 440 * 2^((note - 69) / 12)
            float baseFreq = 440.0f * std::pow(2.0f, (voice.currentNote - 69) / 12.0f);

            // Detuning ratios
            // +7 cents: 2^(7/1200) ≈ 1.00407
            // -7 cents: 2^(-7/1200) ≈ 0.99593
            float ratio1 = 1.0f;       // Base frequency
            float ratio2 = 1.00407f;   // +7 cents
            float ratio3 = 0.99593f;   // -7 cents

            // Generate 3 detuned sine oscillators WITH FM feedback
            // Formula: sin(phase + feedbackDepth * previousOutput)
            float osc1 = std::sin(voice.phase1 + feedbackDepth * voice.previousOutput1);
            float osc2 = std::sin(voice.phase2 + feedbackDepth * voice.previousOutput2);
            float osc3 = std::sin(voice.phase3 + feedbackDepth * voice.previousOutput3);

            // Store outputs for next sample's feedback
            voice.previousOutput1 = osc1;
            voice.previousOutput2 = osc2;
            voice.previousOutput3 = osc3;

            // Sum oscillators (average to prevent clipping)
            float voiceOutput = (osc1 + osc2 + osc3) / 3.0f;

            // Apply harmonic saturation using tanh waveshaping
            voiceOutput = std::tanh(saturationGain * voiceOutput);

            // Calculate velocity-scaled filter cutoff
            // Soft notes (low velocity): darker sound (cutoff reduced by 50%)
            // Hard notes (high velocity): brighter sound (cutoff at parameter value)
            float velocityScaledCutoff = filterCutoffValue * (0.5f + 0.5f * voice.currentVelocity);

            // Clamp to valid range
            velocityScaledCutoff = juce::jlimit(20.0f, 20000.0f, velocityScaledCutoff);

            // Update filter coefficients (12dB/octave low-pass, Q=0.35)
            auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
                currentSampleRate,
                velocityScaledCutoff,
                0.35f  // Fixed resonance
            );
            *voice.filter.coefficients = *coefficients;

            // Process through filter
            voiceOutput = voice.filter.processSample(voiceOutput);

            // Apply ADSR envelope
            float envelope = voice.adsr.getNextSample();
            voiceOutput *= envelope * voice.currentVelocity;

            // Add to stereo mix (centered for now, panning in Phase 3.3)
            mixL += voiceOutput;
            mixR += voiceOutput;

            // Update oscillator phases
            float phaseIncrement1 = (baseFreq * ratio1 * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);
            float phaseIncrement2 = (baseFreq * ratio2 * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);
            float phaseIncrement3 = (baseFreq * ratio3 * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);

            voice.phase1 += phaseIncrement1;
            voice.phase2 += phaseIncrement2;
            voice.phase3 += phaseIncrement3;

            // Wrap phases to [0, 2π] to prevent denormals
            while (voice.phase1 >= juce::MathConstants<float>::twoPi)
                voice.phase1 -= juce::MathConstants<float>::twoPi;
            while (voice.phase2 >= juce::MathConstants<float>::twoPi)
                voice.phase2 -= juce::MathConstants<float>::twoPi;
            while (voice.phase3 >= juce::MathConstants<float>::twoPi)
                voice.phase3 -= juce::MathConstants<float>::twoPi;

            // Mark voice inactive if envelope has finished
            if (!voice.adsr.isActive())
            {
                voice.active = false;
            }
        }

        // Write to output buffer (reduce gain to prevent clipping with 8 voices)
        buffer.setSample(0, sample, mixL * 0.3f);
        if (totalNumOutputChannels > 1)
        {
            buffer.setSample(1, sample, mixR * 0.3f);
        }
    }
}

juce::AudioProcessorEditor* LushPadAudioProcessor::createEditor()
{
    return new LushPadAudioProcessorEditor(*this);
}

void LushPadAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LushPadAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Voice allocation helper methods
void LushPadAudioProcessor::allocateVoice(int note, float velocity)
{
    // First, try to find a free voice
    for (auto& voice : voices)
    {
        if (!voice.active || !voice.adsr.isActive())
        {
            startVoice(voice, note, velocity);
            return;
        }
    }

    // All voices busy - steal the oldest voice
    SynthVoice* oldest = &voices[0];
    for (auto& voice : voices)
    {
        if (voice.timestamp < oldest->timestamp)
        {
            oldest = &voice;
        }
    }

    // Gracefully release stolen voice before reusing
    oldest->adsr.noteOff();
    startVoice(*oldest, note, velocity);
}

void LushPadAudioProcessor::releaseVoice(int note)
{
    for (auto& voice : voices)
    {
        if (voice.active && voice.currentNote == note)
        {
            voice.adsr.noteOff();
        }
    }
}

void LushPadAudioProcessor::startVoice(SynthVoice& voice, int note, float velocity)
{
    voice.active = true;
    voice.currentNote = note;
    voice.currentVelocity = velocity;
    voice.timestamp = voiceCounter++;
    voice.phase1 = voice.phase2 = voice.phase3 = 0.0f;

    // Fixed ADSR parameters (Phase 3.1: not parameter-controlled yet)
    voice.adsrParams.attack = 0.3f;   // 300ms attack
    voice.adsrParams.decay = 0.2f;    // 200ms decay
    voice.adsrParams.sustain = 0.8f;  // 80% sustain level
    voice.adsrParams.release = 2.0f;  // 2000ms release

    voice.adsr.setParameters(voice.adsrParams);
    voice.adsr.noteOn();
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LushPadAudioProcessor();
}
