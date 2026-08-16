#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace omnaria
{
struct GlideLaw
{
    // Exponential decay in cents. This keeps the trajectory musical because
    // equal intervals are treated consistently in logarithmic pitch space.
    static float centsAtTime(float startCents, float seconds, float timeConstantSeconds) noexcept
    {
        const auto tau = juce::jmax(0.001f, timeConstantSeconds);
        return startCents * std::exp(-juce::jmax(0.0f, seconds) / tau);
    }

    static float frequencyFromTarget(float targetHz, float residualCents) noexcept
    {
        return juce::jmax(0.0f, targetHz) * std::pow(2.0f, residualCents / 1200.0f);
    }

    static float intervalCents(int fromMidiNote, int toMidiNote) noexcept
    {
        return static_cast<float>(toMidiNote - fromMidiNote) * 100.0f;
    }

    static float direction(int fromMidiNote, int toMidiNote) noexcept
    {
        return toMidiNote > fromMidiNote ? 1.0f : (toMidiNote < fromMidiNote ? -1.0f : 0.0f);
    }

    // A bounded gesture signal for modulation. 0 = stationary; 1 = very fast /
    // large movement. This is intentionally independent from raw pitch so it can
    // drive timbre without adding random or accidental pitch wobble.
    static float normalisedGestureVelocity(float residualCents, float tauSeconds) noexcept
    {
        const auto tau = juce::jmax(0.001f, tauSeconds);
        const auto centsPerSecond = std::abs(residualCents) / tau;
        return 1.0f - std::exp(-centsPerSecond / 1800.0f);
    }

    // Larger intervals may excite more synthesis, but saturate smoothly so an
    // octave jump does not become twelve times 'more effect' than a semitone.
    static float intervalExcitation(float intervalCents, float characteristicCents = 500.0f) noexcept
    {
        const auto scale = juce::jmax(1.0f, characteristicCents);
        return 1.0f - std::exp(-std::abs(intervalCents) / scale);
    }
};
} // namespace omnaria
