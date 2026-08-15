#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace omnaria
{
struct TempoGateLaw
{
    // Convert a musical division expressed in quarter-note units to Hz.
    // Examples: 1.0 = quarter, 0.5 = eighth, 0.25 = sixteenth.
    static float rateHz(float bpm, float quarterNoteUnits) noexcept
    {
        const auto beatsPerSecond = juce::jmax(20.0f, bpm) / 60.0f;
        return beatsPerSecond / juce::jmax(0.03125f, quarterNoteUnits);
    }

    static float phaseIncrement(float bpm, float quarterNoteUnits, double sampleRate) noexcept
    {
        return rateHz(bpm, quarterNoteUnits) / static_cast<float>(juce::jmax(1.0, sampleRate));
    }

    // A gate with independently controlled duty and edge smoothing. This keeps
    // rhythm timing separate from the note envelope: a pluck can remain a pluck
    // while a trance gate rhythmically exposes or hides it.
    static float gate(float phase01, float duty01, float edge01) noexcept
    {
        phase01 -= std::floor(phase01);
        const auto duty = juce::jlimit(0.02f, 0.98f, duty01);
        const auto edge = juce::jlimit(0.0005f, 0.25f, edge01);
        const auto rise = smoothstep(0.0f, edge, phase01);
        const auto fall = 1.0f - smoothstep(duty - edge, duty, phase01);
        return juce::jlimit(0.0f, 1.0f, rise * fall);
    }

    template <size_t N>
    static float stepValue(float phase01, const std::array<float, N>& pattern) noexcept
    {
        static_assert(N > 0);
        phase01 -= std::floor(phase01);
        const auto index = juce::jlimit(0, static_cast<int>(N) - 1,
                                       static_cast<int>(phase01 * static_cast<float>(N)));
        return pattern[static_cast<size_t>(index)];
    }

    // Phase-coherent retrigger option for rapid bass/sequence notes. Blend 0 is
    // fully free-running, 1 is exact retrigger, values between preserve groove
    // while reducing transient randomness.
    static float retriggerBlend(float freePhase01, float notePhase01, float amount01) noexcept
    {
        const auto a = juce::jlimit(0.0f, 1.0f, amount01);
        const auto x = juce::jmap(a, freePhase01, notePhase01);
        return x - std::floor(x);
    }

private:
    static float smoothstep(float a, float b, float x) noexcept
    {
        if (b <= a) return x >= b ? 1.0f : 0.0f;
        const auto t = juce::jlimit(0.0f, 1.0f, (x - a) / (b - a));
        return t * t * (3.0f - 2.0f * t);
    }
};
} // namespace omnaria
