#pragma once

#include <JuceHeader.h>
#include <array>
#include "PluginProcessor.h"
#include "DiscoverEngine.h"
#include "Phase7Discover.h"
#include "InstructionEngine.h"
#include "ui/WorldGlobe.h"

class ParamKnob final : public juce::Component
{
public:
    ParamKnob(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID, const juce::String& displayName);
    void resized() override;
    juce::Slider slider;
    juce::Label label;
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class ParamCombo final : public juce::Component
{
public:
    ParamCombo(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID,
               const juce::String& displayName, const juce::StringArray& choices);
    void resized() override;
private:
    juce::Label label;
    juce::ComboBox combo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class OmnariaAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OmnariaAudioProcessorEditor(OmnariaAudioProcessor&);
    ~OmnariaAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

    // setSize can be called while the constructor body is still running. Queue the
    // host-visible resize so the editor is fully constructed first.
    void setSize(int width, int height)
    {
        juce::Component::SafePointer<OmnariaAudioProcessorEditor> safeThis(this);
        juce::MessageManager::callAsync([safeThis, width, height]
        {
            if (safeThis != nullptr)
                safeThis->juce::AudioProcessorEditor::setSize(width, height);
        });
    }

private:
    void showSpecialistPage(int page);
    void refreshSampleName();
    unsigned int getDiscoverLocks() const noexcept;
    void persistDiscoverSettings();
    void refreshDiscoverUndo();

    void timerCallback() override;
    void cycleInstructionMode();
    void setInstructionMode(InstructionEngine::Mode mode, bool persist);
    void showInstruction(const InstructionEngine::Suggestion& suggestion);
    void bindInstruction(ParamKnob& knob, const juce::String& parameterID);

    OmnariaAudioProcessor& processor;
    DiscoverEngine discoverEngine;
    InstructionEngine instructionEngine;
    omnaria::WorldGlobe globe;

    ParamCombo oscAShape, oscBShape, phaseMode;
    ParamKnob oscMix, oscBCoarse, pulseWidth, phase, unison, detune, spread, subLevel, subOctave, noiseLevel;
    ParamCombo filterMode, filterCharacter;
    ParamKnob filterCharacterAmount, cutoff, resonance, keytrack, drive, filterEnvAmount, velocityTimbre;
    ParamKnob filterAttack, filterDecay, filterSustain, filterRelease, attack, decay, sustain, release;
    ParamKnob motion, history, focus, coupling, output;

    ParamCombo nastyModel;
    ParamKnob nastyAmount, nastyDeform, nastyFeedback, nastyCoupling, nastyEnergy, nastyDamping, nastyMoment;

    ParamCombo sampleMode;
    ParamKnob sampleLevel, sampleTune, sampleStart, sampleEnd, samplePosition, sampleScan, sampleJitter;
    juce::ToggleButton sampleReverse { "REVERSE" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sampleReverseAttachment;
    juce::TextButton loadSampleButton { "LOAD" };
    juce::TextButton captureSampleButton { "RESAMPLE" };
    juce::Label sampleNameLabel;
    std::unique_ptr<juce::FileChooser> sampleChooser;

    ParamCombo fxOrder, fxDelayDivision;
    ParamKnob fxMotionMix, fxMotionRate, fxMotionDepth, fxDelayMix, fxDelayFeedback, fxSpaceMix, fxSpaceSize, fxSpaceDamping, fxWidth;
    juce::TextButton nastyTabButton { "NASTY" };
    juce::TextButton sampleTabButton { "SAMPLE" };
    juce::TextButton fxTabButton { "FX" };
    int specialistPage { 0 };

    // These controls are laid out by resized(). JUCE may call resized() from
    // setResizable/setResizeLimits before the constructor body reaches its normal
    // dynamic-creation loop. Preconstruct valid controls here so no early layout
    // path can ever dereference a null unique_ptr. The constructor later replaces
    // them with the normal configured instances.
    std::array<std::unique_ptr<ParamKnob>, 4> lfoRates {
        std::make_unique<ParamKnob>(processor.parameters, "lfo1_rate", "LFO 1 Hz"),
        std::make_unique<ParamKnob>(processor.parameters, "lfo2_rate", "LFO 2 Hz"),
        std::make_unique<ParamKnob>(processor.parameters, "lfo3_rate", "LFO 3 Hz"),
        std::make_unique<ParamKnob>(processor.parameters, "lfo4_rate", "LFO 4 Hz")
    };
    std::array<std::unique_ptr<ParamCombo>, 4> lfoModes {
        std::make_unique<ParamCombo>(processor.parameters, "lfo1_mode", "LFO 1", juce::StringArray { "Free", "Retrig", "One Shot" }),
        std::make_unique<ParamCombo>(processor.parameters, "lfo2_mode", "LFO 2", juce::StringArray { "Free", "Retrig", "One Shot" }),
        std::make_unique<ParamCombo>(processor.parameters, "lfo3_mode", "LFO 3", juce::StringArray { "Free", "Retrig", "One Shot" }),
        std::make_unique<ParamCombo>(processor.parameters, "lfo4_mode", "LFO 4", juce::StringArray { "Free", "Retrig", "One Shot" })
    };
    std::array<std::unique_ptr<ParamKnob>, 4> macros {
        std::make_unique<ParamKnob>(processor.parameters, "macro1", "Macro 1"),
        std::make_unique<ParamKnob>(processor.parameters, "macro2", "Macro 2"),
        std::make_unique<ParamKnob>(processor.parameters, "macro3", "Macro 3"),
        std::make_unique<ParamKnob>(processor.parameters, "macro4", "Macro 4")
    };
    std::array<std::unique_ptr<ParamCombo>, 4> modSources {
        std::make_unique<ParamCombo>(processor.parameters, "mod1_source", "Source 1", juce::StringArray { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3", "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod2_source", "Source 2", juce::StringArray { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3", "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod3_source", "Source 3", juce::StringArray { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3", "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod4_source", "Source 4", juce::StringArray { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3", "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic" })
    };
    std::array<std::unique_ptr<ParamCombo>, 4> modDestinations {
        std::make_unique<ParamCombo>(processor.parameters, "mod1_dest", "Destination 1", juce::StringArray { "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width", "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment", "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod2_dest", "Destination 2", juce::StringArray { "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width", "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment", "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod3_dest", "Destination 3", juce::StringArray { "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width", "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment", "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune" }),
        std::make_unique<ParamCombo>(processor.parameters, "mod4_dest", "Destination 4", juce::StringArray { "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width", "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment", "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune" })
    };
    std::array<std::unique_ptr<ParamKnob>, 4> modDepths {
        std::make_unique<ParamKnob>(processor.parameters, "mod1_depth", "Depth 1"),
        std::make_unique<ParamKnob>(processor.parameters, "mod2_depth", "Depth 2"),
        std::make_unique<ParamKnob>(processor.parameters, "mod3_depth", "Depth 3"),
        std::make_unique<ParamKnob>(processor.parameters, "mod4_depth", "Depth 4")
    };

    juce::Label title;
    juce::Label subtitle;

    juce::TextButton discoverButton { "DISCOVER" };
    juce::TextButton undoDiscoverButton { "UNDO" };
    juce::Slider discoverWtf;
    juce::Label familiarLabel;
    juce::Label wtfLabel;
    juce::Label lockLabel;
    juce::TextButton lockCoreButton { "CORE" };
    juce::TextButton lockNastyButton { "NASTY" };
    juce::TextButton lockSampleButton { "SAMPLE" };
    juce::TextButton lockModButton { "MOD" };

    juce::TextButton instructionModeButton { "?" };
    juce::Label instructionTitle;
    juce::Label instructionText;
    juce::TextButton instructionTryButton { "TRY IT" };
    juce::TextButton instructionUndoButton { "UNDO TRY" };
    InstructionEngine::Suggestion currentInstruction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaAudioProcessorEditor)
};
