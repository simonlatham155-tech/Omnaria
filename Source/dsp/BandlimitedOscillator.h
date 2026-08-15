#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace omnaria
{
class BandlimitedOscillator
{
public:
    enum class Shape { sine = 0, saw, pulse };

    void prepare(double newSampleRate) noexcept
    {
        sampleRate = juce::jmax(1.0, newSampleRate);
        reset();
    }

    void reset(double newPhase = 0.0) noexcept
    {
        phase = newPhase - std::floor(newPhase);
    }

    void setFrequency(float newFrequency) noexcept
    {
        frequency = juce::jlimit(0.0f, static_cast<float>(sampleRate * 0.45), newFrequency);
    }

    void setShape(Shape newShape) noexcept { shape = newShape; }
    void setPulseWidth(float newPulseWidth) noexcept { pulseWidth = juce::jlimit(0.05f, 0.95f, newPulseWidth); }
    double getPhase() const noexcept { return phase; }

    // Controlled hard-sync family: amount 0 leaves the slave untouched; amount 1
    // places it exactly at target phase. Intermediate values shorten the current
    // cycle without an abrupt all-or-nothing mode switch.
    void nudgePhaseToward(double targetPhase, float amount01) noexcept
    {
        const auto a = juce::jlimit(0.0f, 1.0f, amount01);
        auto target = targetPhase - std::floor(targetPhase);
        auto delta = target - phase;
        if (delta > 0.5) delta -= 1.0;
        if (delta < -0.5) delta += 1.0;
        phase += static_cast<double>(a) * delta;
        phase -= std::floor(phase);
    }

    float process() noexcept
    {
        const auto dt = juce::jlimit(1.0e-7, 0.45, static_cast<double>(frequency) / sampleRate);
        const auto t = phase;
        float value = 0.0f;

        switch (shape)
        {
            case Shape::sine:
                value = std::sin(static_cast<float>(juce::MathConstants<double>::twoPi * t));
                break;

            case Shape::saw:
                value = static_cast<float>(2.0 * t - 1.0);
                value -= polyBlep(t, dt);
                break;

            case Shape::pulse:
            {
                value = t < pulseWidth ? 1.0f : -1.0f;
                value += polyBlep(t, dt);
                auto shifted = t - static_cast<double>(pulseWidth);
                if (shifted < 0.0)
                    shifted += 1.0;
                value -= polyBlep(shifted, dt);
                break;
            }
        }

        phase += dt;
        phase -= std::floor(phase);
        return value;
    }

private:
    static float polyBlep(double t, double dt) noexcept
    {
        if (t < dt)
        {
            const auto x = t / dt;
            return static_cast<float>(x + x - x * x - 1.0);
        }

        if (t > 1.0 - dt)
        {
            const auto x = (t - 1.0) / dt;
            return static_cast<float>(x * x + x + x + 1.0);
        }

        return 0.0f;
    }

    double sampleRate { 44100.0 };
    double phase { 0.0 };
    float frequency { 440.0f };
    float pulseWidth { 0.5f };
    Shape shape { Shape::saw };
};
} // namespace omnaria
