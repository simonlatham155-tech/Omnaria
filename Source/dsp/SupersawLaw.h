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

    // A fixed cents spread causes absolute beat rate to rise with note frequency:
    // deltaF = f0 * (2^(c/1200)-1). For a lead this can become buzzy in the top
    // octaves and too static in the low range. Blend constant-cents behaviour with
    // an A4-referenced constant-Hz target, then clamp so the patch keeps its identity.
    static float pitchAwareDetune(float publicDetuneCents, float baseHz, float constantHzBlend = 0.58f) noexcept
    {
        publicDetuneCents = juce::jmax(0.0f, publicDetuneCents);
        baseHz = juce::jmax(20.0f, baseHz);
        constantHzBlend = juce::jlimit(0.0f, 1.0f, constantHzBlend);

        constexpr float referenceHz = 440.0f;
        const auto referenceBeatHz = referenceHz * (std::pow(2.0f, publicDetuneCents / 1200.0f) - 1.0f);
        const auto constantHzCents = 1200.0f * std::log2(1.0f + referenceBeatHz / baseHz);
        const auto blended = juce::jmap(constantHzBlend, publicDetuneCents, constantHzCents);

        // Avoid huge low-note spreads or an over-tight top octave.
        const auto lower = publicDetuneCents * 0.62f;
        const auto upper = publicDetuneCents * 1.55f;
        return juce::jlimit(lower, upper, blended);
    }

    // Preserve approximately the same expected incoherent power as the previous
    // equal-gain normaliser, so the new weighting cannot 'win' just by being louder.
    static float voiceGain(int voiceIndex, int voiceCount, float detuneCents) noexcept
    {
        const auto exponent = juce::jmap(juce::jlimit(0.0f, 1.0f, detuneCents / 35.0f), 0.88f, 0.58f);
        const auto oldPerVoiceGain = 0.90f / std::pow(static_cast<float>(juce::jmax(1, voiceCount)), exponent);

        if (voiceCount == 7 && voiceIndex >= 0 && voiceIndex < 7)
        {
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
