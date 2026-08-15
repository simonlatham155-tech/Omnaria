#include "OmnariaVoice.h"
#include "SupersawLaw.h"
#include "LayerArchitecture.h"
#include "TempoGateLaw.h"
#include <cmath>

namespace omnaria
{
OmnariaVoice::OmnariaVoice(juce::AudioProcessorValueTreeState& parameters, const OmnariaState& sharedState, const SamplePool& sharedSamples)
    : params(parameters), state(sharedState), samples(sharedSamples)
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
    sampleVoice.prepare(currentSampleRate);
    resonator.prepare(currentSampleRate);

    juce::dsp::ProcessSpec spec { currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, maximumBlockSize)), 2 };
    filterA.prepare(spec); filterB.prepare(spec);
    filterA.reset(); filterB.reset();
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
    gatePhase = 0.0f;
}

void OmnariaVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentMidiNote = midiNoteNumber;
    pitchWheel = currentPitchWheelPosition;
    noteVelocity = velocity;
    momentPhase = 0.0f;
    gatePhase = 0.0f;
    nastyCell.reset();

    const bool randomPhase = parameter("phase_mode") >= 0.5f;
    const auto requestedPhase = juce::jlimit(0.0f, 1.0f, parameter("phase"));
    for (int i = 0; i < maxUnison; ++i)
    {
        const auto phaseA = randomPhase ? noiseRandom.nextDouble() : std::fmod(static_cast<double>(requestedPhase) + 0.137 * i, 1.0);
        const auto phaseB = randomPhase ? noiseRandom.nextDouble() : std::fmod(static_cast<double>(requestedPhase) + 0.370 + 0.193 * i, 1.0);
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
    sampleVoice.start(samples.get(), midiNoteNumber, juce::roundToInt(parameter("sample_root")),
                      parameter("sample_start"), parameter("sample_end"), parameter("sample_reverse") >= 0.5f);

    const auto noteHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    resonator.trigger(noteHz, velocity,
                      juce::jlimit(0.0f, 1.0f, parameter("macro2")),
                      juce::jlimit(0.03f, 12.0f, parameter("decay")),
                      juce::jlimit(0.0f, 1.0f, parameter("velocity_timbre")));

    filterA.reset(); filterB.reset();
    previousDriveInputL = previousDriveInputR = 0.0f;
    ampEnvelope.noteOn(); filterEnvelope.noteOn();
    configureAuxEnvelopes();
    for (auto& envelope : auxEnvelopes) envelope.noteOn();
}

void OmnariaVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnvelope.noteOff(); filterEnvelope.noteOff();
        for (auto& envelope : auxEnvelopes) envelope.noteOff();
    }
    else
    {
        ampEnvelope.reset(); filterEnvelope.reset();
        for (auto& envelope : auxEnvelopes) envelope.reset();
        nastyCell.reset(); sampleVoice.stop(); resonator.reset();
        clearCurrentNote();
    }
}

void OmnariaVoice::pitchWheelMoved(int v) { pitchWheel = v; }
void OmnariaVoice::controllerMoved(int c, int v) { if (c == 1) modWheel = juce::jlimit(0.0f, 1.0f, v / 127.0f); }
void OmnariaVoice::aftertouchChanged(int v) { aftertouch = juce::jlimit(0.0f, 1.0f, v / 127.0f); }
void OmnariaVoice::channelPressureChanged(int v) { aftertouch = juce::jlimit(0.0f, 1.0f, v / 127.0f); }

void OmnariaVoice::configureFilterTypes(int mode)
{
    activeFilterMode = juce::jlimit(0, 3, mode);
    auto type = juce::dsp::StateVariableTPTFilterType::lowpass;
    if (activeFilterMode == 2) type = juce::dsp::StateVariableTPTFilterType::highpass;
    if (activeFilterMode == 3) type = juce::dsp::StateVariableTPTFilterType::bandpass;
    filterA.setType(type); filterB.setType(type);
}

float OmnariaVoice::antialiasedTanh(float x, float& previousX) noexcept
{
    const auto delta = x - previousX;
    const auto result = std::abs(delta) < 1.0e-4f ? std::tanh(0.5f * (x + previousX))
                                                  : (std::log(std::cosh(x)) - std::log(std::cosh(previousX))) / delta;
    previousX = x;
    return result;
}

