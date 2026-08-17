#include "Engine3Voice.h"
#include <cmath>

namespace
{
constexpr int K = 24;
constexpr double twoPi = juce::MathConstants<double>::twoPi;
}

Engine3Voice::Engine3Voice(Engine3Params p) : params(p) {}

bool Engine3Voice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<Engine3Sound*>(sound) != nullptr;
}

void Engine3Voice::prepare(double sampleRate)
{
    sr = sampleRate;
    adsr.setSampleRate(sr);
}

float Engine3Voice::edgeWarp(float u)
{
    u = juce::jlimit(0.0f, 1.0f, u);
    const float x = 2.0f * u - 1.0f;
    const float y = std::copysign(std::pow(std::abs(x), 0.60f), x);
    return 0.5f + 0.5f * y;
}

float Engine3Voice::noise()
{
    noiseState ^= noiseState << 13;
    noiseState ^= noiseState >> 17;
    noiseState ^= noiseState << 5;
    return (static_cast<float>(noiseState) / static_cast<float>(0xffffffffu)) * 2.0f - 1.0f;
}

void Engine3Voice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    noteVelocity = velocity;
    phases.fill(0.0);
    modPhase = 0.0;
    crossPhase = 0.31;
    smoothedNoise = 0.0;
    noiseState = 0x9e3779b9u ^ static_cast<uint32_t>(midiNoteNumber * 2654435761u);

    adsrParams.attack  = *params.attack;
    adsrParams.decay   = *params.decay;
    adsrParams.sustain = *params.sustain;
    adsrParams.release = *params.release;
    adsr.setParameters(adsrParams);
    adsr.noteOn();
}

void Engine3Voice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void Engine3Voice::renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    const float density = edgeWarp(*params.partialDensity);
    const float bright  = edgeWarp(*params.spectralTilt);
    const float mod     = edgeWarp(*params.modulationDepth);
    const float reson   = edgeWarp(*params.resonance);
    const float excite  = edgeWarp(*params.excitation);
    const float nonlin  = edgeWarp(*params.nonlinearity);
    const float asym    = edgeWarp(*params.asymmetry);

    const double center = 3.0 + 21.0 * density;
    const double tilt = 2.8 - 2.15 * bright;
    const double modRatio = 1.0 + 2.5 * (0.35 + 0.65 * bright);
    const double crossRatio = 1.0 + 0.5 * density;
    const double modInc = twoPi * baseFrequency * modRatio / sr;
    const double crossInc = twoPi * baseFrequency * crossRatio / sr;
    const double drive = 0.75 + 4.5 * nonlin;
    const double bias = 0.55 * asym;
    const double tanhNorm = std::tanh(drive) + 1.0e-9;

    for (int s = 0; s < numSamples; ++s)
    {
        const float n = noise();
        smoothedNoise += (static_cast<double>(n) - smoothedNoise) * (1.0 / 17.0);
        const double lfo = std::sin(modPhase);
        const double cross = std::sin(crossPhase);

        double bank = 0.0;
        for (int i = 0; i < K; ++i)
        {
            const double k = static_cast<double>(i + 1);
            const double gate = 1.0 / (1.0 + std::exp((k - center) / 1.15));
            const double amp = gate * std::pow(k, -tilt);
            const double dispersion = (0.0002 + 0.0105 * reson) * (k - 1.0) / (K - 1.0);
            const double ratio = k * (1.0 + dispersion);
            const double depth = (0.05 + 5.4 * mod) * (0.35 + 0.65 * gate);
            const double phaseKick = depth * (0.72 * lfo + 0.28 * cross)
                                   + (0.20 * excite) * smoothedNoise * (0.25 + 0.75 * k / K);
            const double phaseInc = twoPi * baseFrequency * ratio / sr;
            phases[static_cast<size_t>(i)] += phaseInc;
            if (phases[static_cast<size_t>(i)] > twoPi)
                phases[static_cast<size_t>(i)] -= twoPi;
            const double ampNoise = 1.0 + 0.34 * excite * smoothedNoise;
            bank += amp * ampNoise * std::sin(phases[static_cast<size_t>(i)] + phaseKick);
        }

        const double exc = (0.02 + 0.28 * excite) * smoothedNoise * (0.35 + 0.65 * reson);
        double x = bank + exc;
        x = std::tanh(drive * (x + bias)) / tanhNorm;

        // Remove most of the DC introduced by asymmetric shaping without changing the core interaction.
        static constexpr double dcLeak = 0.9995;
        static thread_local double dcIn = 0.0, dcOut = 0.0;
        const double hp = x - dcIn + dcLeak * dcOut;
        dcIn = x;
        dcOut = hp;

        const float env = adsr.getNextSample();
        const float y = static_cast<float>(0.10 * hp) * env * noteVelocity;
        for (int ch = 0; ch < output.getNumChannels(); ++ch)
            output.addSample(ch, startSample + s, y);

        modPhase += modInc;
        crossPhase += crossInc;
        if (modPhase > twoPi) modPhase -= twoPi;
        if (crossPhase > twoPi) crossPhase -= twoPi;
    }

    if (!adsr.isActive())
        clearCurrentNote();
}
