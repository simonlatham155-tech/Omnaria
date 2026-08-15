#pragma once

#include <JuceHeader.h>
#include <array>
#include "BandlimitedOscillator.h"
#include "NastyCell.h"
#include "OmnariaState.h"
#include "SamplePool.h"

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
    OmnariaVoice(juce::AudioProcessorValueTreeState& parameters, const OmnariaState& sharedState, const SamplePool& sharedSamples);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void prepare(double sampleRate, int maximumBlockSize);

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void aftertouchChanged(int newAftertouchValue) override;
    void channelPressureChanged(int newChannelPressureValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    static constexpr int maxUnison = 9;
    static constexpr int lfoCount = 4;
    static constexpr int auxEnvelopeCount = 3;
    static constexpr int modSlotCount = 4;

    struct ModFrame
    {
        float pitchSemitones { 0.0f };
        float cutoffOctaves { 0.0f };
        float resonanceOffset { 0.0f };
        float mixOffset { 0.0f };
        float detuneCents { 0.0f };
        float spreadOffset { 0.0f };
        float driveDb { 0.0f };
        float pulseWidthOffset { 0.0f };
        float nastyAmount { 0.0f };
        float nastyDeform { 0.0f };
        float nastyFeedback { 0.0f };
        float nastyCoupling { 0.0f };
        float nastyEnergy { 0.0f };
        float nastyDamping { 0.0f };
        float nastyMoment { 0.0f };
        float sampleLevel { 0.0f };
        float samplePosition { 0.0f };
        float sampleScan { 0.0f };
        float sampleJitter { 0.0f };
        float sampleTune { 0.0f };
    };

    float parameter(const char* parameterID) const noexcept;
    float parameter(const juce::String& parameterID) const noexcept;
    BandlimitedOscillator::Shape shapeForIndex(int index) const noexcept;
    float pitchWheelRatio() const noexcept;
    void configureFilterTypes(int mode);
    static float antialiasedTanh(float x, float& previousX) noexcept;

    void configureAuxEnvelopes();
    void advanceLfos();
    void advanceStochasticSources();
    float modulationSourceValue(int sourceIndex) const noexcept;
    ModFrame buildModFrame() const noexcept;
    float momentEnvelope(float amount) noexcept;

    juce::AudioProcessorValueTreeState& params;
    const OmnariaState& state;
    const SamplePool& samples;

    std::array<BandlimitedOscillator, maxUnison> oscillatorA;
    std::array<BandlimitedOscillator, maxUnison> oscillatorB;
    BandlimitedOscillator subOscillator;
    NastyCell nastyCell;
    SampleVoice sampleVoice;

    juce::dsp::StateVariableTPTFilter<float> filterA;
    juce::dsp::StateVariableTPTFilter<float> filterB;
    juce::ADSR ampEnvelope;
    juce::ADSR filterEnvelope;
    std::array<juce::ADSR, auxEnvelopeCount> auxEnvelopes;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedCutoff;
    juce::SmoothedValue<float> smoothedResonance;
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedSpread;

    std::array<double, lfoCount> lfoPhase { 0.0, 0.25, 0.5, 0.75 };
    std::array<float, lfoCount> lfoValues { 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<bool, lfoCount> lfoOneShotComplete { false, false, false, false };
    std::array<float, auxEnvelopeCount> auxEnvelopeValues { 0.0f, 0.0f, 0.0f };

    juce::Random noiseRandom { 0x4f4d4e49 };
    double currentSampleRate { 44100.0 };
    int currentMidiNote { 60 };
    int pitchWheel { 8192 };
    int activeFilterMode { -1 };
    float noteVelocity { 0.0f };
    float modWheel { 0.0f };
    float aftertouch { 0.0f };
    float brownState { 0.0f };
    float stochasticState { 0.0f };
    float stochasticTarget { 0.0f };
    int stochasticSamplesUntilTarget { 1 };
    float momentPhase { 0.0f };
    float previousDriveInputL { 0.0f };
    float previousDriveInputR { 0.0f };
};
} // namespace omnaria
