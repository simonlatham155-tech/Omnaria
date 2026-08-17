#pragma once
#include <JuceHeader.h>

struct Engine3Params
{
    std::atomic<float>* partialDensity{};
    std::atomic<float>* spectralTilt{};
    std::atomic<float>* modulationDepth{};
    std::atomic<float>* resonance{};
    std::atomic<float>* excitation{};
    std::atomic<float>* nonlinearity{};
    std::atomic<float>* asymmetry{};
    std::atomic<float>* attack{};
    std::atomic<float>* decay{};
    std::atomic<float>* sustain{};
    std::atomic<float>* release{};
};

class Engine3Sound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class Engine3Voice : public juce::SynthesiserVoice
{
public:
    explicit Engine3Voice(Engine3Params p);
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void prepare(double sampleRate);
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

private:
    Engine3Params params;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    double sr = 44100.0;
    double baseFrequency = 110.0;
    float noteVelocity = 1.0f;
    std::array<double, 24> phases{};
    double modPhase = 0.0;
    double crossPhase = 0.31;
    double smoothedNoise = 0.0;
    double dcIn = 0.0;
    double dcOut = 0.0;
    uint32_t noiseState = 0x12345678u;

    float noise();
    static float edgeWarp(float u);
};
