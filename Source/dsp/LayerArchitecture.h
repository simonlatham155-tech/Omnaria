#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace omnaria
{
// Shared mathematical laws exposed by the Phase-8 preset audit. These helpers
// keep specialist engines complementary to CORE instead of merely stacking more
// energy into the same spectral/temporal space.
struct LayerArchitecture
{
    struct ThreeBandMask
    {
        float low { 1.0f };
        float mid { 1.0f };
        float high { 1.0f };
    };

    // A3: smooth spectral territory. Returns 0 below the territory, 1 above it.
    static float upperTerritory(float frequencyHz, float crossoverHz, float widthOctaves = 1.0f) noexcept
    {
        const auto f = juce::jmax(1.0f, frequencyHz);
        const auto fc = juce::jmax(1.0f, crossoverHz);
        const auto w = juce::jmax(0.05f, widthOctaves);
        const auto x = std::log2(f / fc) / w;
        return 0.5f + 0.5f * std::tanh(2.0f * x);
    }

    // A4: keep bass/body coherent while progressively permitting upper width.
    static float frequencyDependentWidth(float requestedWidth, float frequencyHz,
                                         float coherenceHz = 180.0f,
                                         float fullWidthHz = 1800.0f) noexcept
    {
        const auto lo = juce::jmax(20.0f, coherenceHz);
        const auto hi = juce::jmax(lo + 1.0f, fullWidthHz);
        const auto x = juce::jlimit(0.0f, 1.0f,
            std::log(juce::jmax(frequencyHz, lo) / lo) / std::log(hi / lo));
        const auto smooth = x * x * (3.0f - 2.0f * x);
        return juce::jlimit(0.0f, 1.0f, requestedWidth) * smooth;
    }

    // A8: FM/PM amount rolls back as carrier approaches Nyquist. This is not a
    // replacement for oversampling/band-limiting, but prevents a preset law from
    // demanding impossible sideband bandwidth.
    static float aliasAwareFmAmount(float requestedAmount, float carrierHz,
                                    float modulatorHz, float sampleRate,
                                    float estimatedSidebandOrders = 6.0f) noexcept
    {
        const auto nyquist = 0.5f * juce::jmax(1.0f, sampleRate);
        const auto occupiedTop = juce::jmax(0.0f, carrierHz)
                               + juce::jmax(0.0f, modulatorHz) * juce::jmax(1.0f, estimatedSidebandOrders);
        const auto margin = juce::jlimit(0.0f, 1.0f, (nyquist - occupiedTop) / (0.35f * nyquist));
        const auto smooth = margin * margin * (3.0f - 2.0f * margin);
        return juce::jmax(0.0f, requestedAmount) * smooth;
    }

    // A10: protected-sub split. Specialist processing receives only the upper
    // body weight; CORE/sub remains dominant below the crossover.
    static float protectedUpperBody(float frequencyHz, float crossoverHz = 120.0f) noexcept
    {
        return upperTerritory(frequencyHz, crossoverHz, 0.75f);
    }

    // A11: simple redundancy score for two normalised layer descriptors. High
    // correlation + high spectral overlap means the second layer is not earning
    // much unique acoustic territory.
    static float redundancyPenalty(float correlation, float spectralOverlap) noexcept
    {
        const auto c = std::pow(juce::jlimit(0.0f, 1.0f, std::abs(correlation)), 1.5f);
        return c * juce::jlimit(0.0f, 1.0f, spectralOverlap);
    }

    // A11: masking cost is intentionally asymmetric: a loud specialist layer
    // covering CORE costs more than a quiet complementary layer.
    static float maskingPenalty(float coreEnergy, float specialistEnergy, float overlap) noexcept
    {
        const auto c = juce::jmax(0.0f, coreEnergy);
        const auto s = juce::jmax(0.0f, specialistEnergy);
        const auto total = juce::jmax(1.0e-9f, c + s);
        return juce::jlimit(0.0f, 1.0f, overlap) * (s / total) * std::sqrt(c * s);
    }

    // A13: formant/resonance target follows pitch only by the requested amount.
    // 0 = fixed formant, 1 = fully key tracked.
    static float trackedFormantHz(float baseHz, int midiNote, float keyTrack) noexcept
    {
        const auto semitones = static_cast<float>(midiNote - 60) * juce::jlimit(0.0f, 1.0f, keyTrack);
        return juce::jlimit(20.0f, 20000.0f, juce::jmax(20.0f, baseHz) * std::pow(2.0f, semitones / 12.0f));
    }

    // A15: exponential musical pitch trajectory for risers/downers/lasers.
    static float trajectoryHz(float startHz, float endHz, float phase01, float curvature = 1.0f) noexcept
    {
        const auto p = std::pow(juce::jlimit(0.0f, 1.0f, phase01), juce::jmax(0.05f, curvature));
        const auto a = std::log(juce::jmax(1.0f, startHz));
        const auto b = std::log(juce::jmax(1.0f, endHz));
        return std::exp(a + (b - a) * p);
    }

    // Layer acceptance efficiency: useful perceptual gain per combined cost.
    static float layerEfficiency(float usefulGain, float masking, float redundancy,
                                 float peakCost, float cpuCost) noexcept
    {
        const auto cost = 1.0e-4f + juce::jmax(0.0f, masking)
                                    + juce::jmax(0.0f, redundancy)
                                    + juce::jmax(0.0f, peakCost)
                                    + juce::jmax(0.0f, cpuCost);
        return juce::jmax(0.0f, usefulGain) / cost;
    }
};
} // namespace omnaria
