#pragma once

#include <atomic>

namespace omnaria
{
struct LatWorldState
{
    std::atomic<float> bpm { 120.0f };
    std::atomic<float> phrasePosition { 0.0f };   // 0..1 across a 16-bar phrase
    std::atomic<float> worldEnergy { 0.0f };     // performance activity / accumulated energy
    std::atomic<float> memoryState { 0.0f };     // decaying history-sensitive state
    std::atomic<float> evolution { 0.0f };
    std::atomic<float> gravity { 0.75f };
    std::atomic<float> interaction { 0.0f };
    std::atomic<float> harmonicCentre { 0.0f };  // pitch-class centre, normalised 0..1
    std::atomic<float> spectralLow { 0.34f };
    std::atomic<float> spectralMid { 0.33f };
    std::atomic<float> spectralHigh { 0.33f };
    std::atomic<int> activeNotes { 0 };
    std::atomic<int> rootPitchClass { 0 };
    std::atomic<bool> hostPlaying { false };
};
} // namespace omnaria
