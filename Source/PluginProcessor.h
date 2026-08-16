#pragma once

#include <JuceHeader.h>
#include "dsp/OmnariaStateEngine.h"
#include "dsp/OmnariaVoice.h"
#include "dsp/SamplePool.h"
#include "dsp/ProductionFX.h"
#include "FactoryPresets.h"

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

    int getNumPrograms() override { return static_cast<int>(omnaria::factoryPresets().size()); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override
    {
        const auto& bank = omnaria::factoryPresets();
        if (index < 0 || index >= static_cast<int>(bank.size())) return;

        // Start every factory program from the actual parameter defaults. This prevents
        // omitted values from leaking from the previously selected patch.
        for (auto* p : getParameters())
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(p))
                ranged->setValueNotifyingHost(ranged->getDefaultValue());

        // Phase 10 normal-bank cleanup: the 48 musical presets are evaluated separately
        // from NASTY. Legacy nasty_* coefficients remain in FactoryPresets.h as design
        // history until the dedicated NASTY recipe pass, but they are deliberately not
        // applied by the normal factory bank. This guarantees a normal preset cannot
        // accidentally engage an unvalidated nonlinear recipe.
        for (const auto& value : bank[static_cast<size_t>(index)].values)
        {
            const juce::String parameterID(value.id);
            if (parameterID.startsWith("nasty_"))
                continue;

            if (auto* p = parameters.getParameter(parameterID))
                p->setValueNotifyingHost(p->convertTo0to1(value.value));
        }

        currentProgram = index;
        parameters.state.setProperty("factory_program", currentProgram, nullptr);
    }
    const juce::String getProgramName(int index) override
    {
        const auto& bank = omnaria::factoryPresets();
        return index >= 0 && index < static_cast<int>(bank.size()) ? bank[static_cast<size_t>(index)].name : juce::String();
    }
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
    omnaria::ProductionFX productionFX;
    juce::Synthesiser synthesiser;
    juce::dsp::Gain<float> outputGain;
    juce::AudioFormatManager formatManager;

    juce::AudioBuffer<float> captureHistory;
    int captureWritePosition { 0 };
    double currentSampleRate { 44100.0 };
    mutable juce::CriticalSection sampleNameLock;
    juce::String sampleName { "EMPTY" };
    int currentProgram { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaAudioProcessor)
};