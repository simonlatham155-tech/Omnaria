#include "LatWorldEngine.h"
#include <cmath>

namespace omnaria
{
void LatWorldEngine::prepare(double newSampleRate)
{
    sampleRate = juce::jmax(1.0, newSampleRate);
    reset();
}

void LatWorldEngine::reset()
{
    heldNotes.fill(false);
    noteVelocity.fill(0.0f);
    energyFollower = 0.0f;
    memoryFollower = 0.0f;
    previousCentre = 0.0f;
    state.activeNotes.store(0);
    state.worldEnergy.store(0.0f);
    state.memoryState.store(0.0f);
}

void LatWorldEngine::processBlock(const juce::MidiBuffer& midi,
                                  juce::AudioPlayHead* playHead,
                                  int numSamples,
                                  float evolutionAmount,
                                  float memoryAmount,
                                  float gravityAmount,
                                  float interactionAmount)
{
    state.evolution.store(juce::jlimit(0.0f, 1.0f, evolutionAmount));
    state.gravity.store(juce::jlimit(0.0f, 1.0f, gravityAmount));
    state.interaction.store(juce::jlimit(0.0f, 1.0f, interactionAmount));

    consumeMidi(midi);
    updateHostPosition(playHead);
    updateMusicalState(numSamples, juce::jlimit(0.0f, 1.0f, memoryAmount));
}

void LatWorldEngine::consumeMidi(const juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            const auto note = juce::jlimit(0, 127, message.getNoteNumber());
            heldNotes[static_cast<size_t>(note)] = true;
            noteVelocity[static_cast<size_t>(note)] = message.getFloatVelocity();
        }
        else if (message.isNoteOff())
        {
            const auto note = juce::jlimit(0, 127, message.getNoteNumber());
            heldNotes[static_cast<size_t>(note)] = false;
            noteVelocity[static_cast<size_t>(note)] = 0.0f;
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            heldNotes.fill(false);
            noteVelocity.fill(0.0f);
        }
    }
}

void LatWorldEngine::updateHostPosition(juce::AudioPlayHead* playHead)
{
    if (playHead == nullptr)
    {
        state.hostPlaying.store(false);
        return;
    }

    const auto position = playHead->getPosition();
    if (! position.hasValue())
    {
        state.hostPlaying.store(false);
        return;
    }

    const auto& info = *position;
    state.hostPlaying.store(info.getIsPlaying());

    if (const auto bpm = info.getBpm())
        state.bpm.store(static_cast<float>(*bpm));

    double beatsPerBar = 4.0;
    if (const auto signature = info.getTimeSignature())
        beatsPerBar = static_cast<double>(signature->numerator) * 4.0 / static_cast<double>(juce::jmax(1, signature->denominator));

    if (const auto ppq = info.getPpqPosition())
    {
        const auto phraseLength = juce::jmax(1.0, beatsPerBar * 16.0);
        auto wrapped = std::fmod(*ppq, phraseLength);
        if (wrapped < 0.0)
            wrapped += phraseLength;
        state.phrasePosition.store(static_cast<float>(wrapped / phraseLength));
    }
}

void LatWorldEngine::updateMusicalState(int numSamples, float memoryAmount)
{
    int count = 0;
    int lowest = 127;
    float velocitySum = 0.0f;
    float noteSum = 0.0f;

    for (int note = 0; note < 128; ++note)
    {
        if (! heldNotes[static_cast<size_t>(note)])
            continue;

        ++count;
        lowest = juce::jmin(lowest, note);
        velocitySum += noteVelocity[static_cast<size_t>(note)];
        noteSum += static_cast<float>(note);
    }

    state.activeNotes.store(count);

    const auto root = count > 0 ? lowest % 12 : state.rootPitchClass.load();
    const auto centre = static_cast<float>(root) / 11.0f;
    state.rootPitchClass.store(root);
    state.harmonicCentre.store(centre);

    const auto averageVelocity = count > 0 ? velocitySum / static_cast<float>(count) : 0.0f;
    const auto density = juce::jlimit(0.0f, 1.0f, static_cast<float>(count) / 6.0f);
    const auto targetEnergy = count > 0 ? averageVelocity * (0.45f + 0.55f * density) : 0.0f;
    const auto energyDecay = std::exp(-static_cast<float>(numSamples) / static_cast<float>(sampleRate * 0.35));
    energyFollower = targetEnergy + (energyFollower - targetEnergy) * energyDecay;
    state.worldEnergy.store(juce::jlimit(0.0f, 1.0f, energyFollower));

    auto centreDelta = std::abs(centre - previousCentre);
    centreDelta = juce::jmin(centreDelta, 1.0f - centreDelta);
    const auto novelty = juce::jlimit(0.0f, 1.0f, centreDelta * 3.0f + targetEnergy * 0.25f);
    const auto memorySeconds = 0.5f + memoryAmount * 18.0f;
    const auto memoryDecay = std::exp(-static_cast<float>(numSamples) / static_cast<float>(sampleRate * memorySeconds));
    memoryFollower = juce::jmax(memoryFollower * memoryDecay, novelty * memoryAmount);
    state.memoryState.store(juce::jlimit(0.0f, 1.0f, memoryFollower));
    previousCentre = centre;

    const auto averageNote = count > 0 ? noteSum / static_cast<float>(count) : 60.0f;
    const auto registerPosition = juce::jlimit(0.0f, 1.0f, (averageNote - 24.0f) / 84.0f);
    const auto evolutionAmount = state.evolution.load();
    auto low = juce::jlimit(0.08f, 0.70f, 0.58f - registerPosition * 0.42f);
    auto high = juce::jlimit(0.10f, 0.68f, 0.14f + registerPosition * 0.30f + evolutionAmount * 0.24f);
    auto mid = juce::jmax(0.08f, 1.0f - low - high);
    const auto total = low + mid + high;
    state.spectralLow.store(low / total);
    state.spectralMid.store(mid / total);
    state.spectralHigh.store(high / total);
}
} // namespace omnaria
