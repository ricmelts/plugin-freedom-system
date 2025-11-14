#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout ScatterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // delay_time - Float (100.0 to 2000.0 ms, default: 500.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_time", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f, 1.0f),
        500.0f,
        "ms"
    ));

    // grain_size - Float (5.0 to 500.0 ms, default: 100.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "grain_size", 1 },
        "Grain Size",
        juce::NormalisableRange<float>(5.0f, 500.0f, 1.0f, 1.0f),
        100.0f,
        "ms"
    ));

    // density - Float (0.0 to 100.0 %, default: 50.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "density", 1 },
        "Density",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    // pitch_random - Float (0.0 to 100.0 %, default: 30.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "pitch_random", 1 },
        "Pitch Random",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        30.0f,
        "%"
    ));

    // scale - Choice (Chromatic, Major, Minor, Pentatonic, Blues)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "scale", 1 },
        "Scale",
        juce::StringArray { "Chromatic", "Major", "Minor", "Pentatonic", "Blues" },
        0
    ));

    // root_note - Choice (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "root_note", 1 },
        "Root Note",
        juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
        0
    ));

    // pan_random - Float (0.0 to 100.0 %, default: 75.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "pan_random", 1 },
        "Pan Random",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        75.0f,
        "%"
    ));

    // feedback - Float (0.0 to 100.0 %, default: 30.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        30.0f,
        "%"
    ));

    // mix - Float (0.0 to 100.0 %, default: 50.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    return layout;
}

ScatterAudioProcessor::ScatterAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Phase 3.2: Initialize scale lookup tables
    initializeScaleTables();
}

ScatterAudioProcessor::~ScatterAudioProcessor()
{
}

void ScatterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Store sample rate for grain size calculations
    currentSampleRate = sampleRate;

    // Prepare DSP spec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Prepare delay buffer (size for maximum delay time: 2000ms)
    auto maxDelayTimeSamples = static_cast<int>(sampleRate * 2.0);  // 2 seconds max
    currentDelayBufferSize = maxDelayTimeSamples;
    delayBuffer.setMaximumDelayInSamples(maxDelayTimeSamples);
    delayBuffer.prepare(spec);
    delayBuffer.reset();

    // Initialize grain scheduler
    grainSpawnCounter = 0;
    lastGrainSpawnInterval = 0;

    // Clear all grain voices
    for (auto& grain : grainVoices)
    {
        grain.active = false;
        grain.readPosition = 0.0f;
        grain.windowPosition = 0.0f;
        grain.grainSizeSamples = 0;
    }
}

void ScatterAudioProcessor::releaseResources()
{
}

void ScatterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused output channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters (atomic, real-time safe)
    auto* delayTimeParam = parameters.getRawParameterValue("delay_time");
    auto* grainSizeParam = parameters.getRawParameterValue("grain_size");
    auto* densityParam = parameters.getRawParameterValue("density");
    auto* pitchRandomParam = parameters.getRawParameterValue("pitch_random");
    auto* scaleParam = parameters.getRawParameterValue("scale");
    auto* rootNoteParam = parameters.getRawParameterValue("root_note");

    float delayTimeMs = delayTimeParam->load();
    float grainSizeMs = grainSizeParam->load();
    float densityPercent = densityParam->load();
    float pitchRandomPercent = pitchRandomParam->load();
    int scaleIndex = static_cast<int>(scaleParam->load());
    int rootNote = static_cast<int>(rootNoteParam->load());

    const int numSamples = buffer.getNumSamples();

    // Phase 3.1: Mono processing for testing (process channel 0 only)
    auto* channelData = buffer.getWritePointer(0);

    // Step 1: Write input to delay buffer
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Write current input to delay buffer
        delayBuffer.pushSample(0, channelData[sample]);
    }

    // Step 2: Update grain scheduler and spawn grains (Phase 3.2: Pass pitch parameters)
    updateGrainScheduler(densityPercent, grainSizeMs, pitchRandomPercent, scaleIndex, rootNote);

    // Step 3: Process active grain voices and write to output
    processGrainVoices(buffer);
}

juce::AudioProcessorEditor* ScatterAudioProcessor::createEditor()
{
    return new ScatterAudioProcessorEditor(*this);
}

void ScatterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ScatterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScatterAudioProcessor();
}

// ============================================================================
// Phase 3.1: Core Granular Engine Helper Methods
// ============================================================================

void ScatterAudioProcessor::generateHannWindow(int sizeInSamples)
{
    hannWindow.resize(sizeInSamples);
    windowTableSize = sizeInSamples;

    // Generate Hann window: hann[n] = 0.5 * (1 - cos(2 * pi * n / N))
    juce::dsp::WindowingFunction<float>::fillWindowingTables(
        hannWindow.data(),
        sizeInSamples,
        juce::dsp::WindowingFunction<float>::hann,
        false  // Not normalized (we want 0-1 range)
    );
}

