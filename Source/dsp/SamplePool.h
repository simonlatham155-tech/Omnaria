#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <mutex>

namespace omnaria
{
struct SampleData
{
    juce::AudioBuffer<float> audio;
    double sampleRate { 44100.0 };
    juce::String name { "Empty" };
};

class SamplePool
{
public:
    void publish(std::shared_ptr<const SampleData> data) noexcept
    {
        current.store(std::move(data));
    }

    std::shared_ptr<const SampleData> get() const noexcept
    {
        return current.load();
    }

private:
    std::atomic<std::shared_ptr<const SampleData>> current;
};

class SampleVoice
{
public:
    void prepare(double outputRate) noexcept { hostSampleRate = juce::jmax(1.0, outputRate); }

    void start(std::shared_ptr<const SampleData> newSample, int midiNote, int rootNote,
               float start01, float end01, bool reverse) noexcept
    {
        sample = std::move(newSample);
        note = midiNote;
        root = rootNote;
        direction = reverse ? -1.0 : 1.0;
        if (! sample || sample->audio.getNumSamples() < 2)
        {
            active = false;
            return;
        }

        const auto length = sample->audio.getNumSamples();
        startIndex = juce::jlimit(0.0, static_cast<double>(length - 2), static_cast<double>(start01) * (length - 1));
        endIndex = juce::jlimit(startIndex + 1.0, static_cast<double>(length - 1), static_cast<double>(end01) * (length - 1));
        position = reverse ? endIndex : startIndex;
        active = true;
    }

    void stop() noexcept { active = false; }
    bool isActive() const noexcept { return active && sample != nullptr; }

    std::pair<float, float> process(float tuneSemitones, int mode, float positionOffset,
                                    float scan, float jitter, float& randomBipolar) noexcept
    {
        if (! isActive()) return { 0.0f, 0.0f };

        const auto channels = sample->audio.getNumChannels();
        const auto sourceRate = juce::jmax(1.0, sample->sampleRate);
        const auto pitchRatio = std::pow(2.0, (static_cast<double>(note - root) + tuneSemitones) / 12.0);
        const auto increment = pitchRatio * sourceRate / hostSampleRate;
        const auto region = juce::jmax(1.0, endIndex - startIndex);

        if (mode == 2) // TEXTURE: scan within the selected region; position modulation becomes a synthesis control.
        {
            const auto centre = startIndex + region * juce::jlimit(0.0f, 1.0f, 0.5f + positionOffset * 0.5f);
            const auto wander = randomBipolar * jitter * region * 0.08;
            position += direction * increment * (0.05 + 1.95 * juce::jlimit(0.0f, 1.0f, scan));
            const auto attraction = (centre + wander - position) * (0.0005 + 0.004 * scan);
            position += attraction;
        }
        else
        {
            position += direction * increment;
        }

        if (position > endIndex || position < startIndex)
        {
            if (mode == 0) // ONE SHOT
            {
                active = false;
                return { 0.0f, 0.0f };
            }

            const auto overshoot = position > endIndex ? position - endIndex : startIndex - position;
            position = direction > 0.0 ? startIndex + std::fmod(overshoot, region)
                                       : endIndex - std::fmod(overshoot, region);
        }

        const auto left = readLinear(0, position);
        const auto right = readLinear(channels > 1 ? 1 : 0, position);
        return { left, right };
    }

private:
    float readLinear(int channel, double index) const noexcept
    {
        const auto size = sample->audio.getNumSamples();
        const auto i0 = juce::jlimit(0, size - 1, static_cast<int>(index));
        const auto i1 = juce::jmin(size - 1, i0 + 1);
        const auto frac = static_cast<float>(index - static_cast<double>(i0));
        const auto* data = sample->audio.getReadPointer(channel);
        return juce::jmap(frac, data[i0], data[i1]);
    }

    std::shared_ptr<const SampleData> sample;
    double hostSampleRate { 44100.0 };
    double position { 0.0 };
    double startIndex { 0.0 };
    double endIndex { 1.0 };
    double direction { 1.0 };
    int note { 60 };
    int root { 60 };
    bool active { false };
};
} // namespace omnaria
