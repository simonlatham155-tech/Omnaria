#pragma once

#include <JuceHeader.h>
#include <string>
#include <vector>
#include "../dsp/SoundQA.h"

class OmnariaAudioProcessor;

namespace omnaria
{
struct PresetQAProbe
{
    std::string probeName;
    SoundQAMetrics metrics;
};

struct PresetQAResult
{
    int program = 0;
    std::string name;
    std::string category;
    std::vector<PresetQAProbe> probes;
};

class PresetRenderHarness
{
public:
    static constexpr double sampleRate = 48000.0;
    static constexpr int blockSize = 256;

    // Runs every factory preset through two deterministic musical probes:
    // 1) a monophonic interval phrase for attack/pitch/glide/translation;
    // 2) a triad probe for peak stacking, masking and stereo behaviour.
    static std::vector<PresetQAResult> runAll();

    static juce::String toJson(const std::vector<PresetQAResult>& results);

private:
    static juce::AudioBuffer<float> renderProbe(int program, bool chordProbe);
    static void renderInto(OmnariaAudioProcessor& processor,
                           juce::AudioBuffer<float>& destination,
                           const juce::MidiBuffer& midi,
                           int totalSamples);
    static juce::MidiBuffer makeMonophonicProbe(int totalSamples);
    static juce::MidiBuffer makeChordProbe(int totalSamples);
};
}