void ScatterAudioProcessor::spawnNewGrain(float grainSizeMs, float pitchRandomPercent, int scaleIndex, int rootNote)
{
    // Convert grain size from ms to samples
    int grainSizeSamples = static_cast<int>(currentSampleRate * grainSizeMs / 1000.0f);

    // Clamp to valid range (avoid zero or negative sizes)
    grainSizeSamples = juce::jmax(1, grainSizeSamples);

    // Find inactive voice (voice allocation)
    GrainVoice* availableVoice = nullptr;

    for (auto& grain : grainVoices)
    {
        if (!grain.active)
        {
            availableVoice = &grain;
            break;
        }
    }

    // If no voices available, steal the first voice (simple voice stealing)
    if (availableVoice == nullptr)
    {
        availableVoice = &grainVoices[0];
    }

    // Phase 3.2: Generate random pitch and quantize to scale
    auto& random = juce::Random::getSystemRandom();

    // Generate random pitch: -7 to +7 semitones, scaled by pitch_random parameter
    float randomPitch = (random.nextFloat() * 2.0f - 1.0f) * 7.0f * (pitchRandomPercent / 100.0f);

    // Quantize to selected scale with root note transposition
    int quantizedPitch = quantizePitchToScale(randomPitch, scaleIndex, rootNote);

    // Calculate playback rate: rate = 2^(semitones / 12)
    float playbackRate = std::pow(2.0f, quantizedPitch / 12.0f);

    // Initialize grain voice
    availableVoice->active = true;
    availableVoice->grainSizeSamples = grainSizeSamples;
    availableVoice->windowPosition = 0.0f;
    availableVoice->playbackRate = playbackRate;  // Phase 3.2: Store playback rate

    // Read position: Start at current delay buffer write position
    // Phase 3.1: No delay time offset (read from current position)
    availableVoice->readPosition = 0.0f;

    // Generate Hann window for this grain size (if not already cached)
    if (windowTableSize != grainSizeSamples)
    {
        generateHannWindow(grainSizeSamples);
    }
}

void ScatterAudioProcessor::updateGrainScheduler(float densityPercent, float grainSizeMs, float pitchRandomPercent, int scaleIndex, int rootNote)
{
    // Grain spawn interval calculation: grainSizeSamples / (density * overlapFactor)
    // At 50% density, grains spawn at ~grainSize intervals (moderate overlap)
    // At 100% density, grains spawn more frequently (dense cloud)

    const float overlapFactor = 2.0f;  // Tuning constant for overlap behavior
    int grainSizeSamples = static_cast<int>(currentSampleRate * grainSizeMs / 1000.0f);
    grainSizeSamples = juce::jmax(1, grainSizeSamples);

    // Calculate spawn interval (avoid division by zero)
    float densityNormalized = juce::jmax(0.01f, densityPercent / 100.0f);
    int spawnInterval = static_cast<int>(grainSizeSamples / (densityNormalized * overlapFactor));
    spawnInterval = juce::jmax(1, spawnInterval);  // At least 1 sample

    lastGrainSpawnInterval = spawnInterval;

    // Increment sample counter
    grainSpawnCounter++;

    // Check if it's time to spawn a new grain
    if (grainSpawnCounter >= spawnInterval)
    {
        spawnNewGrain(grainSizeMs, pitchRandomPercent, scaleIndex, rootNote);
        grainSpawnCounter = 0;  // Reset counter
    }
}

void ScatterAudioProcessor::processGrainVoices(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();

    // Phase 3.1: Clear output buffer (grains will be summed into it)
    buffer.clear();

    // Process each active grain voice
    for (auto& grain : grainVoices)
    {
        if (!grain.active)
            continue;

        // For each sample in the buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Check if grain has completed
            if (grain.windowPosition >= 1.0f)
            {
                grain.active = false;
                break;
            }

            // Calculate window index (map 0.0-1.0 to 0..grainSizeSamples-1)
            int windowIndex = static_cast<int>(grain.windowPosition * grain.grainSizeSamples);
            windowIndex = juce::jlimit(0, grain.grainSizeSamples - 1, windowIndex);

            // Get window envelope value (or 1.0 if table not generated yet)
            float windowValue = 1.0f;
            if (windowIndex < static_cast<int>(hannWindow.size()))
            {
                windowValue = hannWindow[windowIndex];
            }

            // Read from delay buffer (interpolated)
            // Phase 3.1: Read at grain's read position (no delay offset yet)
            float delaySamples = grain.readPosition;
            float delayedSample = delayBuffer.popSample(0, delaySamples);

            // Apply window envelope
            float grainOutput = delayedSample * windowValue;

            // Sum to output buffer (mono for Phase 3.1)
            auto* outputData = buffer.getWritePointer(0);
            outputData[sample] += grainOutput;

            // Advance grain window position (always at rate 1.0 - envelope progresses normally)
            grain.windowPosition += 1.0f / grain.grainSizeSamples;

            // Phase 3.2: Advance read position by playback rate (pitch shift)
            // Forward playback only (Phase 3.3 will add reverse)
            grain.readPosition += grain.playbackRate;

            // Wrap read position if it exceeds delay buffer size
            if (grain.readPosition >= currentDelayBufferSize)
            {
                grain.readPosition = 0.0f;
            }
        }
    }
}
