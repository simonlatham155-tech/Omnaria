#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace omnaria
{
// Shared Phase-8 architecture for plucks, mallets, guitar-like bodies and
// transient/body separation. The exciter is intentionally short; the resonator
// owns pitch and decay so samples/noise do not have to become the whole sound.
class ExcitationResonator
{
public:
    static constexpr int maxModes = 6;

    void prepare(double newSampleRate) noexcept
    {
        sampleRate = juce::jmax(1.0, newSampleRate);
        reset();
    }

    void reset() noexcept
    {
        for (auto& m : modes) { m.y1 = 0.0f; m.y2 = 0.0f; }
        exciter = 0.0f;
        exciterDecay = 0.0f;
    }

    // material01: 0 soft/wood, 1 hard/glass. Harder materials excite more
    // upper modal energy and decay more slowly in the high modes.
    void trigger(float baseHz, float velocity, float material01,
                 float decaySeconds, float brightness01) noexcept
    {
        baseFrequency = juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.20), baseHz);
        velocity = juce::jlimit(0.0f, 1.0f, velocity);
        material = juce::jlimit(0.0f, 1.0f, material01);
        brightness = juce::jlimit(0.0f, 1.0f, brightness01);
        const auto decay = juce::jlimit(0.03f, 12.0f, decaySeconds);

        // Short excitation pulse. Energy is bounded so stronger transients do
        // not automatically win via peak level.
        exciter = std::sqrt(velocity) * (0.55f + 0.45f * brightness);
        const auto exciterMs = juce::jmap(material, 0.010f, 0.0015f);
        exciterDecay = std::exp(-1.0f / static_cast<float>(sampleRate * exciterMs));

        // Inharmonic ratios span wood/string-like to bell/glass-like behaviour.
        static constexpr std::array<float, maxModes> harmonic { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
        static constexpr std::array<float, maxModes> glassy   { 1.0f, 2.32f, 3.87f, 5.21f, 6.93f, 8.41f };

        for (int i = 0; i < maxModes; ++i)
        {
            auto& m = modes[static_cast<size_t>(i)];
            const auto ratio = juce::jmap(material, harmonic[static_cast<size_t>(i)], glassy[static_cast<size_t>(i)]);
            const auto freq = juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.45), baseFrequency * ratio);
            const auto omega = juce::MathConstants<float>::twoPi * freq / static_cast<float>(sampleRate);

            // Upper modes die faster for wood/soft materials; hard materials
            // preserve more upper modal energy.
            const auto modeIndex = static_cast<float>(i) / static_cast<float>(maxModes - 1);
            const auto highDamping = juce::jmap(material, 2.8f, 0.75f);
            const auto modeDecay = decay / (1.0f + highDamping * modeIndex * modeIndex);
            const auto r = std::exp(-1.0f / static_cast<float>(sampleRate * juce::jmax(0.01f, modeDecay)));
            m.a1 = 2.0f * r * std::cos(omega);
            m.a2 = -(r * r);

            const auto spectralTilt = std::pow(1.0f - 0.72f * modeIndex, juce::jmap(brightness, 1.6f, 0.55f));
            const auto hardBoost = 1.0f + material * modeIndex * 0.55f;
            m.gain = spectralTilt * hardBoost;
        }

        normaliseModeEnergy();
    }

    float process(float externalExcitation = 0.0f) noexcept
    {
        const auto impulse = exciter + externalExcitation;
        exciter *= exciterDecay;
        if (std::abs(exciter) < 1.0e-7f) exciter = 0.0f;

        float y = 0.0f;
        for (auto& m : modes)
        {
            const auto next = impulse * m.gain + m.a1 * m.y1 + m.a2 * m.y2;
            m.y2 = m.y1;
            m.y1 = next;
            y += next;
        }
        return y * outputNormalisation;
    }

private:
    struct Mode
    {
        float a1 { 0.0f }, a2 { 0.0f };
        float y1 { 0.0f }, y2 { 0.0f };
        float gain { 0.0f };
    };

    void normaliseModeEnergy() noexcept
    {
        float energy = 0.0f;
        for (const auto& m : modes) energy += m.gain * m.gain;
        outputNormalisation = energy > 1.0e-8f ? 0.72f / std::sqrt(energy) : 0.0f;
    }

    std::array<Mode, maxModes> modes {};
    double sampleRate { 44100.0 };
    float baseFrequency { 440.0f };
    float velocity { 0.0f }, material { 0.0f }, brightness { 0.0f };
    float exciter { 0.0f }, exciterDecay { 0.0f };
    float outputNormalisation { 0.0f };
};

// Peak-efficient transient/body crossfade. It favours the exciter only while it
// carries unique attack information, then hands energy to the resonant body.
struct TransientBodyLaw
{
    static float transientWeight(float elapsedSeconds, float transientSeconds) noexcept
    {
        const auto t = juce::jmax(0.0005f, transientSeconds);
        return std::exp(-juce::jmax(0.0f, elapsedSeconds) / t);
    }

    static float bodyWeight(float transientWeightValue) noexcept
    {
        const auto a = juce::jlimit(0.0f, 1.0f, transientWeightValue);
        return std::sqrt(juce::jmax(0.0f, 1.0f - a * a));
    }

    static std::pair<float, float> constantPowerWeights(float elapsedSeconds, float transientSeconds) noexcept
    {
        const auto transient = transientWeight(elapsedSeconds, transientSeconds);
        return { transient, bodyWeight(transient) };
    }
};
} // namespace omnaria
