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
    nastyCell.prepare(currentSampleRate);

    juce::dsp::ProcessSpec spec { currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, maximumBlockSize)), 2 };
    filterA.prepare(spec);
    filterB.prepare(spec);
    filterA.reset();
    filterB.reset();
    ampEnvelope.setSampleRate(currentSampleRate);
    filterEnvelope.setSampleRate(currentSampleRate);
    for (auto& envelope : auxEnvelopes) envelope.setSampleRate(currentSampleRate);

    smoothedCutoff.reset(currentSampleRate, 0.018);
    smoothedResonance.reset(currentSampleRate, 0.018);
    smoothedDrive.reset(currentSampleRate, 0.012);
    smoothedMix.reset(currentSampleRate, 0.012);
    smoothedSpread.reset(currentSampleRate, 0.012);
    smoothedCutoff.setCurrentAndTargetValue(7200.0f);
    smoothedResonance.setCurrentAndTargetValue(1.30f);
    smoothedDrive.setCurrentAndTargetValue(1.5f);
    smoothedMix.setCurrentAndTargetValue(0.28f);
    smoothedSpread.setCurrentAndTargetValue(0.82f);

    brownState = stochasticState = stochasticTarget = 0.0f;
    stochasticSamplesUntilTarget = 1;
    momentPhase = 0.0f;
}

void OmnariaVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentMidiNote = midiNoteNumber;
    pitchWheel = currentPitchWheelPosition;
    noteVelocity = velocity;
    momentPhase = 0.0f;
    nastyCell.reset();

    const bool randomPhase = parameter("phase_mode") >= 0.5f;
    const auto requestedPhase = juce::jlimit(0.0f, 1.0f, parameter("phase"));
    for (int i = 0; i < maxUnison; ++i)
    {
        const auto phaseA = randomPhase ? noiseRandom.nextDouble()
                                        : std::fmod(static_cast<double>(requestedPhase) + 0.137 * static_cast<double>(i), 1.0);
        const auto phaseB = randomPhase ? noiseRandom.nextDouble()
                                        : std::fmod(static_cast<double>(requestedPhase) + 0.370 + 0.193 * static_cast<double>(i), 1.0);
        oscillatorA[static_cast<size_t>(i)].reset(phaseA);
        oscillatorB[static_cast<size_t>(i)].reset(phaseB);
    }

    for (int i = 0; i < lfoCount; ++i)
    {
        const auto mode = juce::roundToInt(parameter("lfo" + juce::String(i + 1) + "_mode"));
        if (mode != 0) lfoPhase[static_cast<size_t>(i)] = 0.0;
        lfoOneShotComplete[static_cast<size_t>(i)] = false;
    }

    subOscillator.reset(randomPhase ? noiseRandom.nextDouble() : requestedPhase);
    filterA.reset();
    filterB.reset();
    previousDriveInputL = previousDriveInputR = 0.0f;
    ampEnvelope.noteOn();
    filterEnvelope.noteOn();
    configureAuxEnvelopes();
    for (auto& envelope : auxEnvelopes) envelope.noteOn();
}

void OmnariaVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnvelope.noteOff();
        filterEnvelope.noteOff();
        for (auto& envelope : auxEnvelopes) envelope.noteOff();
    }
    else
    {
        ampEnvelope.reset();
        filterEnvelope.reset();
        for (auto& envelope : auxEnvelopes) envelope.reset();
        nastyCell.reset();
        clearCurrentNote();
    }
}

void OmnariaVoice::pitchWheelMoved(int newPitchWheelValue) { pitchWheel = newPitchWheelValue; }

void OmnariaVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    if (controllerNumber == 1)
        modWheel = juce::jlimit(0.0f, 1.0f, static_cast<float>(newControllerValue) / 127.0f);
}

void OmnariaVoice::aftertouchChanged(int newAftertouchValue)
{
    aftertouch = juce::jlimit(0.0f, 1.0f, static_cast<float>(newAftertouchValue) / 127.0f);
}

