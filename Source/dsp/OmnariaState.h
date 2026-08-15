#pragma once

#include <atomic>

namespace omnaria
{
// Shared performance and host state. This is infrastructure, not a claimed synthesis method.
struct OmnariaState
{
    std::atomic<float> bpm { 120.0f };
    std::atomic<float> phrasePosition { 0.0f };   // 0..1 across a 16-bar phrase
    std::atomic<float> performanceEnergy { 0.0f };
    std::atomic<float> historyState { 0.0f };
    std::atomic<float> motion { 0.0f };
    std::atomic<float> focus { 0.75f };
    std::atomic<float> coupling { 0.0f };
    std::atomic<float> harmonicCentre { 0.0f };
    std::atomic<float> spectralLow { 0.34f };
    std::atomic<float> spectralMid { 0.33f };
    std::atomic<float> spectralHigh { 0.33f };
    std::atomic<int> activeNotes { 0 };
    std::atomic<int> rootPitchClass { 0 };
    std::atomic<bool> hostPlaying { false };
};
} // namespace omnaria