void OmnariaVoice::configureAuxEnvelopes()
{
    for (int i = 0; i < auxEnvelopeCount; ++i)
    {
        const auto prefix = "env" + juce::String(i + 1) + "_";
        auxEnvelopes[static_cast<size_t>(i)].setParameters({ parameter(prefix + "attack"), parameter(prefix + "decay"),
                                                            parameter(prefix + "sustain"), parameter(prefix + "release") });
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
            phase += rateHz / currentSampleRate;
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
    brownState = juce::jlimit(-1.0f, 1.0f, brownState * 0.9995f + (noiseRandom.nextFloat() * 2.0f - 1.0f) * 0.0060f);
    if (--stochasticSamplesUntilTarget <= 0)
    {
        stochasticTarget = noiseRandom.nextFloat() * 2.0f - 1.0f;
        stochasticSamplesUntilTarget = juce::jmax(1, static_cast<int>((0.06f + noiseRandom.nextFloat() * 0.24f) * currentSampleRate));
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
        case 9: return juce::jlimit(0.0f, 1.0f, currentMidiNote / 127.0f);
        case 10: return modWheel; case 11: return aftertouch;
        case 12: return parameter("macro1"); case 13: return parameter("macro2");
        case 14: return parameter("macro3"); case 15: return parameter("macro4");
        case 16: return brownState; case 17: return stochasticState;
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
            case 9: frame.nastyAmount += amount; break; case 10: frame.nastyDeform += amount; break;
            case 11: frame.nastyFeedback += amount; break; case 12: frame.nastyCoupling += amount; break;
            case 13: frame.nastyEnergy += amount; break; case 14: frame.nastyDamping += amount; break;
            case 15: frame.nastyMoment += amount; break;
            case 16: frame.sampleLevel += amount; break; case 17: frame.samplePosition += amount; break;
            case 18: frame.sampleScan += amount; break; case 19: frame.sampleJitter += amount; break;
            case 20: frame.sampleTune += amount * 12.0f; break;
            default: break;
        }
    }
    return frame;
}

