#include "OmnariaVoice.h"
#include <cmath>

namespace omnaria
{
OmnariaVoice::OmnariaVoice(juce::AudioProcessorValueTreeState& parameters, const OmnariaState& sharedState)
    : params(parameters), state(sharedState)
{
    configureFilterTypes(1);
}

bool OmnariaVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<OmnariaSound*>(sound) != nullptr;
}

void OmnariaVoice::prepare(double sampleRate, int maximumBlockSize)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    for (auto& osc : oscillatorA) osc.prepare(currentSampleRate);
    for (auto& osc : oscillatorB) osc.prepare(currentSampleRate);
    subOscillator.prepare(currentSampleRate);
    subOscillator.setShape(BandlimitedOscillator::Shape::sine);

    juce::dsp::ProcessSpec spec { currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, maximumBlockSize)), 2 };
    filterA.prepare(spec);
    filterB.prepare(spec);
    filterA.reset();
    filterB.reset();
    ampEnvelope.setSampleRate(currentSampleRate);
    filterEnvelope.setSampleRate(currentSampleRate);

    smoothedCutoff.reset(currentSampleRate, 0.018);
    smoothedResonance.reset(currentSampleRate, 0.018);
    smoothedDrive.reset(currentSampleRate, 0.012);
    smoothedMix.reset(currentSampleRate, 0.012);
    smoothedSpread.reset(currentSampleRate, 0.012);
    smoothedCutoff.setCurrentAndTargetValue(7200.0f);
    smoothedResonance.setCurrentAndTargetValue(0.72f);
    smoothedDrive.setCurrentAndTargetValue(1.5f);
    smoothedMix.setCurrentAndTargetValue(0.28f);
    smoothedSpread.setCurrentAndTargetValue(0.82f);
}

void OmnariaVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentMidiNote = midiNoteNumber;
    pitchWheel = currentPitchWheelPosition;
    noteVelocity = velocity;

    const bool randomPhase = parameter("phase_mode") >= 0.5f;
    const auto requestedPhase = juce::jlimit(0.0f, 1.0f, parameter("phase"));

    for (int i = 0; i < maxUnison; ++i)
    {
        const auto phaseA = randomPhase
            ? noiseRandom.nextDouble()
            : std::fmod(static_cast<double>(requestedPhase) + 0.137 * static_cast<double>(i), 1.0);
        const auto phaseB = randomPhase
            ? noiseRandom.nextDouble()
            : std::fmod(static_cast<double>(requestedPhase) + 0.370 + 0.193 * static_cast<double>(i), 1.0);
        oscillatorA[static_cast<size_t>(i)].reset(phaseA);
        oscillatorB[static_cast<size_t>(i)].reset(phaseB);
    }

    subOscillator.reset(randomPhase ? noiseRandom.nextDouble() : requestedPhase);
    filterA.reset();
    filterB.reset();
    ampEnvelope.noteOn();
    filterEnvelope.noteOn();
}

void OmnariaVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnvelope.noteOff();
        filterEnvelope.noteOff();
    }
    else
    {
        ampEnvelope.reset();
        filterEnvelope.reset();
        clearCurrentNote();
    }
}

void OmnariaVoice::pitchWheelMoved(int newPitchWheelValue)
{
    pitchWheel = newPitchWheelValue;
}

void OmnariaVoice::controllerMoved(int, int)
{
}

void OmnariaVoice::configureFilterTypes(int mode)
{
    activeFilterMode = juce::jlimit(0, 3, mode);
    auto type = juce::dsp::StateVariableTPTFilterType::lowpass;
    if (activeFilterMode == 2) type = juce::dsp::StateVariableTPTFilterType::highpass;
    if (activeFilterMode == 3) type = juce::dsp::StateVariableTPTFilterType::bandpass;
    filterA.setType(type);
    filterB.setType(type);
}

void OmnariaVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive()) return;

    const auto shapeA = shapeForIndex(static_cast<int>(std::round(parameter("oscA_shape"))));
    const auto shapeB = shapeForIndex(static_cast<int>(std::round(parameter("oscB_shape"))));
    const auto coarseB = parameter("oscB_coarse");
    const auto pulseWidth = juce::jlimit(0.05f, 0.95f, parameter("pulse_width"));
    const auto unisonCount = juce::jlimit(1, maxUnison, static_cast<int>(std::round(parameter("unison"))));
    const auto detuneCents = parameter("detune");
    const auto subLevel = juce::jlimit(0.0f, 1.0f, parameter("sub_level"));
    const auto subOctave = juce::jlimit(-2, 0, juce::roundToInt(parameter("sub_octave")));
    const auto noiseLevel = juce::jlimit(0.0f, 0.5f, parameter("noise_level"));
    const auto filterMode = juce::jlimit(0, 3, juce::roundToInt(parameter("filter_mode")));
    if (filterMode != activeFilterMode) configureFilterTypes(filterMode);

    juce::ADSR::Parameters ampParams;
    ampParams.attack = parameter("attack");
    ampParams.decay = parameter("decay");
    ampParams.sustain = parameter("sustain");
    ampParams.release = parameter("release");
    ampEnvelope.setParameters(ampParams);

    juce::ADSR::Parameters filterParams;
    filterParams.attack = parameter("filter_attack");
    filterParams.decay = parameter("filter_decay");
    filterParams.sustain = parameter("filter_sustain");
    filterParams.release = parameter("filter_release");
    filterEnvelope.setParameters(filterParams);

    const auto motion = state.motion.load();
    const auto energy = state.performanceEnergy.load();
    const auto history = state.historyState.load();
    const auto phrase = state.phrasePosition.load();

    const auto baseHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(currentMidiNote)) * pitchWheelRatio();
    const auto bRatio = std::pow(2.0f, coarseB / 12.0f);
    const auto subRatio = std::pow(2.0f, static_cast<float>(subOctave));
    subOscillator.setFrequency(baseHz * subRatio);

    auto baseCutoff = parameter("cutoff");
    const auto keytrack = juce::jlimit(0.0f, 1.0f, parameter("keytrack"));
    baseCutoff *= std::pow(2.0f, (static_cast<float>(currentMidiNote) - 60.0f) / 12.0f * keytrack);
    if (motion > 0.0f)
    {
        const auto phraseArc = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phrase);
        const auto movementOctaves = motion * (0.55f * energy + 0.35f * phraseArc + 0.20f * history);
        baseCutoff *= std::pow(2.0f, movementOctaves);
    }

    smoothedCutoff.setTargetValue(juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), baseCutoff));
    smoothedResonance.setTargetValue(juce::jlimit(0.2f, 12.0f, parameter("resonance")));
    smoothedDrive.setTargetValue(juce::jmax(0.0f, parameter("drive")));
    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("osc_mix")));
    smoothedSpread.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("spread")));

    for (int i = 0; i < unisonCount; ++i)
    {
        const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
        const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
        const auto drift = motion * history * 1.5f * std::sin((static_cast<float>(currentMidiNote) + i * 7.0f) * 0.37f);
        const auto cents = position * detuneCents + drift;
        const auto detuneRatio = std::pow(2.0f, cents / 1200.0f);

        auto& a = oscillatorA[static_cast<size_t>(i)];
        auto& b = oscillatorB[static_cast<size_t>(i)];
        a.setShape(shapeA);
        b.setShape(shapeB);
        a.setPulseWidth(pulseWidth);
        b.setPulseWidth(pulseWidth);
        a.setFrequency(baseHz * detuneRatio);
        b.setFrequency(baseHz * bRatio * detuneRatio);
    }

    const auto filterEnvAmount = parameter("filter_env_amt");
    const auto velocityTimbre = juce::jlimit(0.0f, 1.0f, parameter("velocity_timbre"));
    const auto velocityScale = juce::jlimit(0.25f, 1.75f, 1.0f + velocityTimbre * (noteVelocity - 0.5f) * 1.4f);
    const auto numChannels = outputBuffer.getNumChannels();

    // Correlated unison voices are normalised more strongly at low detune, then
    // relaxed as detune increases. This protects centre weight and patch level.
    const auto decorrelation = juce::jlimit(0.0f, 1.0f, detuneCents / 35.0f);
    const auto exponent = juce::jmap(decorrelation, 0.88f, 0.58f);
    const auto unisonNormalisation = 0.90f / std::pow(static_cast<float>(unisonCount), exponent);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float left = 0.0f;
        float right = 0.0f;
        const auto mix = smoothedMix.getNextValue();
        const auto spread = smoothedSpread.getNextValue();

        for (int i = 0; i < unisonCount; ++i)
        {
            const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
            const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
            const auto pan = juce::jlimit(-1.0f, 1.0f, position * spread);
            const auto leftGain = std::sqrt(0.5f * (1.0f - pan));
            const auto rightGain = std::sqrt(0.5f * (1.0f + pan));
            const auto a = oscillatorA[static_cast<size_t>(i)].process();
            const auto b = oscillatorB[static_cast<size_t>(i)].process();
            const auto voiceSample = juce::jmap(mix, a, b) * unisonNormalisation;
            left += voiceSample * leftGain;
            right += voiceSample * rightGain;
        }

        const auto centredSub = subOscillator.process() * subLevel * 0.55f;
        const auto noise = (noiseRandom.nextFloat() * 2.0f - 1.0f) * noiseLevel * 0.30f;
        left += centredSub + noise;
        right += centredSub + noise;

        const auto amp = ampEnvelope.getNextSample() * noteVelocity;
        const auto filterEnv = filterEnvelope.getNextSample();
        left *= amp;
        right *= amp;

        const auto driveDb = smoothedDrive.getNextValue();
        if (driveDb > 0.01f)
        {
            const auto expressiveDrive = 1.0f + velocityTimbre * noteVelocity * 0.35f;
            const auto driveGain = juce::Decibels::decibelsToGain(driveDb) * expressiveDrive;
            const auto norm = 1.0f / juce::jmax(0.25f, std::tanh(driveGain));
            left = std::tanh(left * driveGain) * norm;
            right = std::tanh(right * driveGain) * norm;
        }

        const auto envelopeOctaves = filterEnvAmount * filterEnv * velocityScale;
        const auto cutoff = juce::jlimit(20.0f,
                                         static_cast<float>(currentSampleRate * 0.45),
                                         smoothedCutoff.getNextValue() * std::pow(2.0f, envelopeOctaves));
        const auto resonance = smoothedResonance.getNextValue();
        filterA.setCutoffFrequency(cutoff);
        filterA.setResonance(resonance);
        filterB.setCutoffFrequency(cutoff);
        filterB.setResonance(resonance);

        left = filterA.processSample(0, left);
        right = filterA.processSample(1, right);
        if (filterMode == 1)
        {
            left = filterB.processSample(0, left);
            right = filterB.processSample(1, right);
        }

        const auto targetSample = startSample + sample;
        if (numChannels > 0) outputBuffer.addSample(0, targetSample, left);
        if (numChannels > 1) outputBuffer.addSample(1, targetSample, right);
    }

    if (! ampEnvelope.isActive()) clearCurrentNote();
}

float OmnariaVoice::parameter(const char* parameterID) const noexcept
{
    if (const auto* value = params.getRawParameterValue(parameterID)) return value->load();
    return 0.0f;
}

BandlimitedOscillator::Shape OmnariaVoice::shapeForIndex(int index) const noexcept
{
    switch (index)
    {
        case 1: return BandlimitedOscillator::Shape::pulse;
        case 2: return BandlimitedOscillator::Shape::sine;
        default: return BandlimitedOscillator::Shape::saw;
    }
}

float OmnariaVoice::pitchWheelRatio() const noexcept
{
    const auto normalised = (static_cast<float>(pitchWheel) - 8192.0f) / 8192.0f;
    return std::pow(2.0f, (normalised * 2.0f) / 12.0f);
}
} // namespace omnaria
