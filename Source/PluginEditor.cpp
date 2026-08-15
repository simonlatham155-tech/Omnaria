#include "PluginEditor.h"
#include <array>

namespace
{
const auto accent = juce::Colour::fromRGB(150, 100, 255);
const auto panelColour = juce::Colour::fromRGB(20, 19, 30);
const auto panelEdge = juce::Colour::fromRGB(57, 49, 78);

const juce::StringArray modulationSources {
    "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3",
    "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4",
    "Brown", "Stochastic"
};
const juce::StringArray modulationDestinations {
    "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width",
    "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment"
};
}

ParamKnob::ParamKnob(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID, const juce::String& displayName)
{
    label.setText(displayName.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.60f));
    addAndMakeVisible(label);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
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

ParamCombo::ParamCombo(juce::AudioProcessorValueTreeState& state, const juce::String& parameterID,
                       const juce::String& displayName, const juce::StringArray& choices)
{
    label.setText(displayName.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.58f));
    addAndMakeVisible(label);
    for (int i = 0; i < choices.size(); ++i) combo.addItem(choices[i], i + 1);
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
    : AudioProcessorEditor(&p), processor(p), globe(p.getEngineState()),
      oscAShape(p.parameters, "oscA_shape", "Osc A", { "Saw", "Pulse", "Sine" }),
      oscBShape(p.parameters, "oscB_shape", "Osc B", { "Saw", "Pulse", "Sine" }),
      phaseMode(p.parameters, "phase_mode", "Phase Mode", { "Retrig", "Random" }),
      oscMix(p.parameters, "osc_mix", "Mix"), oscBCoarse(p.parameters, "oscB_coarse", "B Tune"),
      pulseWidth(p.parameters, "pulse_width", "Pulse"), phase(p.parameters, "phase", "Phase"),
      unison(p.parameters, "unison", "Unison"), detune(p.parameters, "detune", "Detune"), spread(p.parameters, "spread", "Spread"),
      subLevel(p.parameters, "sub_level", "Sub"), subOctave(p.parameters, "sub_octave", "Sub Oct"), noiseLevel(p.parameters, "noise_level", "Noise"),
      filterMode(p.parameters, "filter_mode", "Filter", { "LP12", "LP24", "HP12", "BP12" }),
      cutoff(p.parameters, "cutoff", "Cutoff"), resonance(p.parameters, "resonance", "Resonance"), keytrack(p.parameters, "keytrack", "Keytrack"),
      drive(p.parameters, "drive", "Drive"), filterEnvAmount(p.parameters, "filter_env_amt", "Filt Env"), velocityTimbre(p.parameters, "velocity_timbre", "Velocity"),
      filterAttack(p.parameters, "filter_attack", "F Attack"), filterDecay(p.parameters, "filter_decay", "F Decay"),
      filterSustain(p.parameters, "filter_sustain", "F Sustain"), filterRelease(p.parameters, "filter_release", "F Release"),
      attack(p.parameters, "attack", "Attack"), decay(p.parameters, "decay", "Decay"), sustain(p.parameters, "sustain", "Sustain"), release(p.parameters, "release", "Release"),
      motion(p.parameters, "motion", "Motion"), history(p.parameters, "history", "History"), focus(p.parameters, "focus", "Focus"),
      coupling(p.parameters, "coupling", "Coupling"), output(p.parameters, "output", "Output"),
      nastyModel(p.parameters, "nasty_model", "NASTY Model", { "Fold", "Feedback", "Coupled", "Duffing" }),
      nastyAmount(p.parameters, "nasty_amount", "Amount"), nastyDeform(p.parameters, "nasty_deform", "Deform"),
      nastyFeedback(p.parameters, "nasty_feedback", "Feedback"), nastyCoupling(p.parameters, "nasty_coupling", "Coupling"),
      nastyEnergy(p.parameters, "nasty_energy", "Energy"), nastyDamping(p.parameters, "nasty_damping", "Damping"),
      nastyMoment(p.parameters, "nasty_moment", "Moment")
{
    setSize(1280, 980);
    setResizable(true, false);
    setResizeLimits(1180, 860, 1800, 1250);

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

    const std::array<juce::Component*, 42> components {
        &globe, &oscAShape, &oscBShape, &phaseMode, &oscMix, &oscBCoarse, &pulseWidth, &phase,
        &unison, &detune, &spread, &subLevel, &subOctave, &noiseLevel,
        &filterMode, &cutoff, &resonance, &keytrack, &drive, &filterEnvAmount, &velocityTimbre,
        &filterAttack, &filterDecay, &filterSustain, &filterRelease, &attack, &decay, &sustain, &release,
        &motion, &history, &focus, &coupling, &output,
        &nastyModel, &nastyAmount, &nastyDeform, &nastyFeedback, &nastyCoupling, &nastyEnergy, &nastyDamping, &nastyMoment
    };
    for (auto* component : components) addAndMakeVisible(*component);

    for (int i = 0; i < 4; ++i)
    {
        const auto suffix = juce::String(i + 1);
        lfoRates[i] = std::make_unique<ParamKnob>(p.parameters, "lfo" + suffix + "_rate", "LFO " + suffix + " Hz");
        lfoModes[i] = std::make_unique<ParamCombo>(p.parameters, "lfo" + suffix + "_mode", "LFO " + suffix, juce::StringArray { "Free", "Retrig", "One Shot" });
        macros[i] = std::make_unique<ParamKnob>(p.parameters, "macro" + suffix, "Macro " + suffix);
        modSources[i] = std::make_unique<ParamCombo>(p.parameters, "mod" + suffix + "_source", "Source " + suffix, modulationSources);
        modDestinations[i] = std::make_unique<ParamCombo>(p.parameters, "mod" + suffix + "_dest", "Destination " + suffix, modulationDestinations);
        modDepths[i] = std::make_unique<ParamKnob>(p.parameters, "mod" + suffix + "_depth", "Depth " + suffix);
        addAndMakeVisible(*lfoRates[i]); addAndMakeVisible(*lfoModes[i]); addAndMakeVisible(*macros[i]);
        addAndMakeVisible(*modSources[i]); addAndMakeVisible(*modDestinations[i]); addAndMakeVisible(*modDepths[i]);
    }

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
    auto modulationStrip = body.removeFromBottom(246.0f).reduced(14.0f, 8.0f);
    auto nastyStrip = body.removeFromBottom(124.0f).reduced(14.0f, 6.0f);
    auto stateStrip = body.removeFromBottom(118.0f).reduced(14.0f, 6.0f);
    body = body.reduced(14.0f, 8.0f);
    auto left = body.removeFromLeft(350.0f);
    auto right = body.removeFromRight(430.0f);
    auto centre = body.reduced(10.0f, 0.0f);
    for (const auto& panel : { left, centre, right, stateStrip, nastyStrip, modulationStrip })
    {
        g.setColour(panelColour); g.fillRoundedRectangle(panel, 15.0f);
        g.setColour(panelEdge.withAlpha(0.80f)); g.drawRoundedRectangle(panel, 15.0f, 1.0f);
    }
    g.setColour(juce::Colours::white.withAlpha(0.48f));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("CORE OSCILLATORS", left.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("ENGINE STATE", centre.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("FILTER / EXPRESSION / AMP", right.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("PERFORMANCE", stateStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("NASTY — MOMENT ENGINE", nastyStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("MODULATION — VISIBLE ROUTING", modulationStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.setFont(juce::FontOptions(15.0f)); g.drawText("LATHAM", 20, 20, 90, 24, juce::Justification::centredLeft, false);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold)); g.drawText("AUDIO", 78, 20, 74, 24, juce::Justification::centredLeft, false);
}

void OmnariaAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    title.setBounds(bounds.getCentreX() - 150, 10, 300, 38);
    subtitle.setBounds(bounds.getCentreX() - 150, 45, 300, 18);
    discoverButton.setBounds(bounds.getRight() - 132, 20, 108, 34);

    auto body = bounds;
    body.removeFromTop(76);
    auto modulationStrip = body.removeFromBottom(246).reduced(14, 8);
    auto nastyStrip = body.removeFromBottom(124).reduced(14, 6);
    auto stateStrip = body.removeFromBottom(118).reduced(14, 6);
    body = body.reduced(14, 8);

    auto left = body.removeFromLeft(350).reduced(12);
    auto right = body.removeFromRight(430).reduced(12);
    auto centre = body.reduced(10, 8);
    left.removeFromTop(24);
    auto comboRow = left.removeFromTop(50);
    const auto comboCell = comboRow.getWidth() / 3;
    oscAShape.setBounds(comboRow.removeFromLeft(comboCell)); oscBShape.setBounds(comboRow.removeFromLeft(comboCell)); phaseMode.setBounds(comboRow);
    const auto oscRowHeight = juce::jmax(70, left.getHeight() / 3);
    auto oscRow1 = left.removeFromTop(oscRowHeight); auto oscCell1 = oscRow1.getWidth() / 4;
    oscMix.setBounds(oscRow1.removeFromLeft(oscCell1)); oscBCoarse.setBounds(oscRow1.removeFromLeft(oscCell1)); pulseWidth.setBounds(oscRow1.removeFromLeft(oscCell1)); phase.setBounds(oscRow1);
    auto oscRow2 = left.removeFromTop(oscRowHeight); auto oscCell2 = oscRow2.getWidth() / 3;
    unison.setBounds(oscRow2.removeFromLeft(oscCell2)); detune.setBounds(oscRow2.removeFromLeft(oscCell2)); spread.setBounds(oscRow2);
    auto oscRow3 = left; auto oscCell3 = oscRow3.getWidth() / 3;
    subLevel.setBounds(oscRow3.removeFromLeft(oscCell3)); subOctave.setBounds(oscRow3.removeFromLeft(oscCell3)); noiseLevel.setBounds(oscRow3);

    centre.removeFromTop(18); globe.setBounds(centre);
    right.removeFromTop(24); filterMode.setBounds(right.removeFromTop(50));
    const auto rightRowHeight = juce::jmax(70, right.getHeight() / 3);
    auto toneRow = right.removeFromTop(rightRowHeight); const auto toneCell = toneRow.getWidth() / 6;
    cutoff.setBounds(toneRow.removeFromLeft(toneCell)); resonance.setBounds(toneRow.removeFromLeft(toneCell)); keytrack.setBounds(toneRow.removeFromLeft(toneCell));
    filterEnvAmount.setBounds(toneRow.removeFromLeft(toneCell)); velocityTimbre.setBounds(toneRow.removeFromLeft(toneCell)); drive.setBounds(toneRow);
    auto filterEnvRow = right.removeFromTop(rightRowHeight); const auto filterEnvCell = filterEnvRow.getWidth() / 4;
    filterAttack.setBounds(filterEnvRow.removeFromLeft(filterEnvCell)); filterDecay.setBounds(filterEnvRow.removeFromLeft(filterEnvCell));
    filterSustain.setBounds(filterEnvRow.removeFromLeft(filterEnvCell)); filterRelease.setBounds(filterEnvRow);
    auto ampRow = right; const auto ampCell = ampRow.getWidth() / 4;
    attack.setBounds(ampRow.removeFromLeft(ampCell)); decay.setBounds(ampRow.removeFromLeft(ampCell)); sustain.setBounds(ampRow.removeFromLeft(ampCell)); release.setBounds(ampRow);

    stateStrip.removeFromTop(24); const auto stateCell = stateStrip.getWidth() / 5;
    motion.setBounds(stateStrip.removeFromLeft(stateCell)); history.setBounds(stateStrip.removeFromLeft(stateCell)); focus.setBounds(stateStrip.removeFromLeft(stateCell));
    coupling.setBounds(stateStrip.removeFromLeft(stateCell)); output.setBounds(stateStrip);

    nastyStrip.removeFromTop(24);
    auto nastyModelArea = nastyStrip.removeFromLeft(175).reduced(5, 0); nastyModel.setBounds(nastyModelArea);
    const auto nastyCellWidth = nastyStrip.getWidth() / 7;
    nastyAmount.setBounds(nastyStrip.removeFromLeft(nastyCellWidth)); nastyDeform.setBounds(nastyStrip.removeFromLeft(nastyCellWidth));
    nastyFeedback.setBounds(nastyStrip.removeFromLeft(nastyCellWidth)); nastyCoupling.setBounds(nastyStrip.removeFromLeft(nastyCellWidth));
    nastyEnergy.setBounds(nastyStrip.removeFromLeft(nastyCellWidth)); nastyDamping.setBounds(nastyStrip.removeFromLeft(nastyCellWidth)); nastyMoment.setBounds(nastyStrip);

    modulationStrip.removeFromTop(24);
    auto sourceRow = modulationStrip.removeFromTop(82); const auto sourceCell = sourceRow.getWidth() / 8;
    for (int i = 0; i < 4; ++i) lfoRates[i]->setBounds(sourceRow.removeFromLeft(sourceCell));
    for (int i = 0; i < 4; ++i) macros[i]->setBounds(sourceRow.removeFromLeft(sourceCell));
    auto modeRow = modulationStrip.removeFromTop(44); const auto modeCell = modeRow.getWidth() / 4;
    for (int i = 0; i < 4; ++i) lfoModes[i]->setBounds(modeRow.removeFromLeft(modeCell).reduced(4, 0));
    auto routeRow = modulationStrip; const auto routeCellWidth = routeRow.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto slot = routeRow.removeFromLeft(routeCellWidth).reduced(5, 0);
        auto depthArea = slot.removeFromRight(78);
        auto sourceArea = slot.removeFromTop(slot.getHeight() / 2);
        modSources[i]->setBounds(sourceArea); modDestinations[i]->setBounds(slot); modDepths[i]->setBounds(depthArea);
    }
}