float OmnariaVoice::momentEnvelope(float amount) noexcept
{
    amount = juce::jlimit(0.0f, 1.0f, amount);
    if (amount <= 0.0001f) return 0.0f;
    const auto durationSeconds = juce::jmap(amount, 1.8f, 0.55f);
    momentPhase = juce::jmin(1.0f, momentPhase + 1.0f / static_cast<float>(currentSampleRate * durationSeconds));
    const auto p = momentPhase;
    float shape = 0.0f;
    if (p < 0.30f) { const auto t = p / 0.30f; shape = t * t * (3.0f - 2.0f * t); }
    else if (p < 0.58f) shape = 1.0f;
    else { const auto t = (p - 0.58f) / 0.42f; shape = 1.0f - t * t * (3.0f - 2.0f * t); }
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

    const auto motion = state.motion.load(), energy = state.performanceEnergy.load(), history = state.historyState.load(), phrase = state.phrasePosition.load();
    const auto baseHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(currentMidiNote)) * pitchWheelRatio();
    const auto bRatio = std::pow(2.0f, coarseB / 12.0f);
    const auto subHz = baseHz * std::pow(2.0f, static_cast<float>(subOctave));
    subOscillator.setFrequency(subHz);

    auto baseCutoff = parameter("cutoff");
    baseCutoff *= std::pow(2.0f, (currentMidiNote - 60.0f) / 12.0f * juce::jlimit(0.0f, 1.0f, parameter("keytrack")));
    if (motion > 0.0f)
    {
        const auto phraseArc = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phrase);
        baseCutoff *= std::pow(2.0f, motion * (0.55f * energy + 0.35f * phraseArc + 0.20f * history));
    }
    smoothedCutoff.setTargetValue(juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), baseCutoff));
    const auto publicResonance = juce::jlimit(0.2f, 12.0f, parameter("resonance"));
    smoothedResonance.setTargetValue(juce::jmap((publicResonance - 0.2f) / 11.8f, 1.35f, 0.08f));
    smoothedDrive.setTargetValue(juce::jmax(0.0f, parameter("drive")));
    smoothedMix.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("osc_mix")));
    smoothedSpread.setTargetValue(juce::jlimit(0.0f, 1.0f, parameter("spread")));
    for (int i = 0; i < unisonCount; ++i) { oscillatorA[i].setShape(shapeA); oscillatorB[i].setShape(shapeB); }

    const auto filterEnvAmount = parameter("filter_env_amt");
    const auto velocityTimbre = juce::jlimit(0.0f, 1.0f, parameter("velocity_timbre"));
    const auto velocityScale = juce::jlimit(0.25f, 1.75f, 1.0f + velocityTimbre * (noteVelocity - 0.5f) * 1.4f);
    const auto numChannels = outputBuffer.getNumChannels();
    const auto sampleMode = juce::jlimit(0, 2, juce::roundToInt(parameter("sample_mode")));
    const auto resonatorAmount = juce::jlimit(0.0f, 1.0f, parameter("macro3"));
    const auto gateAmount = juce::jlimit(0.0f, 1.0f, parameter("macro4"));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        advanceLfos(); advanceStochasticSources();
        for (int i = 0; i < auxEnvelopeCount; ++i) auxEnvelopeValues[i] = auxEnvelopes[i].getNextSample();
        const auto frame = buildModFrame();
        const auto rawDetuneCents = juce::jmax(0.0f, baseDetuneCents + frame.detuneCents);
        const auto detuneCents = SupersawLaw::pitchAwareDetune(rawDetuneCents, baseHz);

        if ((sample & 7) == 0)
        {
            const auto pitchRatio = std::pow(2.0f, frame.pitchSemitones / 12.0f);
            const auto pulseWidth = juce::jlimit(0.05f, 0.95f, basePulseWidth + frame.pulseWidthOffset);
            for (int i = 0; i < unisonCount; ++i)
            {
                const auto frequencyPosition = SupersawLaw::frequencyPosition(i, unisonCount);
                const auto drift = motion * history * 1.5f * std::sin((currentMidiNote + i * 7.0f) * 0.37f);
                const auto ratio = std::pow(2.0f, (frequencyPosition * detuneCents + drift) / 1200.0f);
                oscillatorA[i].setPulseWidth(pulseWidth); oscillatorB[i].setPulseWidth(pulseWidth);
                oscillatorA[i].setFrequency(baseHz * pitchRatio * ratio); oscillatorB[i].setFrequency(baseHz * pitchRatio * bRatio * ratio);
            }
        }

        float left = 0.0f, right = 0.0f;
        const auto mix = juce::jlimit(0.0f, 1.0f, smoothedMix.getNextValue() + frame.mixOffset);
        const auto spread = juce::jlimit(0.0f, 1.0f, smoothedSpread.getNextValue() + frame.spreadOffset);
        for (int i = 0; i < unisonCount; ++i)
        {
            const auto stereoPosition = SupersawLaw::stereoPosition(i, unisonCount);
            const auto frequencyPosition = SupersawLaw::frequencyPosition(i, unisonCount);
            const auto ratio = std::pow(2.0f, frequencyPosition * detuneCents / 1200.0f);
            const auto sourceHz = baseHz * ratio * juce::jmap(mix, 1.0f, bRatio);
            const auto effectiveSpread = LayerArchitecture::frequencyDependentWidth(spread, sourceHz);
            const auto pan = juce::jlimit(-1.0f, 1.0f, stereoPosition * effectiveSpread);
            const auto voiceGain = SupersawLaw::voiceGain(i, unisonCount, detuneCents);
            const auto source = juce::jmap(mix, oscillatorA[i].process(), oscillatorB[i].process()) * voiceGain;
            left += source * std::sqrt(0.5f * (1.0f - pan)); right += source * std::sqrt(0.5f * (1.0f + pan));
        }

        const auto noise = (noiseRandom.nextFloat() * 2.0f - 1.0f) * noiseLevel * 0.30f;
        left += noise; right += noise;

        if (resonatorAmount > 0.0001f)
        {
            const auto resonantBody = resonator.process() * resonatorAmount * 0.42f;
            left += resonantBody;
            right += resonantBody;
        }
        else
        {
            // Keep resonator state advancing even when inaudible so automation into
            // the layer does not reveal a frozen/discontinuous body.
            (void) resonator.process();
        }

        auto randomTexture = stochasticState * 0.7f + brownState * 0.3f;
        const auto samplePair = sampleVoice.process(parameter("sample_tune") + frame.sampleTune, sampleMode,
                                                    juce::jlimit(-1.0f, 1.0f, parameter("sample_position") * 2.0f - 1.0f + frame.samplePosition),
                                                    juce::jlimit(0.0f, 1.0f, parameter("sample_scan") + frame.sampleScan),
                                                    juce::jlimit(0.0f, 1.0f, parameter("sample_jitter") + frame.sampleJitter), randomTexture);
        const auto sampleLevel = juce::jlimit(0.0f, 1.0f, parameter("sample_level") + frame.sampleLevel);
        left += samplePair.first * sampleLevel; right += samplePair.second * sampleLevel;

        const auto amp = ampEnvelope.getNextSample() * noteVelocity;
        const auto filterEnv = filterEnvelope.getNextSample();
        left *= amp; right *= amp;

        const auto driveDb = juce::jlimit(0.0f, 36.0f, smoothedDrive.getNextValue() + frame.driveDb);
        if (driveDb > 0.01f)
        {
            const auto gain = juce::Decibels::decibelsToGain(driveDb) * (1.0f + velocityTimbre * noteVelocity * 0.35f);
            const auto norm = 1.0f / juce::jmax(0.25f, std::tanh(gain));
            left = antialiasedTanh(left * gain, previousDriveInputL) * norm;
            right = antialiasedTanh(right * gain, previousDriveInputR) * norm;
        }

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
            left = juce::jmap(wet, left, nastyCell.process(left, 0, model, deform, feedback, coupling, nastyEnergy, damping));
            right = juce::jmap(wet, right, nastyCell.process(right, 1, model, deform, feedback, coupling, nastyEnergy, damping));
        }

        const auto cutoff = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45),
                                         smoothedCutoff.getNextValue() * std::pow(2.0f, filterEnvAmount * filterEnv * velocityScale + frame.cutoffOctaves));
        const auto resonance01 = (juce::jlimit(0.2f, 12.0f, publicResonance + frame.resonanceOffset) - 0.2f) / 11.8f;
        const auto resonance = juce::jmap(resonance01, 1.35f, 0.08f);
        filterA.setCutoffFrequency(cutoff); filterA.setResonance(resonance);
        filterB.setCutoffFrequency(cutoff); filterB.setResonance(resonance);
        left = filterA.processSample(0, left); right = filterA.processSample(1, right);
        if (filterMode == 1) { left = filterB.processSample(0, left); right = filterB.processSample(1, right); }

        // A10: protected sub is generated from its own coherent oscillator and is
        // recombined after drive, NASTY and filtering. It therefore cannot acquire
        // stereo cancellation or nonlinear low-mid fog from specialist body layers.
        const auto protectedSub = subOscillator.process() * subLevel * 0.55f * amp;
        left += protectedSub;
        right += protectedSub;

        // A9: rhythm is independent from natural note articulation. Macro 4 is a
        // temporary default-off activation path until dedicated UI parameters land.
        gatePhase += TempoGateLaw::phaseIncrement(state.bpm.load(), 0.25f, currentSampleRate);
        gatePhase -= std::floor(gatePhase);
        const auto rhythmicGate = TempoGateLaw::gate(gatePhase, 0.56f, 0.035f);
        const auto gateGain = juce::jmap(gateAmount, 1.0f, rhythmicGate);
        left *= gateGain;
        right *= gateGain;

        const auto target = startSample + sample;
        if (numChannels > 0) outputBuffer.addSample(0, target, left);
        if (numChannels > 1) outputBuffer.addSample(1, target, right);
    }

    if (! ampEnvelope.isActive()) { sampleVoice.stop(); resonator.reset(); clearCurrentNote(); }
}

float OmnariaVoice::parameter(const char* id) const noexcept { if (const auto* v = params.getRawParameterValue(id)) return v->load(); return 0.0f; }
float OmnariaVoice::parameter(const juce::String& id) const noexcept { if (const auto* v = params.getRawParameterValue(id)) return v->load(); return 0.0f; }

BandlimitedOscillator::Shape OmnariaVoice::shapeForIndex(int index) const noexcept
{
    switch (index) { case 1: return BandlimitedOscillator::Shape::pulse; case 2: return BandlimitedOscillator::Shape::sine; default: return BandlimitedOscillator::Shape::saw; }
}

float OmnariaVoice::pitchWheelRatio() const noexcept
{
    const auto normalised = (pitchWheel - 8192.0f) / 8192.0f;
    return std::pow(2.0f, (normalised * 2.0f) / 12.0f);
}
} // namespace omnaria
