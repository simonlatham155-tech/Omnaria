#pragma once

#include <JuceHeader.h>
#include <array>
#include "OmnariaState.h"

namespace omnaria
{
class OmnariaStateEngine
{
public:
    explicit OmnariaStateEngine(OmnariaState& sharedState) : state(sharedState) {}

    void prepare(double newSampleRate);
    void reset();

    void processBlock(const juce::MidiBuffer& midi,
                      juce::AudioPlayHead* playHead,
                      int numSamples,
                      float motionAmount,
                      float historyAmount,
                      float focusAmount,
                      float couplingAmount);

private:
    void consumeMidi(const juce::MidiBuffer& midi);
    void updateHostPosition(juce::AudioPlayHead* playHead);
    void updatePerformanceState(int numSamples, float historyAmount);

    OmnariaState& state;
    double sampleRate { 44100.0 };
    std::array<bool, 128> heldNotes {};
    std::array<float, 128> noteVelocity {};
    float energyFollower { 0.0f };
    float historyFollower { 0.0f };
    float previousCentre { 0.0f };
};
} // namespace omnaria
