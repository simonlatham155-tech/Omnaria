#pragma once

#include <JuceHeader.h>
#include "dsp/OmnariaStateEngine.h"
#include "dsp/OmnariaVoice.h"
#include "dsp/SamplePool.h"

class OmnariaAudioProcessor final : public juce::AudioProcessor
{
public:
    OmnariaAudioProcessor();
    ~OmnariaAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void randomiseDiscoverable();
    bool loadSampleFile(const juce::File& file);
    bool captureRecentOutput();
    juce::String getSampleName() const;
    const omnaria::OmnariaState& getEngineState() const noexcept { return engineState; }

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void setParameterFromActualValue(const juce::String& id, float actualValue);
    void writeCaptureHistory(const juce::AudioBuffer<float>& buffer);

    omnaria::OmnariaState engineState;
    omnaria::OmnariaStateEngine stateEngine;
    omnaria::SamplePool samplePool;
    juce::Synthesiser synthesiser;
    juce::dsp::Gain<float> outputGain;
    juce::AudioFormatManager formatManager;

    juce::AudioBuffer<float> captureHistory;
    int captureWritePosition { 0 };
    double currentSampleRate { 44100.0 };
    mutable juce::CriticalSection sampleNameLock;
    juce::String sampleName { "EMPTY" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaAudioProcessor)
};
