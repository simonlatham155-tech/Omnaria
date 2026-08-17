#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OmnariaEngine3TestAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OmnariaEngine3TestAudioProcessorEditor(OmnariaEngine3TestAudioProcessor&);
    ~OmnariaEngine3TestAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OmnariaEngine3TestAudioProcessor& processor;
    std::array<juce::Slider, 11> sliders;
    std::array<juce::Label, 11> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    static constexpr std::array<const char*, 11> ids {{
        "partialDensity", "spectralTilt", "modDepth", "resonance", "excitation", "nonlinearity", "asymmetry",
        "attack", "decay", "sustain", "release"
    }};
    static constexpr std::array<const char*, 11> names {{
        "DENSITY", "TILT", "MOD", "RESONANCE", "EXCITE", "DRIVE", "ASYMMETRY",
        "ATTACK", "DECAY", "SUSTAIN", "RELEASE"
    }};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmnariaEngine3TestAudioProcessorEditor)
};
