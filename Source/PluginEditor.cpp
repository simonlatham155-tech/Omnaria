#include "PluginEditor.h"

OmnariaEngine3TestAudioProcessorEditor::OmnariaEngine3TestAudioProcessorEditor(OmnariaEngine3TestAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(980, 520);
    attachments.reserve(ids.size());

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& s = sliders[i];
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(90, 215, 190));
        s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(s);

        auto& l = labels[i];
        l.setText(names[i], juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colour::fromRGB(210, 215, 220));
        l.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        addAndMakeVisible(l);

        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, ids[i], s));
    }
}

void OmnariaEngine3TestAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(16, 19, 22));
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour::fromRGB(28, 33, 38));
    g.fillRoundedRectangle(bounds.reduced(18.0f), 14.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(30.0f, juce::Font::plain));
    g.drawText("LATHAM", 34, 22, 122, 36, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    g.drawText("AUDIO", 146, 22, 120, 36, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(90, 215, 190));
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("OMNARIA ENGINE 3 — TEST SYNTH", 34, 66, 500, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(155, 165, 172));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("Qualified shared-primitive core • 8 voices • no FX", 34, 96, 500, 22, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(45, 51, 57));
    g.drawLine(34.0f, 128.0f, static_cast<float>(getWidth() - 34), 128.0f, 1.0f);
}

void OmnariaEngine3TestAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    area.removeFromTop(120);

    auto engineRow = area.removeFromTop(180);
    const int engineW = engineRow.getWidth() / 7;
    for (int i = 0; i < 7; ++i)
    {
        auto cell = engineRow.removeFromLeft(engineW).reduced(6);
        labels[static_cast<size_t>(i)].setBounds(cell.removeFromTop(22));
        sliders[static_cast<size_t>(i)].setBounds(cell);
    }

    area.removeFromTop(16);
    auto envRow = area.removeFromTop(150);
    const int envW = envRow.getWidth() / 4;
    for (int i = 7; i < 11; ++i)
    {
        auto cell = envRow.removeFromLeft(envW).reduced(18, 4);
        labels[static_cast<size_t>(i)].setBounds(cell.removeFromTop(22));
        sliders[static_cast<size_t>(i)].setBounds(cell);
    }
}
