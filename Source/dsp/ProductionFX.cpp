#include "ProductionFX.h"
#include <cmath>

namespace omnaria
{
void ProductionFX::prepare(double sampleRate, int maximumBlockSize)
{
    currentSampleRate = juce::jmax(1.0, sampleRate);
    const juce::dsp::ProcessSpec spec { currentSampleRate, static_cast<juce::uint32>(juce::jmax(1, maximumBlockSize)), 2 };
    chorus.prepare(spec);
    reverb.prepare(spec);
    delayBuffer.setSize(2, juce::jmax(2, static_cast<int>(currentSampleRate * 4.0)), false, true, false);
    reset();
}

void ProductionFX::reset()
{
    chorus.reset();
    reverb.reset();
    delayBuffer.clear();
    delayWritePosition = 0;
}

void ProductionFX::process(juce::AudioBuffer<float>& buffer, float bpm, float motionMix, float motionRate, float motionDepth,
                           float delayMix, int delayDivision, float delayFeedback, float spaceMix, float spaceSize,
                           float spaceDamping, float width, Order order)
{
    if (buffer.getNumSamples() <= 0) return;
    if (order == Order::motionDelaySpace)
    {
        processMotion(buffer, motionMix, motionRate, motionDepth);
        processDelay(buffer, bpm, delayMix, delayDivision, delayFeedback);
    }
    else
    {
        processDelay(buffer, bpm, delayMix, delayDivision, delayFeedback);
        processMotion(buffer, motionMix, motionRate, motionDepth);
    }
    processSpace(buffer, spaceMix, spaceSize, spaceDamping);
    processWidth(buffer, width);
}

void ProductionFX::processMotion(juce::AudioBuffer<float>& buffer, float mix, float rate, float depth)
{
    mix = juce::jlimit(0.0f, 1.0f, mix);
    if (mix <= 0.0001f) return;
    chorus.setRate(juce::jlimit(0.03f, 8.0f, rate));
    chorus.setDepth(juce::jlimit(0.0f, 1.0f, depth));
    chorus.setCentreDelay(8.0f);
    chorus.setFeedback(0.08f);
    chorus.setMix(mix);
    juce::dsp::AudioBlock<float> block(buffer);
    chorus.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void ProductionFX::processDelay(juce::AudioBuffer<float>& buffer, float bpm, float mix, int division, float feedback)
{
    mix = juce::jlimit(0.0f, 1.0f, mix);
    if (mix <= 0.0001f || delayBuffer.getNumSamples() < 2) return;
    bpm = juce::jlimit(20.0f, 400.0f, bpm);
    constexpr std::array<float, 8> beats { 4.0f, 2.0f, 1.0f, 0.5f, 0.25f, 0.75f, 0.375f, 0.125f };
    const auto beatCount = beats[static_cast<size_t>(juce::jlimit(0, 7, division))];
    const auto delaySamples = juce::jlimit(1, delayBuffer.getNumSamples() - 1,
                                           static_cast<int>((60.0f / bpm) * beatCount * currentSampleRate));
    feedback = juce::jlimit(0.0f, 0.94f, feedback);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto read = (delayWritePosition - delaySamples + delayBuffer.getNumSamples()) % delayBuffer.getNumSamples();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto lane = juce::jmin(ch, 1);
            const auto dry = buffer.getSample(ch, i);
            const auto wet = delayBuffer.getSample(lane, read);
            const auto cross = buffer.getNumChannels() > 1 ? delayBuffer.getSample(1 - lane, read) : wet;
            const auto pingPong = 0.72f * cross + 0.28f * wet;
            delayBuffer.setSample(lane, delayWritePosition, dry + pingPong * feedback);
            buffer.setSample(ch, i, dry * (1.0f - mix) + pingPong * mix);
        }
        delayWritePosition = (delayWritePosition + 1) % delayBuffer.getNumSamples();
    }
}

void ProductionFX::processSpace(juce::AudioBuffer<float>& buffer, float mix, float size, float damping)
{
    mix = juce::jlimit(0.0f, 1.0f, mix);
    if (mix <= 0.0001f) return;
    juce::dsp::Reverb::Parameters p;
    p.roomSize = juce::jlimit(0.0f, 1.0f, size);
    p.damping = juce::jlimit(0.0f, 1.0f, damping);
    p.wetLevel = mix;
    p.dryLevel = 1.0f - mix;
    p.width = 1.0f;
    p.freezeMode = 0.0f;
    reverb.setParameters(p);
    juce::dsp::AudioBlock<float> block(buffer);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void ProductionFX::processWidth(juce::AudioBuffer<float>& buffer, float width)
{
    if (buffer.getNumChannels() < 2) return;
    width = juce::jlimit(0.0f, 2.0f, width);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto l = buffer.getSample(0, i), r = buffer.getSample(1, i);
        const auto mid = 0.5f * (l + r), side = 0.5f * (l - r) * width;
        buffer.setSample(0, i, mid + side);
        buffer.setSample(1, i, mid - side);
    }
}
} // namespace omnaria