void OmnariaVoice::channelPressureChanged(int newChannelPressureValue)
{
    aftertouch = juce::jlimit(0.0f, 1.0f, static_cast<float>(newChannelPressureValue) / 127.0f);
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

float OmnariaVoice::antialiasedTanh(float x, float& previousX) noexcept
{
    const auto delta = x - previousX;
    float result = 0.0f;
    if (std::abs(delta) < 1.0e-4f)
        result = std::tanh(0.5f * (x + previousX));
    else
        result = (std::log(std::cosh(x)) - std::log(std::cosh(previousX))) / delta;
    previousX = x;
    return result;
}

void OmnariaVoice::configureAuxEnvelopes()
{
    for (int i = 0; i < auxEnvelopeCount; ++i)
    {
        const auto prefix = "env" + juce::String(i + 1) + "_";
        juce::ADSR::Parameters p { parameter(prefix + "attack"), parameter(prefix + "decay"),
                                  parameter(prefix + "sustain"), parameter(prefix + "release") };
        auxEnvelopes[static_cast<size_t>(i)].setParameters(p);
    }
}

void OmnariaVoice::advanceLfos()
{
    const auto bpm = juce::jmax(20.0f, state.bpm.load());
    constexpr float beatMultipliers[] { 0.0f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
    for (int i = 0; i < lfoCount; ++i)
    {
        const auto suffix = juce::String(i + 1);
        const auto mode = juce::roundToInt(parameter("lfo" + suffix + "_mode"));
        const auto syncIndex = juce::jlimit(0, 7, juce::roundToInt(parameter("lfo" + suffix + "_sync")));
        auto rateHz = juce::jmax(0.01f, parameter("lfo" + suffix + "_rate"));
        if (syncIndex > 0) rateHz = (bpm / 60.0f) * beatMultipliers[syncIndex];

        auto& phase = lfoPhase[static_cast<size_t>(i)];
        auto& complete = lfoOneShotComplete[static_cast<size_t>(i)];
        if (! complete)
        {
            phase += static_cast<double>(rateHz) / currentSampleRate;
            if (phase >= 1.0)
            {
                if (mode == 2) { phase = 1.0; complete = true; }
                else phase -= std::floor(phase);
            }
        }
        lfoValues[static_cast<size_t>(i)] = std::sin(juce::MathConstants<float>::twoPi * static_cast<float>(phase));
    }
}

void OmnariaVoice::advanceStochasticSources()
{
    const auto brownStep = (noiseRandom.nextFloat() * 2.0f - 1.0f) * 0.0060f;
    brownState = juce::jlimit(-1.0f, 1.0f, brownState * 0.9995f + brownStep);

    if (--stochasticSamplesUntilTarget <= 0)
    {
        stochasticTarget = noiseRandom.nextFloat() * 2.0f - 1.0f;
        const auto duration = 0.06f + noiseRandom.nextFloat() * 0.24f;
        stochasticSamplesUntilTarget = juce::jmax(1, static_cast<int>(duration * currentSampleRate));
    }
    const auto smoothing = 1.0f - std::exp(-1.0f / static_cast<float>(0.045 * currentSampleRate));
    stochasticState += (stochasticTarget - stochasticState) * smoothing;
}

float OmnariaVoice::modulationSourceValue(int sourceIndex) const noexcept
{
    if (sourceIndex >= 1 && sourceIndex <= 4) return lfoValues[static_cast<size_t>(sourceIndex - 1)];
    if (sourceIndex >= 5 && sourceIndex <= 7) return auxEnvelopeValues[static_cast<size_t>(sourceIndex - 5)];
    switch (sourceIndex)
    {
        case 8: return noteVelocity;
        case 9: return juce::jlimit(0.0f, 1.0f, static_cast<float>(currentMidiNote) / 127.0f);
        case 10: return modWheel;
        case 11: return aftertouch;
        case 12: return parameter("macro1");
        case 13: return parameter("macro2");
        case 14: return parameter("macro3");
        case 15: return parameter("macro4");
        case 16: return brownState;
        case 17: return stochasticState;
        default: return 0.0f;
    }
}

OmnariaVoice::ModFrame OmnariaVoice::buildModFrame() const noexcept
{
    ModFrame frame;
    for (int i = 0; i < modSlotCount; ++i)
    {
        const auto suffix = juce::String(i + 1);
        const auto source = juce::roundToInt(parameter("mod" + suffix + "_source"));
        const auto destination = juce::roundToInt(parameter("mod" + suffix + "_dest"));
        const auto amount = modulationSourceValue(source) * juce::jlimit(-1.0f, 1.0f, parameter("mod" + suffix + "_depth"));
        switch (destination)
        {
            case 1: frame.pitchSemitones += amount * 12.0f; break;
            case 2: frame.cutoffOctaves += amount * 4.0f; break;
            case 3: frame.resonanceOffset += amount * 4.0f; break;
            case 4: frame.mixOffset += amount * 0.50f; break;
            case 5: frame.detuneCents += amount * 18.0f; break;
            case 6: frame.spreadOffset += amount * 0.50f; break;
            case 7: frame.driveDb += amount * 12.0f; break;
            case 8: frame.pulseWidthOffset += amount * 0.35f; break;
            case 9: frame.nastyAmount += amount; break;
            case 10: frame.nastyDeform += amount; break;
            case 11: frame.nastyFeedback += amount; break;
            case 12: frame.nastyCoupling += amount; break;
            case 13: frame.nastyEnergy += amount; break;
            case 14: frame.nastyDamping += amount; break;
            case 15: frame.nastyMoment += amount; break;
            default: break;
        }
    }
    return frame;
}

float OmnariaVoice::momentEnvelope(float amount) noexcept
{
    amount = juce::jlimit(0.0f, 1.0f, amount);
    if (amount <= 0.0001f) return 0.0f;

    // A one-shot record-event trajectory: tension -> peak -> recovery.
    const auto durationSeconds = juce::jmap(amount, 1.8f, 0.55f);
    momentPhase = juce::jmin(1.0f, momentPhase + 1.0f / static_cast<float>(currentSampleRate * durationSeconds));
    const auto p = momentPhase;
    float shape = 0.0f;
    if (p < 0.30f)
    {
        const auto t = p / 0.30f;
        shape = t * t * (3.0f - 2.0f * t);
    }
    else if (p < 0.58f)
        shape = 1.0f;
    else
    {
        const auto t = (p - 0.58f) / 0.42f;
        shape = 1.0f - t * t * (3.0f - 2.0f * t);
    }
    return amount * shape;
}

void OmnariaVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive()) return;

    const auto shapeA = shapeForIndex(juce::roundToInt(parameter("oscA_shape")));
    const auto shapeB = shapeForIndex(juce::roundToInt(parameter("oscB_shape")));
    const auto coarseB = parameter("oscB_coarse");
    const auto basePulseWidth = juce::jlimit(0.05f, 0.95f, parameter("pulse_width"));
    const auto unisonCount = juce::jlimit(1, maxUnison, juce::roundToInt(parameter("unison")));
    const auto baseDetuneCents = parameter("detune");
    const auto subLevel = juce::jlimit(0.0f, 1.0f, parameter("sub_level"));
    const auto subOctave = juce::jlimit(-2, 0, juce::roundToInt(parameter("sub_octave")));
    const auto noiseLevel = juce::jlimit(0.0f, 0.5f, parameter("noise_level"));
    const auto filterMode = juce::jlimit(0, 3, juce::roundToInt(parameter("filter_mode")));
    if (filterMode != activeFilterMode) configureFilterTypes(filterMode);

    ampEnvelope.setParameters({ parameter("attack"), parameter("decay"), parameter("sustain"), parameter("release") });
    filterEnvelope.setParameters({ parameter("filter_attack"), parameter("filter_decay"), parameter("filter_sustain"), parameter("filter_release") });
    configureAuxEnvelopes();

    const auto motion = state.motion.load();
    const auto energy = state.performanceEnergy.load();
    const auto history = state.historyState.load();
    const auto phrase = state.phrasePosition.load();
    const auto baseHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(currentMidiNote)) * pitchWheelRatio();
    const auto bRatio = std::pow(2.0f, coarseB / 12.0f);
    subOscillator.setFrequency(baseHz * std::pow(2.0f, static_cast<float>(subOctave)));

    auto baseCutoff = parameter("cutoff");
    const auto keytrack = juce::jlimit(0.0f, 1.0f, parameter("keytrack"));
    baseCutoff *= std::pow(2.0f, (static_cast<float>(currentMidiNote) - 60.0f) / 12.0f * keytrack);
    if (motion > 0.0f)
    {
        const auto phraseArc = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phrase);
        baseCutoff *= std::pow(2.0f, motion * (0.55f * energy + 0.35f * phraseArc + 0.20f * history));
    }
    smoothedCutoff.setTargetValue(juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), baseCutoff));

    const auto publicResonance = juce::jlimit(0.2f, 12.0f, parameter("resonance"));
    const auto resonance01 = (publicResonance - 0.2f) / 11.8f;
    smoothedResonance.setTargetValue(juce::jmap(resonance01, 1.35f, 0.08f));
    smoothedDrive.setTargetValue(juce::jmax(0.0f, parameter("drive")));
    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("osc_mix")));
    smoothedSpread.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("spread")));
    for (int i = 0; i < unisonCount; ++i)
    {
        oscillatorA[static_cast<size_t>(i)].setShape(shapeA);
        oscillatorB[static_cast<size_t>(i)].setShape(shapeB);
    }

    const auto filterEnvAmount = parameter("filter_env_amt");
    const auto velocityTimbre = juce::jlimit(0.0f, 1.0f, parameter("velocity_timbre"));
    const auto velocityScale = juce::jlimit(0.25f, 1.75f, 1.0f + velocityTimbre * (noteVelocity - 0.5f) * 1.4f);
    const auto numChannels = outputBuffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        advanceLfos();
        advanceStochasticSources();
        for (int i = 0; i < auxEnvelopeCount; ++i)
            auxEnvelopeValues[static_cast<size_t>(i)] = auxEnvelopes[static_cast<size_t>(i)].getNextSample();
        const auto frame = buildModFrame();

        const auto detuneCents = juce::jmax(0.0f, baseDetuneCents + frame.detuneCents);
        const auto decorrelation = juce::jlimit(0.0f, 1.0f, detuneCents / 35.0f);
        const auto exponent = juce::jmap(decorrelation, 0.88f, 0.58f);
        const auto unisonNormalisation = 0.90f / std::pow(static_cast<float>(unisonCount), exponent);

        if ((sample & 7) == 0)
        {
            const auto pitchRatio = std::pow(2.0f, frame.pitchSemitones / 12.0f);
            const auto pulseWidth = juce::jlimit(0.05f, 0.95f, basePulseWidth + frame.pulseWidthOffset);
            for (int i = 0; i < unisonCount; ++i)
            {
                const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
                const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
                const auto drift = motion * history * 1.5f * std::sin((static_cast<float>(currentMidiNote) + i * 7.0f) * 0.37f);
                const auto detuneRatio = std::pow(2.0f, (position * detuneCents + drift) / 1200.0f);
                auto& a = oscillatorA[static_cast<size_t>(i)];
                auto& b = oscillatorB[static_cast<size_t>(i)];
                a.setPulseWidth(pulseWidth); b.setPulseWidth(pulseWidth);
                a.setFrequency(baseHz * pitchRatio * detuneRatio);
                b.setFrequency(baseHz * pitchRatio * bRatio * detuneRatio);
            }
        }

        float left = 0.0f, right = 0.0f;
        const auto mix = juce::jlimit(0.0f, 1.0f, smoothedMix.getNextValue() + frame.mixOffset);
        const auto spread = juce::jlimit(0.0f, 1.0f, smoothedSpread.getNextValue() + frame.spreadOffset);
        for (int i = 0; i < unisonCount; ++i)
        {
            const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
            const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
            const auto pan = juce::jlimit(-1.0f, 1.0f, position * spread);
            const auto leftGain = std::sqrt(0.5f * (1.0f - pan));
            const auto rightGain = std::sqrt(0.5f * (1.0f + pan));
            const auto source = juce::jmap(mix, oscillatorA[static_cast<size_t>(i)].process(), oscillatorB[static_cast<size_t>(i)].process()) * unisonNormalisation;
            left += source * leftGain; right += source * rightGain;
        }

        const auto centredSub = subOscillator.process() * subLevel * 0.55f;
        const auto noise = (noiseRandom.nextFloat() * 2.0f - 1.0f) * noiseLevel * 0.30f;
        left += centredSub + noise; right += centredSub + noise;

        const auto amp = ampEnvelope.getNextSample() * noteVelocity;
        const auto filterEnv = filterEnvelope.getNextSample();
        left *= amp; right *= amp;

        const auto driveDb = juce::jlimit(0.0f, 36.0f, smoothedDrive.getNextValue() + frame.driveDb);
        if (driveDb > 0.01f)
        {
            const auto driveGain = juce::Decibels::decibelsToGain(driveDb) * (1.0f + velocityTimbre * noteVelocity * 0.35f);
            const auto norm = 1.0f / juce::jmax(0.25f, std::tanh(driveGain));
            left = antialiasedTanh(left * driveGain, previousDriveInputL) * norm;
            right = antialiasedTanh(right * driveGain, previousDriveInputR) * norm;
        }

        // NASTY is a parallel synthesis character path. Amount=0 is bit-for-behaviour bypass.
        const auto nastyAmount = juce::jlimit(0.0f, 1.0f, parameter("nasty_amount") + frame.nastyAmount);
        const auto moment = momentEnvelope(juce::jlimit(0.0f, 1.0f, parameter("nasty_moment") + frame.nastyMoment));
        if (nastyAmount > 0.0001f || moment > 0.0001f)
        {
            const auto model = static_cast<NastyCell::Model>(juce::jlimit(0, 3, juce::roundToInt(parameter("nasty_model"))));
            const auto deform = juce::jlimit(0.0f, 1.0f, parameter("nasty_deform") + frame.nastyDeform + 0.35f * moment);
            const auto feedback = juce::jlimit(0.0f, 0.985f, parameter("nasty_feedback") + frame.nastyFeedback + 0.25f * moment);
            const auto coupling = juce::jlimit(0.0f, 1.0f, parameter("nasty_coupling") + frame.nastyCoupling + 0.30f * moment);
            const auto nastyEnergy = juce::jlimit(0.0f, 1.0f, parameter("nasty_energy") + frame.nastyEnergy + 0.55f * moment);
            const auto damping = juce::jlimit(0.02f, 1.0f, parameter("nasty_damping") + frame.nastyDamping - 0.25f * moment);
            const auto wet = juce::jlimit(0.0f, 1.0f, nastyAmount + 0.65f * moment);
            const auto wetL = nastyCell.process(left, 0, model, deform, feedback, coupling, nastyEnergy, damping);
            const auto wetR = nastyCell.process(right, 1, model, deform, feedback, coupling, nastyEnergy, damping);
            left = juce::jmap(wet, left, wetL);
            right = juce::jmap(wet, right, wetR);
        }

        const auto cutoff = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45),
                                         smoothedCutoff.getNextValue() * std::pow(2.0f, filterEnvAmount * filterEnv * velocityScale + frame.cutoffOctaves));
        const auto moddedPublicResonance = juce::jlimit(0.2f, 12.0f, publicResonance + frame.resonanceOffset);
        const auto moddedResonance01 = (moddedPublicResonance - 0.2f) / 11.8f;
        const auto resonance = juce::jmap(moddedResonance01, 1.35f, 0.08f);
        filterA.setCutoffFrequency(cutoff); filterA.setResonance(resonance);
        filterB.setCutoffFrequency(cutoff); filterB.setResonance(resonance);
        left = filterA.processSample(0, left); right = filterA.processSample(1, right);
        if (filterMode == 1)
        {
            left = filterB.processSample(0, left); right = filterB.processSample(1, right);
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

float OmnariaVoice::parameter(const juce::String& parameterID) const noexcept
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
