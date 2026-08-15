#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace omnaria
{
class NastyCell
{
public:
    enum class Model { Fold = 0, Feedback = 1, Coupled = 2, Duffing = 3 };

    void prepare(double sampleRate)
    {
        fs = juce::jmax(1.0, sampleRate);
        reset();
    }

    void reset() noexcept
    {
        z1L = z1R = 0.0f;
        resonatorL = resonatorR = 0.0f;
        velocityL = velocityR = 0.0f;
    }

    float process(float x, int channel, Model model, float deform, float feedback,
                  float coupling, float energy, float damping) noexcept
    {
        auto& z1 = channel == 0 ? z1L : z1R;
        auto& q = channel == 0 ? resonatorL : resonatorR;
        auto& v = channel == 0 ? velocityL : velocityR;

        deform = juce::jlimit(0.0f, 1.0f, deform);
        feedback = juce::jlimit(0.0f, 0.985f, feedback);
        coupling = juce::jlimit(0.0f, 1.0f, coupling);
        energy = juce::jlimit(0.0f, 1.0f, energy);
        damping = juce::jlimit(0.02f, 1.0f, damping);

        float y = x;
        switch (model)
        {
            case Model::Fold:
            {
                const auto gain = 1.0f + 10.0f * deform + 3.0f * energy;
                const auto t = x * gain + z1 * feedback;
                y = fold(t);
                break;
            }
            case Model::Feedback:
            {
                const auto phaseLike = x + feedback * std::tanh(z1 * (1.0f + 8.0f * deform));
                y = std::sin(phaseLike * (1.0f + 6.0f * energy));
                break;
            }
            case Model::Coupled:
            {
                const auto other = channel == 0 ? resonatorR : resonatorL;
                const auto force = x * (0.12f + 0.88f * energy) + coupling * std::tanh(other - q);
                v += (force - (0.02f + damping * 0.18f) * v - (0.08f + 0.30f * deform) * q
                      - deform * 0.14f * q * q * q);
                v = juce::jlimit(-3.0f, 3.0f, v);
                q = juce::jlimit(-3.0f, 3.0f, q + v * 0.035f);
                y = std::tanh(x + q * (0.35f + 1.4f * coupling));
                break;
            }
            case Model::Duffing:
            {
                const auto beta = 0.10f + 1.20f * deform;
                const auto force = x * (0.15f + energy * 1.35f) + feedback * z1;
                v += force - damping * 0.12f * v - 0.20f * q - beta * q * q * q;
                v = juce::jlimit(-2.5f, 2.5f, v);
                q = juce::jlimit(-2.5f, 2.5f, q + v * 0.028f);
                y = std::tanh(x + q);
                break;
            }
        }

        if (! std::isfinite(y))
            y = 0.0f;

        // Energy guard: keep the cell near instability, never numerically explosive.
        const auto stateEnergy = std::abs(q) + 0.35f * std::abs(v);
        if (stateEnergy > 2.2f)
        {
            q *= 0.92f;
            v *= 0.86f;
        }

        z1 = juce::jlimit(-2.0f, 2.0f, y);
        return z1;
    }

private:
    static float fold(float x) noexcept
    {
        x = std::fmod(x + 3.0f, 4.0f);
        if (x < 0.0f) x += 4.0f;
        return 1.0f - std::abs(x - 2.0f);
    }

    double fs { 44100.0 };
    float z1L { 0.0f }, z1R { 0.0f };
    float resonatorL { 0.0f }, resonatorR { 0.0f };
    float velocityL { 0.0f }, velocityR { 0.0f };
};
} // namespace omnaria
