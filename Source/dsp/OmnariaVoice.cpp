#include "OmnariaVoice.h"
#include <cmath>

namespace omnaria
{
OmnariaVoice::OmnariaVoice(juce::AudioProcessorValueTreeState& parameters, const OmnariaState& sharedState)
    : params(parameters), state(sharedState)
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

bool OmnariaVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<OmnariaSound*>(sound) != nullptr;
}

void OmnariaVoice::prepare(double sampleRate, int maximumBlockSize)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    for (auto& osc : oscillatorA)
        osc.prepare(currentSampleRate);
    for (auto& osc : oscillatorB)
        osc.prepare(currentSampleRate);

    juce::dsp::ProcessSpec spec { currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, maximumBlockSize)), 2 };
    filter.prepare(spec);
    filter.reset();
    ampEnvelope.setSampleRate(currentSampleRate);
    filterEnvelope.setSampleRate(currentSampleRate);
}

void OmnariaVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentMidiNote = midiNoteNumber;
    pitchWheel = currentPitchWheelPosition;
    noteVelocity = velocity;

    for (int i = 0; i < maxUnison; ++i)
    {
        const auto phaseA = std::fmod(0.173 * static_cast<double>(i) + 0.031 * midiNoteNumber, 1.0);
        const auto phaseB = std::fmod(0.397 * static_cast<double>(i) + 0.019 * midiNoteNumber, 1.0);
        oscillatorA[static_cast<size_t>(i)].reset(phaseA);
        oscillatorB[static_cast<size_t>(i)].reset(phaseB);
    }

    filter.reset();
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

void OmnariaVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const auto shapeA = shapeForIndex(static_cast<int>(std::round(parameter("oscA_shape"))));
    const auto shapeB = shapeForIndex(static_cast<int>(std::round(parameter("oscB_shape"))));
    const auto mix = juce::jlimit(0.0f, 1.0f, parameter("osc_mix"));
    const auto coarseB = parameter("oscB_coarse");
    const auto unisonCount = juce::jlimit(1, maxUnison, static_cast<int>(std::round(parameter("unison"))));
    const auto detuneCents = parameter("detune");
    const auto spread = juce::jlimit(0.0f, 1.0f, parameter("spread"));
    const auto driveDb = juce::jmax(0.0f, parameter("drive"));

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

    auto baseCutoff = parameter("cutoff");
    if (motion > 0.0f)
    {
        const auto phraseArc = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phrase);
        const auto movementOctaves = motion * (0.55f * energy + 0.35f * phraseArc + 0.20f * history);
        baseCutoff *= std::pow(2.0f, movementOctaves);
    }

    const auto resonance = juce::jlimit(0.2f, 12.0f, parameter("resonance"));
    filter.setResonance(resonance);

    const auto normalisation = 0.82f / std::sqrt(static_cast<float>(unisonCount));
    const auto driveGain = juce::Decibels::decibelsToGain(driveDb);

    for (int i = 0; i < unisonCount; ++i)
    {
        const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
        // A slightly curved distribution keeps more energy near the centre while
        // retaining wide outer voices. This is a stronger dance-music baseline
        // than evenly spacing every unison oscillator.
        const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
        const auto drift = motion * history * 1.5f * std::sin((static_cast<float>(currentMidiNote) + i * 7.0f) * 0.37f);
        const auto cents = position * detuneCents + drift;
        const auto detuneRatio = std::pow(2.0f, cents / 1200.0f);

        auto& a = oscillatorA[static_cast<size_t>(i)];
        auto& b = oscillatorB[static_cast<size_t>(i)];
        a.setShape(shapeA);
        b.setShape(shapeB);
        a.setFrequency(baseHz * detuneRatio);
        b.setFrequency(baseHz * bRatio * detuneRatio);
    }

    const auto filterEnvAmount = parameter("filter_env_amt");
    const auto velocityTimbre = juce::jlimit(0.0f, 1.0f, parameter("velocity_timbre"));
    const auto velocityScale = 1.0f + velocityTimbre * (noteVelocity - 0.5f) * 1.4f;
    const auto numChannels = outputBuffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float left = 0.0f;
        float right = 0.0f;

        for (int i = 0; i < unisonCount; ++i)
        {
            const auto linearPosition = unisonCount == 1 ? 0.0f : (2.0f * static_cast<float>(i) / static_cast<float>(unisonCount - 1) - 1.0f);
            const auto position = std::copysign(std::pow(std::abs(linearPosition), 1.18f), linearPosition);
            const auto pan = juce::jlimit(-1.0f, 1.0f, position * spread);
            const auto leftGain = std::sqrt(0.5f * (1.0f - pan));
            const auto rightGain = std::sqrt(0.5f * (1.0f + pan));

            const auto a = oscillatorA[static_cast<size_t>(i)].process();
            const auto b = oscillatorB[static_cast<size_t>(i)].process();
            const auto voiceSample = juce::jmap(mix, a, b) * normalisation;
            left += voiceSample * leftGain;
            right += voiceSample * rightGain;
        }

        const auto amp = ampEnvelope.getNextSample() * noteVelocity;
        const auto filterEnv = filterEnvelope.getNextSample();
        left *= amp;
        right *= amp;

        if (driveDb > 0.01f)
        {
            const auto expressiveDrive = 1.0f + velocityTimbre * noteVelocity * 0.35f;
            left = std::tanh(left * driveGain * expressiveDrive);
            right = std::tanh(right * driveGain * expressiveDrive);
        }

        const auto envelopeOctaves = filterEnvAmount * filterEnv * velocityScale;
        const auto cutoff = juce::jlimit(20.0f,
                                         static_cast<float>(currentSampleRate * 0.45),
                                         baseCutoff * std::pow(2.0f, envelopeOctaves));
        filter.setCutoffFrequency(cutoff);

        left = filter.processSample(0, left);
        right = filter.processSample(1, right);

        const auto targetSample = startSample + sample;
        if (numChannels > 0)
            outputBuffer.addSample(0, targetSample, left);
        if (numChannels > 1)
            outputBuffer.addSample(1, targetSample, right);
    }

    if (! ampEnvelope.isActive())
        clearCurrentNote();
}

float OmnariaVoice::parameter(const char* parameterID) const noexcept
{
    if (const auto* value = params.getRawParameterValue(parameterID))
        return value->load();
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
