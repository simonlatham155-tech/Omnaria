#pragma once

#include <JuceHeader.h>
#include <array>

namespace omnaria
{
class ProductionFX
{
public:
    enum class Order { motionDelaySpace = 0, delayMotionSpace = 1 };

    void prepare(double sampleRate, int maximumBlockSize);
    void reset();

    void process(juce::AudioBuffer<float>& buffer,
                 float bpm,
                 float motionMix,
                 float motionRate,
                 float motionDepth,
                 float delayMix,
                 int delayDivision,
                 float delayFeedback,
                 float spaceMix,
                 float spaceSize,
                 float spaceDamping,
                 float width,
                 Order order);

private:
    void processMotion(juce::AudioBuffer<float>& buffer, float mix, float rate, float depth);
    void processDelay(juce::AudioBuffer<float>& buffer, float bpm, float mix, int division, float feedback);
    void processSpace(juce::AudioBuffer<float>& buffer, float mix, float size, float damping);
    static void processWidth(juce::AudioBuffer<float>& buffer, float width);

    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> delayBuffer;
    int delayWritePosition { 0 };
    double currentSampleRate { 44100.0 };
};
} // namespace omnaria
