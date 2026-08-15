#pragma once

#include <JuceHeader.h>
#include <array>
#include "LatWorldState.h"

namespace omnaria
{
class LatWorldEngine
{
public:
    explicit LatWorldEngine(LatWorldState& sharedState) : state(sharedState) {}

    void prepare(double newSampleRate);
    void reset();

    void processBlock(const juce::MidiBuffer& midi,
                      juce::AudioPlayHead* playHead,
                      int numSamples,
                      float evolutionAmount,
                      float memoryAmount,
                      float gravityAmount,
                      float interactionAmount);

private:
    void consumeMidi(const juce::MidiBuffer& midi);
    void updateHostPosition(juce::AudioPlayHead* playHead);
    void updateMusicalState(int numSamples, float memoryAmount);

    LatWorldState& state;
    double sampleRate { 44100.0 };
    std::array<bool, 128> heldNotes {};
    std::array<float, 128> noteVelocity {};
    float energyFollower { 0.0f };
    float memoryFollower { 0.0f };
    float previousCentre { 0.0f };
};
} // namespace omnaria
