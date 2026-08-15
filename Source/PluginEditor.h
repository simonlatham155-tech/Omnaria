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

    std::array<std::unique_ptr<ParamKnob>, 4> lfoRates;
    std::array<std::unique_ptr<ParamCombo>, 4> lfoModes;
    std::array<std::unique_ptr<ParamKnob>, 4> macros;
    std::array<std::unique_ptr<ParamCombo>, 4> modSources;
    std::array<std::unique_ptr<ParamCombo>, 4> modDestinations;
    std::array<std::unique_ptr<ParamKnob>, 4> modDepths;

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
