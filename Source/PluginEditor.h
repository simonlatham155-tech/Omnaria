#pragma once

#include <JuceHeader.h>
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
    ParamKnob oscMix;
    ParamKnob oscBCoarse;
    ParamKnob unison;
    ParamKnob detune;
    ParamKnob spread;

    ParamKnob cutoff;
    ParamKnob resonance;
    ParamKnob drive;
    ParamKnob attack;
    ParamKnob decay;
    ParamKnob sustain;
    ParamKnob release;

    ParamKnob motion;
    ParamKnob history;
    ParamKnob focus;
    ParamKnob coupling;
    ParamKnob output;

    juce::Label title;
    juce::Label subtitle;
    juce::TextButton discoverButton { "DISCOVER" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaAudioProcessorEditor)
};
