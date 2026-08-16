#include "PresetRenderHarness.h"
#include "../PluginProcessor.h"
#include "../FactoryPresets.h"
#include <memory>

namespace omnaria
{
namespace
{
int secondsToSamples(double seconds)
{
    return static_cast<int>(std::round(seconds * PresetRenderHarness::sampleRate));
}

void addNote(juce::MidiBuffer& midi, int note, float velocity, double startSeconds, double endSeconds)
{
    midi.addEvent(juce::MidiMessage::noteOn(1, note, velocity), secondsToSamples(startSeconds));
    midi.addEvent(juce::MidiMessage::noteOff(1, note), secondsToSamples(endSeconds));
}

juce::var metricsToVar(const SoundQAMetrics& m)
{
    auto* object = new juce::DynamicObject();
    object->setProperty("peakDb", m.peakDb);
    object->setProperty("rmsDb", m.rmsDb);
    object->setProperty("crestDb", m.crestDb);
    object->setProperty("stereoCorrelation", m.stereoCorrelation);
    object->setProperty("lowBandSideRatio", m.lowBandSideRatio);
    object->setProperty("transientRatio", m.transientRatio);
    object->setProperty("dcOffset", m.dcOffset);
    object->setProperty("highBandEnergyRatio", m.highBandEnergyRatio);
    object->setProperty("spectralCentroidHz", m.spectralCentroidHz);
    object->setProperty("peakWaste", m.peakWaste);
    juce::Array<juce::var> warnings;
    for (const auto& warning : m.warnings) warnings.add(juce::String(warning));
    object->setProperty("warnings", warnings);
    return juce::var(object);
}
}

std::vector<PresetQAResult> PresetRenderHarness::runAll()
{
    std::vector<PresetQAResult> results;
    const auto& bank = factoryPresets();
    results.reserve(bank.size());

    for (int program = 0; program < static_cast<int>(bank.size()); ++program)
    {
        PresetQAResult result;
        result.program = program;
        result.name = bank[static_cast<size_t>(program)].name;
        result.category = bank[static_cast<size_t>(program)].category;

        auto mono = renderProbe(program, false);
        result.probes.push_back({ "mono_interval", SoundQA::analyse(mono, sampleRate) });

        auto chord = renderProbe(program, true);
        result.probes.push_back({ "triad", SoundQA::analyse(chord, sampleRate) });

        results.push_back(std::move(result));
    }
    return results;
}

juce::AudioBuffer<float> PresetRenderHarness::renderProbe(int program, bool chordProbe)
{
    constexpr double durationSeconds = 4.0;
    const int totalSamples = secondsToSamples(durationSeconds);
    juce::AudioBuffer<float> rendered(2, totalSamples);
    rendered.clear();

    // Fresh processor per probe keeps voices, envelopes, RNG and FX histories
    // deterministic and prevents one factory program contaminating the next.
    OmnariaAudioProcessor processor;
    processor.setRateAndBufferSizeDetails(sampleRate, blockSize);
    processor.prepareToPlay(sampleRate, blockSize);
    processor.setCurrentProgram(program);

    const auto midi = chordProbe ? makeChordProbe(totalSamples) : makeMonophonicProbe(totalSamples);
    renderInto(processor, rendered, midi, totalSamples);
    processor.releaseResources();
    return rendered;
}

void PresetRenderHarness::renderInto(OmnariaAudioProcessor& processor,
                                     juce::AudioBuffer<float>& destination,
                                     const juce::MidiBuffer& midi,
                                     int totalSamples)
{
    int position = 0;
    while (position < totalSamples)
    {
        const int n = std::min(blockSize, totalSamples - position);
        juce::AudioBuffer<float> block(2, n);
        block.clear();

        juce::MidiBuffer blockMidi;
        const auto end = position + n;
        for (const auto metadata : midi)
        {
            if (metadata.samplePosition >= position && metadata.samplePosition < end)
                blockMidi.addEvent(metadata.getMessage(), metadata.samplePosition - position);
        }

        processor.processBlock(block, blockMidi);
        for (int channel = 0; channel < 2; ++channel)
            destination.copyFrom(channel, position, block, channel, 0, n);
        position += n;
    }
}

juce::MidiBuffer PresetRenderHarness::makeMonophonicProbe(int)
{
    juce::MidiBuffer midi;
    // Stable starting note, a fifth and an octave jump exercise transient,
    // key tracking, mono glide and pitch-dependent specialist layers.
    addNote(midi, 48, 0.82f, 0.10, 1.05); // C3
    addNote(midi, 55, 0.76f, 1.15, 2.05); // G3
    addNote(midi, 60, 0.90f, 2.10, 3.25); // C4
    return midi;
}

juce::MidiBuffer PresetRenderHarness::makeChordProbe(int)
{
    juce::MidiBuffer midi;
    // C minor then F minor. This deliberately reveals peak stacking, low-mid
    // masking, phase behaviour and whether width survives polyphonic use.
    for (const int note : { 48, 51, 55 }) addNote(midi, note, 0.72f, 0.10, 1.55);
    for (const int note : { 53, 56, 60 }) addNote(midi, note, 0.76f, 1.80, 3.30);
    return midi;
}

juce::String PresetRenderHarness::toJson(const std::vector<PresetQAResult>& results)
{
    juce::Array<juce::var> presetArray;
    for (const auto& result : results)
    {
        auto* preset = new juce::DynamicObject();
        preset->setProperty("program", result.program + 1);
        preset->setProperty("name", juce::String(result.name));
        preset->setProperty("category", juce::String(result.category));
        juce::Array<juce::var> probes;
        for (const auto& probe : result.probes)
        {
            auto* p = new juce::DynamicObject();
            p->setProperty("name", juce::String(probe.probeName));
            p->setProperty("metrics", metricsToVar(probe.metrics));
            probes.add(juce::var(p));
        }
        preset->setProperty("probes", probes);
        presetArray.add(juce::var(preset));
    }

    auto* root = new juce::DynamicObject();
    root->setProperty("sampleRate", sampleRate);
    root->setProperty("blockSize", blockSize);
    root->setProperty("presets", presetArray);
    return juce::JSON::toString(juce::var(root), true);
}
}
