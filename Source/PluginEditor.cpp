#include "PluginEditor.h"
#include <array>

namespace
{
const auto accent = juce::Colour::fromRGB(150, 100, 255);
const auto panelColour = juce::Colour::fromRGB(20, 19, 30);
const auto panelEdge = juce::Colour::fromRGB(57, 49, 78);
}

ParamKnob::ParamKnob(juce::AudioProcessorValueTreeState& state,
                     const juce::String& parameterID,
                     const juce::String& displayName)
{
    label.setText(displayName.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.60f));
    addAndMakeVisible(label);

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::white.withAlpha(0.10f));
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white.withAlpha(0.90f));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white.withAlpha(0.78f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, parameterID, slider);
}

void ParamKnob::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromTop(18));
    slider.setBounds(area);
}

ParamCombo::ParamCombo(juce::AudioProcessorValueTreeState& state,
                       const juce::String& parameterID,
                       const juce::String& displayName,
                       const juce::StringArray& choices)
{
    label.setText(displayName.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.58f));
    addAndMakeVisible(label);

    for (int i = 0; i < choices.size(); ++i)
        combo.addItem(choices[i], i + 1);

    combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(12, 12, 19));
    combo.setColour(juce::ComboBox::outlineColourId, panelEdge);
    combo.setColour(juce::ComboBox::textColourId, juce::Colours::white.withAlpha(0.82f));
    combo.setColour(juce::ComboBox::arrowColourId, accent);
    addAndMakeVisible(combo);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, parameterID, combo);
}

void ParamCombo::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromTop(17));
    combo.setBounds(area.reduced(0, 2));
}

OmnariaAudioProcessorEditor::OmnariaAudioProcessorEditor(OmnariaAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      globe(p.getEngineState()),
      oscAShape(p.parameters, "oscA_shape", "Osc A", { "Saw", "Pulse", "Sine" }),
      oscBShape(p.parameters, "oscB_shape", "Osc B", { "Saw", "Pulse", "Sine" }),
      oscMix(p.parameters, "osc_mix", "Mix"),
      oscBCoarse(p.parameters, "oscB_coarse", "B Tune"),
      unison(p.parameters, "unison", "Unison"),
      detune(p.parameters, "detune", "Detune"),
      spread(p.parameters, "spread", "Spread"),
      cutoff(p.parameters, "cutoff", "Cutoff"),
      resonance(p.parameters, "resonance", "Resonance"),
      drive(p.parameters, "drive", "Drive"),
      filterEnvAmount(p.parameters, "filter_env_amt", "Filt Env"),
      velocityTimbre(p.parameters, "velocity_timbre", "Velocity"),
      filterAttack(p.parameters, "filter_attack", "F Attack"),
      filterDecay(p.parameters, "filter_decay", "F Decay"),
      filterSustain(p.parameters, "filter_sustain", "F Sustain"),
      filterRelease(p.parameters, "filter_release", "F Release"),
      attack(p.parameters, "attack", "Attack"),
      decay(p.parameters, "decay", "Decay"),
      sustain(p.parameters, "sustain", "Sustain"),
      release(p.parameters, "release", "Release"),
      motion(p.parameters, "motion", "Motion"),
      history(p.parameters, "history", "History"),
      focus(p.parameters, "focus", "Focus"),
      coupling(p.parameters, "coupling", "Coupling"),
      output(p.parameters, "output", "Output")
{
    setSize(1280, 760);
    setResizable(true, false);
    setResizeLimits(1040, 680, 1800, 1100);

    title.setText("OMNARIA", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.96f));
    addAndMakeVisible(title);

    subtitle.setText("FLAGSHIP SYNTHESIZER", juce::dontSendNotification);
    subtitle.setJustificationType(juce::Justification::centred);
    subtitle.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    subtitle.setColour(juce::Label::textColourId, accent.withAlpha(0.92f));
    addAndMakeVisible(subtitle);

    const std::array<juce::Component*, 26> components {
        &globe,
        &oscAShape, &oscBShape, &oscMix, &oscBCoarse, &unison, &detune, &spread,
        &cutoff, &resonance, &drive, &filterEnvAmount, &velocityTimbre,
        &filterAttack, &filterDecay, &filterSustain, &filterRelease,
        &attack, &decay, &sustain, &release,
        &motion, &history, &focus, &coupling, &output
    };

    for (auto* component : components)
        addAndMakeVisible(*component);

    discoverButton.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.22f));
    discoverButton.setColour(juce::TextButton::buttonOnColourId, accent.withAlpha(0.34f));
    discoverButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.90f));
    discoverButton.onClick = [this] { processor.randomiseDiscoverable(); };
    addAndMakeVisible(discoverButton);
}

void OmnariaAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(8, 8, 13));

    auto body = getLocalBounds().toFloat();
    body.removeFromTop(76.0f);
    auto stateStrip = body.removeFromBottom(166.0f).reduced(14.0f, 10.0f);
    body = body.reduced(14.0f, 8.0f);

    auto left = body.removeFromLeft(286.0f);
    auto right = body.removeFromRight(390.0f);
    auto centre = body.reduced(10.0f, 0.0f);

    for (const auto& panel : { left, centre, right, stateStrip })
    {
        g.setColour(panelColour);
        g.fillRoundedRectangle(panel, 15.0f);
        g.setColour(panelEdge.withAlpha(0.80f));
        g.drawRoundedRectangle(panel, 15.0f, 1.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.48f));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("CORE ENGINE", left.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("ENGINE STATE", centre.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("FILTER / EXPRESSION / AMP", right.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("PERFORMANCE", stateStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.setFont(juce::FontOptions(15.0f));
    g.drawText("LATHAM", 20, 20, 90, 24, juce::Justification::centredLeft, false);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("AUDIO", 78, 20, 74, 24, juce::Justification::centredLeft, false);
}

void OmnariaAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    title.setBounds(bounds.getCentreX() - 150, 10, 300, 38);
    subtitle.setBounds(bounds.getCentreX() - 150, 45, 300, 18);
    discoverButton.setBounds(bounds.getRight() - 132, 20, 108, 34);

    auto body = bounds;
    body.removeFromTop(76);
    auto stateStrip = body.removeFromBottom(166).reduced(14, 10);
    body = body.reduced(14, 8);

    auto left = body.removeFromLeft(286).reduced(12);
    auto right = body.removeFromRight(390).reduced(12);
    auto centre = body.reduced(10, 8);

    left.removeFromTop(24);
    oscAShape.setBounds(left.removeFromTop(50));
    oscBShape.setBounds(left.removeFromTop(50));
    left.removeFromTop(6);

    auto leftRow1 = left.removeFromTop(left.getHeight() / 2);
    const auto leftCellWidth = leftRow1.getWidth() / 3;
    oscMix.setBounds(leftRow1.removeFromLeft(leftCellWidth));
    oscBCoarse.setBounds(leftRow1.removeFromLeft(leftCellWidth));
    unison.setBounds(leftRow1);

    auto leftRow2 = left;
    const auto leftCellWidth2 = leftRow2.getWidth() / 2;
    detune.setBounds(leftRow2.removeFromLeft(leftCellWidth2));
    spread.setBounds(leftRow2);

    centre.removeFromTop(18);
    globe.setBounds(centre);

    right.removeFromTop(24);
    const auto rowHeight = juce::jmax(78, right.getHeight() / 3);

    auto filterRow = right.removeFromTop(rowHeight);
    const auto filterCell = filterRow.getWidth() / 5;
    cutoff.setBounds(filterRow.removeFromLeft(filterCell));
    resonance.setBounds(filterRow.removeFromLeft(filterCell));
    filterEnvAmount.setBounds(filterRow.removeFromLeft(filterCell));
    velocityTimbre.setBounds(filterRow.removeFromLeft(filterCell));
    drive.setBounds(filterRow);

    auto filterEnvRow = right.removeFromTop(rowHeight);
    const auto filterEnvCell = filterEnvRow.getWidth() / 4;
    filterAttack.setBounds(filterEnvRow.removeFromLeft(filterEnvCell));
    filterDecay.setBounds(filterEnvRow.removeFromLeft(filterEnvCell));
    filterSustain.setBounds(filterEnvRow.removeFromLeft(filterEnvCell));
    filterRelease.setBounds(filterEnvRow);

    auto ampRow = right;
    const auto ampCell = ampRow.getWidth() / 4;
    attack.setBounds(ampRow.removeFromLeft(ampCell));
    decay.setBounds(ampRow.removeFromLeft(ampCell));
    sustain.setBounds(ampRow.removeFromLeft(ampCell));
    release.setBounds(ampRow);

    stateStrip.removeFromTop(24);
    const auto stateCell = stateStrip.getWidth() / 5;
    motion.setBounds(stateStrip.removeFromLeft(stateCell));
    history.setBounds(stateStrip.removeFromLeft(stateCell));
    focus.setBounds(stateStrip.removeFromLeft(stateCell));
    coupling.setBounds(stateStrip.removeFromLeft(stateCell));
    output.setBounds(stateStrip);
}
