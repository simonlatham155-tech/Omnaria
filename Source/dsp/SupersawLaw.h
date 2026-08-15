#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace omnaria
{
// Phase 8 / Preset 01 physical unison law.
// Frequency placement, stereo placement and energy placement are intentionally separate.
struct SupersawLaw
{
    static constexpr std::array<float, 7> frequencyOffsets {
        -1.0000f, -0.5762f, -0.2877f, 0.0f, 0.3241f, 0.6473f, 0.9461f
    };

    static constexpr std::array<float, 7> stereoPositions {
        -1.0000f, -0.6400f, -0.3150f, 0.0f, 0.3150f, 0.6400f, 1.0000f
    };

    // Power-normalised centre weighting. The centre carries more coherent energy
    // for pitch certainty / mono / small-speaker survival; the sides remain strong
    // enough to provide the moving stereo cloud. Sum(g_i^2) == approximately 1.
    static constexpr std::array<float, 7> powerWeights {
        0.30690303f, 0.34952845f, 0.39641641f,
        0.50297996f,
        0.39641641f, 0.34952845f, 0.30690303f
    };

    static float frequencyPosition(int voiceIndex, int voiceCount) noexcept
    {
        if (voiceCount <= 1)
            return 0.0f;

        if (voiceCount == 7 && voiceIndex >= 0 && voiceIndex < 7)
            return frequencyOffsets[static_cast<size_t>(voiceIndex)];

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

    // Preserve approximately the same expected incoherent power as the previous
    // equal-gain normaliser, so the new weighting cannot 'win' just by being louder.
    static float voiceGain(int voiceIndex, int voiceCount, float detuneCents) noexcept
    {
        const auto exponent = juce::jmap(juce::jlimit(0.0f, 1.0f, detuneCents / 35.0f), 0.88f, 0.58f);
        const auto oldPerVoiceGain = 0.90f / std::pow(static_cast<float>(juce::jmax(1, voiceCount)), exponent);

        if (voiceCount == 7 && voiceIndex >= 0 && voiceIndex < 7)
        {
            // Old expected incoherent power = N * g^2. Multiply our unit-power
            // weight vector by sqrt(N) * g to retain that same expected power.
            const auto referencePowerScale = std::sqrt(static_cast<float>(voiceCount)) * oldPerVoiceGain;
            return powerWeights[static_cast<size_t>(voiceIndex)] * referencePowerScale;
        }

        return oldPerVoiceGain;
    }

    // Candidate B micro-decorrelation: Brown changes cloud width, not centre pitch.
    static float brownCloudScale(float brown, float depth = 0.035f) noexcept
    {
        constexpr float destinationCents = 18.0f;
        return destinationCents * juce::jlimit(-1.0f, 1.0f, brown)
               * juce::jlimit(0.0f, 1.0f, depth);
    }

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
