#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace omnaria
{
// Phase 8 / Preset 01 physical unison law.
// Frequency placement and stereo placement are intentionally separate.
struct SupersawLaw
{
    // Seven-voice normalized frequency offsets. The centre voice remains exactly
    // at pitch. Side voices are non-uniform so pairwise difference frequencies
    // are distributed rather than sharing one dominant beat spacing.
    // Values are normalized to the public Detune amount (outer voices = +/-1).
    static constexpr std::array<float, 7> frequencyOffsets {
        -1.0000f, -0.5762f, -0.2877f, 0.0f, 0.3241f, 0.6473f, 0.9461f
    };

    // Stereo positions are a separate perceptual problem. Keep a coherent centre
    // and spread the outer voices smoothly without tying pan to frequency offset.
    static constexpr std::array<float, 7> stereoPositions {
        -1.0000f, -0.6400f, -0.3150f, 0.0f, 0.3150f, 0.6400f, 1.0000f
    };

    static float frequencyPosition(int voiceIndex, int voiceCount) noexcept
    {
        if (voiceCount <= 1)
            return 0.0f;

        if (voiceCount == 7 && voiceIndex >= 0 && voiceIndex < 7)
            return frequencyOffsets[static_cast<size_t>(voiceIndex)];

        // Generic fallback for other unison counts: non-linear symmetric spacing.
        const auto linear = 2.0f * static_cast<float>(voiceIndex)
                          / static_cast<float>(voiceCount - 1) - 1.0f;
        return std::copysign(std::pow(std::abs(linear), 1.18f), linear);
    }

    static float stereoPosition(int voiceIndex, int voiceCount) noexcept
    {
        if (voiceCount <= 1)
            return 0.0f;

        if (voiceCount == 7 && voiceIndex >= 0 && voiceIndex < 7)
            return stereoPositions[static_cast<size_t>(voiceIndex)];

        const auto linear = 2.0f * static_cast<float>(voiceIndex)
                          / static_cast<float>(voiceCount - 1) - 1.0f;
        return std::copysign(std::pow(std::abs(linear), 1.10f), linear);
    }

    // Candidate B micro-decorrelation: Brown changes cloud width, not centre pitch.
    static float brownCloudScale(float brown, float depth = 0.035f) noexcept
    {
        // Existing modulation destination maps depth to 18 cents. Express the same
        // +/-0.63-cent bound here as a multiplicative cloud-width perturbation.
        constexpr float destinationCents = 18.0f;
        return destinationCents * juce::jlimit(-1.0f, 1.0f, brown)
               * juce::jlimit(0.0f, 1.0f, depth);
    }

    // Useful offline diagnostic: lower values indicate fewer duplicated pairwise
    // beat spacings. This does not replace listening; it is an optimisation term.
    static float repeatedBeatPenalty(float detuneCents) noexcept
    {
        std::array<float, 21> differences {};
        int count = 0;
        for (size_t i = 0; i < frequencyOffsets.size(); ++i)
            for (size_t j = i + 1; j < frequencyOffsets.size(); ++j)
                differences[static_cast<size_t>(count++)]
                    = std::abs(frequencyOffsets[i] - frequencyOffsets[j]) * detuneCents;

        float penalty = 0.0f;
        constexpr float toleranceCents = 0.35f;
        for (int i = 0; i < count; ++i)
            for (int j = i + 1; j < count; ++j)
            {
                const auto delta = std::abs(differences[static_cast<size_t>(i)]
                                          - differences[static_cast<size_t>(j)]);
                if (delta < toleranceCents)
                    penalty += 1.0f - delta / toleranceCents;
            }
        return penalty;
    }
};
} // namespace omnaria
