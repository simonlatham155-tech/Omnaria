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

    // Gesture telemetry for mono/legato synthesis. These values describe the
    // musical journey between note-ons; they do not themselves alter pitch.
    std::atomic<int> previousNoteOn { 60 };
    std::atomic<int> targetNoteOn { 60 };
    std::atomic<float> glideIntervalCents { 0.0f };
    std::atomic<float> glideDirection { 0.0f };    // -1 down, 0 none, +1 up
    std::atomic<float> glideExcitation { 0.0f };   // 0..1 interval-derived gesture strength
    std::atomic<unsigned int> noteOnSerial { 0 }; // increments for every note-on
};
} // namespace omnaria
