#pragma once

#include <JuceHeader.h>
#include <array>
#include "BandlimitedOscillator.h"
#include "OmnariaState.h"

namespace omnaria
{
class OmnariaSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class OmnariaVoice final : public juce::SynthesiserVoice
{
public:
    OmnariaVoice(juce::AudioProcessorValueTreeState& parameters, const OmnariaState& sharedState);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void prepare(double sampleRate, int maximumBlockSize);

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    static constexpr int maxUnison = 9;

    float parameter(const char* parameterID) const noexcept;
    BandlimitedOscillator::Shape shapeForIndex(int index) const noexcept;
    float pitchWheelRatio() const noexcept;
    void configureFilterTypes(int mode);

    juce::AudioProcessorValueTreeState& params;
    const OmnariaState& state;

    std::array<BandlimitedOscillator, maxUnison> oscillatorA;
    std::array<BandlimitedOscillator, maxUnison> oscillatorB;
    BandlimitedOscillator subOscillator;

    juce::dsp::StateVariableTPTFilter<float> filterA;
    juce::dsp::StateVariableTPTFilter<float> filterB;
    juce::ADSR ampEnvelope;
    juce::ADSR filterEnvelope;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedCutoff;
    juce::SmoothedValue<float> smoothedResonance;
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedSpread;

    juce::Random noiseRandom { 0x4f4d4e49 };
    double currentSampleRate { 44100.0 };
    int currentMidiNote { 60 };
    int pitchWheel { 8192 };
    int activeFilterMode { -1 };
    float noteVelocity { 0.0f };
};
} // namespace omnaria
