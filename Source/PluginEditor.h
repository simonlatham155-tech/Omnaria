#pragma once

#include <JuceHeader.h>
#include <array>
#include "PluginProcessor.h"
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
    ParamCombo(juce::AudioProcessorValueTreeState& state,
               const juce::String& parameterID,
               const juce::String& displayName,
               const juce::StringArray& choices);
    void resized() override;

private:
    juce::Label label;
    juce::ComboBox combo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class OmnariaAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit OmnariaAudioProcessorEditor(OmnariaAudioProcessor&);
    ~OmnariaAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OmnariaAudioProcessor& processor;
    omnaria::WorldGlobe globe;

    ParamCombo oscAShape;
    ParamCombo oscBShape;
    ParamCombo phaseMode;
    ParamKnob oscMix;
    ParamKnob oscBCoarse;
    ParamKnob pulseWidth;
    ParamKnob phase;
    ParamKnob unison;
    ParamKnob detune;
    ParamKnob spread;
    ParamKnob subLevel;
    ParamKnob subOctave;
    ParamKnob noiseLevel;

    ParamCombo filterMode;
    ParamKnob cutoff;
    ParamKnob resonance;
    ParamKnob keytrack;
    ParamKnob drive;
    ParamKnob filterEnvAmount;
    ParamKnob velocityTimbre;
    ParamKnob filterAttack;
    ParamKnob filterDecay;
    ParamKnob filterSustain;
    ParamKnob filterRelease;

    ParamKnob attack;
    ParamKnob decay;
    ParamKnob sustain;
    ParamKnob release;

    ParamKnob motion;
    ParamKnob history;
    ParamKnob focus;
    ParamKnob coupling;
    ParamKnob output;

    std::array<std::unique_ptr<ParamKnob>, 4> lfoRates;
    std::array<std::unique_ptr<ParamCombo>, 4> lfoModes;
    std::array<std::unique_ptr<ParamKnob>, 4> macros;
    std::array<std::unique_ptr<ParamCombo>, 4> modSources;
    std::array<std::unique_ptr<ParamCombo>, 4> modDestinations;
    std::array<std::unique_ptr<ParamKnob>, 4> modDepths;

    juce::Label title;
    juce::Label subtitle;
    juce::TextButton discoverButton { "DISCOVER" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaAudioProcessorEditor)
};
