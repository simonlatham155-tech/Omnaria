#pragma once

#include <JuceHeader.h>
#include "../dsp/OmnariaState.h"

namespace omnaria
{
class WorldGlobe final : public juce::Component,
                         private juce::Timer
{
public:
    explicit WorldGlobe(const OmnariaState& engineState);
    ~WorldGlobe() override = default;

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    juce::Point<float> projectParticle(float longitude, float latitude, float radius, juce::Point<float> centre) const;

    const OmnariaState& state;
    float animationPhase { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorldGlobe)
};
} // namespace omnaria
