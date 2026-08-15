#include "PluginEditor.h"
#include <array>

namespace
{
const auto accent = juce::Colour::fromRGB(150, 100, 255);
const auto panelColour = juce::Colour::fromRGB(20, 19, 30);
const auto panelEdge = juce::Colour::fromRGB(57, 49, 78);
const juce::StringArray modulationSources {
    "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2", "Env 3",
    "Velocity", "Key", "Mod Wheel", "Aftertouch", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Brown", "Stochastic"
};
const juce::StringArray modulationDestinations {
    "None", "Pitch", "Cutoff", "Resonance", "Osc Mix", "Detune", "Spread", "Drive", "Pulse Width",
    "NASTY Amount", "NASTY Deform", "NASTY Feedback", "NASTY Coupling", "NASTY Energy", "NASTY Damping", "NASTY Moment",
    "Sample Level", "Sample Position", "Sample Scan", "Sample Jitter", "Sample Tune"
};
void setComponentsVisible(std::initializer_list<juce::Component*> components, bool visible)
{
    for (auto* component : components) component->setVisible(visible);
}
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
void ParamKnob::resized() { auto a = getLocalBounds(); label.setBounds(a.removeFromTop(18)); slider.setBounds(a); }

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
void ParamCombo::resized() { auto a = getLocalBounds(); label.setBounds(a.removeFromTop(17)); combo.setBounds(a.reduced(0, 2)); }

OmnariaAudioProcessorEditor::OmnariaAudioProcessorEditor(OmnariaAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), discoverEngine(p.parameters), globe(p.getEngineState()),
      oscAShape(p.parameters, "oscA_shape", "Osc A", { "Saw", "Pulse", "Sine" }), oscBShape(p.parameters, "oscB_shape", "Osc B", { "Saw", "Pulse", "Sine" }),
      phaseMode(p.parameters, "phase_mode", "Phase Mode", { "Retrig", "Random" }), oscMix(p.parameters, "osc_mix", "Mix"), oscBCoarse(p.parameters, "oscB_coarse", "B Tune"),
      pulseWidth(p.parameters, "pulse_width", "Pulse"), phase(p.parameters, "phase", "Phase"), unison(p.parameters, "unison", "Unison"), detune(p.parameters, "detune", "Detune"),
      spread(p.parameters, "spread", "Spread"), subLevel(p.parameters, "sub_level", "Sub"), subOctave(p.parameters, "sub_octave", "Sub Oct"), noiseLevel(p.parameters, "noise_level", "Noise"),
      filterMode(p.parameters, "filter_mode", "Filter", { "LP12", "LP24", "HP12", "BP12" }), cutoff(p.parameters, "cutoff", "Cutoff"), resonance(p.parameters, "resonance", "Resonance"),
      keytrack(p.parameters, "keytrack", "Keytrack"), drive(p.parameters, "drive", "Drive"), filterEnvAmount(p.parameters, "filter_env_amt", "Filt Env"), velocityTimbre(p.parameters, "velocity_timbre", "Velocity"),
      filterAttack(p.parameters, "filter_attack", "F Attack"), filterDecay(p.parameters, "filter_decay", "F Decay"), filterSustain(p.parameters, "filter_sustain", "F Sustain"), filterRelease(p.parameters, "filter_release", "F Release"),
      attack(p.parameters, "attack", "Attack"), decay(p.parameters, "decay", "Decay"), sustain(p.parameters, "sustain", "Sustain"), release(p.parameters, "release", "Release"),
      motion(p.parameters, "motion", "Motion"), history(p.parameters, "history", "History"), focus(p.parameters, "focus", "Focus"), coupling(p.parameters, "coupling", "Coupling"), output(p.parameters, "output", "Output"),
      nastyModel(p.parameters, "nasty_model", "NASTY Model", { "Fold", "Feedback", "Coupled", "Duffing" }), nastyAmount(p.parameters, "nasty_amount", "Amount"),
      nastyDeform(p.parameters, "nasty_deform", "Deform"), nastyFeedback(p.parameters, "nasty_feedback", "Feedback"), nastyCoupling(p.parameters, "nasty_coupling", "Coupling"),
      nastyEnergy(p.parameters, "nasty_energy", "Energy"), nastyDamping(p.parameters, "nasty_damping", "Damping"), nastyMoment(p.parameters, "nasty_moment", "Moment"),
      sampleMode(p.parameters, "sample_mode", "Mode", { "One Shot", "Loop", "Texture" }), sampleLevel(p.parameters, "sample_level", "Level"), sampleTune(p.parameters, "sample_tune", "Tune"),
      sampleStart(p.parameters, "sample_start", "Start"), sampleEnd(p.parameters, "sample_end", "End"), samplePosition(p.parameters, "sample_position", "Position"),
      sampleScan(p.parameters, "sample_scan", "Scan"), sampleJitter(p.parameters, "sample_jitter", "Jitter")
{
    setSize(1280, 980); setResizable(true, false); setResizeLimits(1180, 860, 1800, 1250);
    title.setText("OMNARIA", juce::dontSendNotification); title.setJustificationType(juce::Justification::centred); title.setFont(juce::FontOptions(30.0f, juce::Font::bold)); title.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.96f)); addAndMakeVisible(title);
    subtitle.setText("FLAGSHIP SYNTHESIZER", juce::dontSendNotification); subtitle.setJustificationType(juce::Justification::centred); subtitle.setFont(juce::FontOptions(10.0f, juce::Font::bold)); subtitle.setColour(juce::Label::textColourId, accent.withAlpha(0.92f)); addAndMakeVisible(subtitle);

    const std::array<juce::Component*, 50> components {
        &globe, &oscAShape, &oscBShape, &phaseMode, &oscMix, &oscBCoarse, &pulseWidth, &phase, &unison, &detune, &spread, &subLevel, &subOctave, &noiseLevel,
        &filterMode, &cutoff, &resonance, &keytrack, &drive, &filterEnvAmount, &velocityTimbre, &filterAttack, &filterDecay, &filterSustain, &filterRelease,
        &attack, &decay, &sustain, &release, &motion, &history, &focus, &coupling, &output, &nastyModel, &nastyAmount, &nastyDeform, &nastyFeedback,
        &nastyCoupling, &nastyEnergy, &nastyDamping, &nastyMoment, &sampleMode, &sampleLevel, &sampleTune, &sampleStart, &sampleEnd, &samplePosition, &sampleScan, &sampleJitter
    };
    for (auto* c : components) addAndMakeVisible(*c);

    sampleReverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "sample_reverse", sampleReverse);
    sampleReverse.setColour(juce::ToggleButton::textColourId, juce::Colours::white.withAlpha(0.75f)); addAndMakeVisible(sampleReverse);
    sampleNameLabel.setJustificationType(juce::Justification::centredLeft); sampleNameLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.58f)); sampleNameLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold)); addAndMakeVisible(sampleNameLabel);

    for (auto* button : std::array<juce::TextButton*, 4> { &loadSampleButton, &captureSampleButton, &nastyTabButton, &sampleTabButton })
    {
        button->setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.16f)); button->setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.86f)); addAndMakeVisible(*button);
    }
    nastyTabButton.onClick = [this] { showSpecialistPage(false); }; sampleTabButton.onClick = [this] { showSpecialistPage(true); };
    loadSampleButton.onClick = [this]
    {
        sampleChooser = std::make_unique<juce::FileChooser>("Load sample", juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.mp3");
        sampleChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser) { if (processor.loadSampleFile(chooser.getResult())) refreshSampleName(); });
    };
    captureSampleButton.onClick = [this] { if (processor.captureRecentOutput()) refreshSampleName(); };
    refreshSampleName();

    for (int i = 0; i < 4; ++i)
    {
        const auto s = juce::String(i + 1);
        lfoRates[i] = std::make_unique<ParamKnob>(p.parameters, "lfo" + s + "_rate", "LFO " + s + " Hz");
        lfoModes[i] = std::make_unique<ParamCombo>(p.parameters, "lfo" + s + "_mode", "LFO " + s, juce::StringArray { "Free", "Retrig", "One Shot" });
        macros[i] = std::make_unique<ParamKnob>(p.parameters, "macro" + s, "Macro " + s);
        modSources[i] = std::make_unique<ParamCombo>(p.parameters, "mod" + s + "_source", "Source " + s, modulationSources);
        modDestinations[i] = std::make_unique<ParamCombo>(p.parameters, "mod" + s + "_dest", "Destination " + s, modulationDestinations);
        modDepths[i] = std::make_unique<ParamKnob>(p.parameters, "mod" + s + "_depth", "Depth " + s);
        addAndMakeVisible(*lfoRates[i]); addAndMakeVisible(*lfoModes[i]); addAndMakeVisible(*macros[i]); addAndMakeVisible(*modSources[i]); addAndMakeVisible(*modDestinations[i]); addAndMakeVisible(*modDepths[i]);
    }

    discoverButton.setColour(juce::TextButton::buttonColourId, accent.withAlpha(0.28f)); discoverButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.94f)); addAndMakeVisible(discoverButton);
    undoDiscoverButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.07f)); undoDiscoverButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.72f)); addAndMakeVisible(undoDiscoverButton);

    discoverWtf.setSliderStyle(juce::Slider::LinearHorizontal); discoverWtf.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); discoverWtf.setRange(0.0, 1.0, 0.01);
    discoverWtf.setColour(juce::Slider::trackColourId, accent.withAlpha(0.55f)); discoverWtf.setColour(juce::Slider::thumbColourId, juce::Colours::white.withAlpha(0.90f)); addAndMakeVisible(discoverWtf);
    const auto savedWtf = static_cast<double>(p.parameters.state.getProperty("discover_wtf", 0.35)); discoverWtf.setValue(savedWtf, juce::dontSendNotification);

    familiarLabel.setText("FAM", juce::dontSendNotification); wtfLabel.setText("WTF", juce::dontSendNotification); lockLabel.setText("LOCK", juce::dontSendNotification);
    for (auto* label : std::array<juce::Label*, 3> { &familiarLabel, &wtfLabel, &lockLabel })
    {
        label->setFont(juce::FontOptions(8.0f, juce::Font::bold)); label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.50f)); label->setJustificationType(juce::Justification::centred); addAndMakeVisible(*label);
    }

    auto setupLock = [&p, this] (juce::TextButton& button, const char* property)
    {
        button.setClickingTogglesState(true); button.setToggleState(static_cast<bool>(p.parameters.state.getProperty(property, false)), juce::dontSendNotification);
        button.setColour(juce::TextButton::buttonColourId, juce::Colours::white.withAlpha(0.05f)); button.setColour(juce::TextButton::buttonOnColourId, accent.withAlpha(0.42f));
        button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.62f)); button.setColour(juce::TextButton::textColourOnId, juce::Colours::white.withAlpha(0.96f));
        button.onClick = [this] { persistDiscoverSettings(); }; addAndMakeVisible(button);
    };
    setupLock(lockCoreButton, "discover_lock_core"); setupLock(lockNastyButton, "discover_lock_nasty"); setupLock(lockSampleButton, "discover_lock_sample"); setupLock(lockModButton, "discover_lock_mod");

    discoverWtf.onValueChange = [this] { persistDiscoverSettings(); };
    discoverButton.onClick = [this]
    {
        discoverEngine.discover(static_cast<float>(discoverWtf.getValue()), getDiscoverLocks(), processor.getSampleName() != "EMPTY");
        refreshDiscoverUndo();
    };
    undoDiscoverButton.onClick = [this] { discoverEngine.undo(); refreshDiscoverUndo(); };
    refreshDiscoverUndo();
    showSpecialistPage(false);
}

unsigned int OmnariaAudioProcessorEditor::getDiscoverLocks() const noexcept
{
    unsigned int locks = 0;
    if (lockCoreButton.getToggleState()) locks |= DiscoverEngine::lockCore;
    if (lockNastyButton.getToggleState()) locks |= DiscoverEngine::lockNasty;
    if (lockSampleButton.getToggleState()) locks |= DiscoverEngine::lockSample;
    if (lockModButton.getToggleState()) locks |= DiscoverEngine::lockMod;
    return locks;
}

void OmnariaAudioProcessorEditor::persistDiscoverSettings()
{
    processor.parameters.state.setProperty("discover_wtf", discoverWtf.getValue(), nullptr);
    processor.parameters.state.setProperty("discover_lock_core", lockCoreButton.getToggleState(), nullptr);
    processor.parameters.state.setProperty("discover_lock_nasty", lockNastyButton.getToggleState(), nullptr);
    processor.parameters.state.setProperty("discover_lock_sample", lockSampleButton.getToggleState(), nullptr);
    processor.parameters.state.setProperty("discover_lock_mod", lockModButton.getToggleState(), nullptr);
}

void OmnariaAudioProcessorEditor::refreshDiscoverUndo()
{
    undoDiscoverButton.setEnabled(discoverEngine.canUndo());
}

void OmnariaAudioProcessorEditor::refreshSampleName() { sampleNameLabel.setText("SOURCE: " + processor.getSampleName(), juce::dontSendNotification); }

void OmnariaAudioProcessorEditor::showSpecialistPage(bool samplePage)
{
    showingSamplePage = samplePage;
    setComponentsVisible({ &nastyModel, &nastyAmount, &nastyDeform, &nastyFeedback, &nastyCoupling, &nastyEnergy, &nastyDamping, &nastyMoment }, ! samplePage);
    setComponentsVisible({ &sampleMode, &sampleLevel, &sampleTune, &sampleStart, &sampleEnd, &samplePosition, &sampleScan, &sampleJitter,
                           &sampleReverse, &loadSampleButton, &captureSampleButton, &sampleNameLabel }, samplePage);
    nastyTabButton.setToggleState(! samplePage, juce::dontSendNotification); sampleTabButton.setToggleState(samplePage, juce::dontSendNotification);
    resized(); repaint();
}

void OmnariaAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(8, 8, 13));
    auto body = getLocalBounds().toFloat(); body.removeFromTop(76.0f);
    auto modulationStrip = body.removeFromBottom(246.0f).reduced(14.0f, 8.0f); auto specialistStrip = body.removeFromBottom(124.0f).reduced(14.0f, 6.0f); auto stateStrip = body.removeFromBottom(118.0f).reduced(14.0f, 6.0f);
    body = body.reduced(14.0f, 8.0f); auto left = body.removeFromLeft(350.0f), right = body.removeFromRight(430.0f), centre = body.reduced(10.0f, 0.0f);
    for (const auto& panel : { left, centre, right, stateStrip, specialistStrip, modulationStrip }) { g.setColour(panelColour); g.fillRoundedRectangle(panel, 15.0f); g.setColour(panelEdge.withAlpha(0.80f)); g.drawRoundedRectangle(panel, 15.0f, 1.0f); }
    g.setColour(juce::Colours::white.withAlpha(0.48f)); g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("CORE OSCILLATORS", left.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft); g.drawText("ENGINE STATE", centre.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("FILTER / EXPRESSION / AMP", right.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft); g.drawText("PERFORMANCE", stateStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText(showingSamplePage ? "SAMPLE — SYNTHESIS / RESAMPLE" : "NASTY — MOMENT ENGINE", specialistStrip.withTrimmedLeft(190.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.drawText("MODULATION — VISIBLE ROUTING", modulationStrip.withTrimmedLeft(14.0f).removeFromTop(24.0f), juce::Justification::centredLeft);
    g.setColour(juce::Colours::white.withAlpha(0.72f)); g.setFont(juce::FontOptions(15.0f)); g.drawText("LATHAM", 20, 20, 90, 24, juce::Justification::centredLeft, false); g.setFont(juce::FontOptions(15.0f, juce::Font::bold)); g.drawText("AUDIO", 78, 20, 74, 24, juce::Justification::centredLeft, false);
}

void OmnariaAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds(); title.setBounds(bounds.getCentreX() - 150, 10, 300, 38); subtitle.setBounds(bounds.getCentreX() - 150, 45, 300, 18);
    auto body = bounds; body.removeFromTop(76); auto modulationStrip = body.removeFromBottom(246).reduced(14, 8); auto specialistStrip = body.removeFromBottom(124).reduced(14, 6); auto stateStrip = body.removeFromBottom(118).reduced(14, 6); body = body.reduced(14, 8);
    auto left = body.removeFromLeft(350).reduced(12), right = body.removeFromRight(430).reduced(12), centre = body.reduced(10, 8);

    left.removeFromTop(24); auto comboRow = left.removeFromTop(50); const auto comboCell = comboRow.getWidth() / 3; oscAShape.setBounds(comboRow.removeFromLeft(comboCell)); oscBShape.setBounds(comboRow.removeFromLeft(comboCell)); phaseMode.setBounds(comboRow);
    const auto oscH = juce::jmax(70, left.getHeight() / 3); auto r1 = left.removeFromTop(oscH); auto c1 = r1.getWidth() / 4; oscMix.setBounds(r1.removeFromLeft(c1)); oscBCoarse.setBounds(r1.removeFromLeft(c1)); pulseWidth.setBounds(r1.removeFromLeft(c1)); phase.setBounds(r1);
    auto r2 = left.removeFromTop(oscH); auto c2 = r2.getWidth() / 3; unison.setBounds(r2.removeFromLeft(c2)); detune.setBounds(r2.removeFromLeft(c2)); spread.setBounds(r2); auto r3 = left; auto c3 = r3.getWidth() / 3; subLevel.setBounds(r3.removeFromLeft(c3)); subOctave.setBounds(r3.removeFromLeft(c3)); noiseLevel.setBounds(r3);
    centre.removeFromTop(18); globe.setBounds(centre);
    right.removeFromTop(24); filterMode.setBounds(right.removeFromTop(50)); const auto rightH = juce::jmax(70, right.getHeight() / 3); auto tone = right.removeFromTop(rightH); const auto toneC = tone.getWidth() / 6;
    cutoff.setBounds(tone.removeFromLeft(toneC)); resonance.setBounds(tone.removeFromLeft(toneC)); keytrack.setBounds(tone.removeFromLeft(toneC)); filterEnvAmount.setBounds(tone.removeFromLeft(toneC)); velocityTimbre.setBounds(tone.removeFromLeft(toneC)); drive.setBounds(tone);
    auto fenv = right.removeFromTop(rightH); const auto envC = fenv.getWidth() / 4; filterAttack.setBounds(fenv.removeFromLeft(envC)); filterDecay.setBounds(fenv.removeFromLeft(envC)); filterSustain.setBounds(fenv.removeFromLeft(envC)); filterRelease.setBounds(fenv);
    auto aenv = right; const auto aC = aenv.getWidth() / 4; attack.setBounds(aenv.removeFromLeft(aC)); decay.setBounds(aenv.removeFromLeft(aC)); sustain.setBounds(aenv.removeFromLeft(aC)); release.setBounds(aenv);

    stateStrip.removeFromTop(24); const auto stateC = stateStrip.getWidth() / 5; motion.setBounds(stateStrip.removeFromLeft(stateC)); history.setBounds(stateStrip.removeFromLeft(stateC)); focus.setBounds(stateStrip.removeFromLeft(stateC)); coupling.setBounds(stateStrip.removeFromLeft(stateC)); output.setBounds(stateStrip);
    auto header = specialistStrip.removeFromTop(26); nastyTabButton.setBounds(header.removeFromLeft(76).reduced(2)); sampleTabButton.setBounds(header.removeFromLeft(76).reduced(2));
    if (! showingSamplePage)
    {
        const auto cell = specialistStrip.getWidth() / 8; nastyModel.setBounds(specialistStrip.removeFromLeft(cell)); nastyAmount.setBounds(specialistStrip.removeFromLeft(cell)); nastyDeform.setBounds(specialistStrip.removeFromLeft(cell)); nastyFeedback.setBounds(specialistStrip.removeFromLeft(cell)); nastyCoupling.setBounds(specialistStrip.removeFromLeft(cell)); nastyEnergy.setBounds(specialistStrip.removeFromLeft(cell)); nastyDamping.setBounds(specialistStrip.removeFromLeft(cell)); nastyMoment.setBounds(specialistStrip);
    }
    else
    {
        auto controls = specialistStrip; auto actionArea = controls.removeFromRight(210); sampleNameLabel.setBounds(actionArea.removeFromTop(24)); auto buttons = actionArea.removeFromTop(32); loadSampleButton.setBounds(buttons.removeFromLeft(86).reduced(2)); captureSampleButton.setBounds(buttons.removeFromLeft(112).reduced(2)); sampleReverse.setBounds(actionArea.removeFromTop(28));
        const auto cell = controls.getWidth() / 8; sampleMode.setBounds(controls.removeFromLeft(cell)); sampleLevel.setBounds(controls.removeFromLeft(cell)); sampleTune.setBounds(controls.removeFromLeft(cell)); sampleStart.setBounds(controls.removeFromLeft(cell)); sampleEnd.setBounds(controls.removeFromLeft(cell)); samplePosition.setBounds(controls.removeFromLeft(cell)); sampleScan.setBounds(controls.removeFromLeft(cell)); sampleJitter.setBounds(controls);
    }

    auto discoverHeader = modulationStrip.removeFromTop(24);
    auto controls = discoverHeader.removeFromRight(622).reduced(2, 0);
    lockLabel.setBounds(controls.removeFromLeft(34));
    for (auto* button : std::array<juce::TextButton*, 4> { &lockCoreButton, &lockNastyButton, &lockSampleButton, &lockModButton }) button->setBounds(controls.removeFromLeft(54).reduced(2, 1));
    familiarLabel.setBounds(controls.removeFromLeft(28)); discoverWtf.setBounds(controls.removeFromLeft(128).reduced(2, 2)); wtfLabel.setBounds(controls.removeFromLeft(28));
    undoDiscoverButton.setBounds(controls.removeFromLeft(62).reduced(2, 1)); discoverButton.setBounds(controls.reduced(2, 1));

    auto sourceRow = modulationStrip.removeFromTop(82); const auto sourceC = sourceRow.getWidth() / 8; for (int i = 0; i < 4; ++i) lfoRates[i]->setBounds(sourceRow.removeFromLeft(sourceC)); for (int i = 0; i < 4; ++i) macros[i]->setBounds(sourceRow.removeFromLeft(sourceC));
    auto modeRow = modulationStrip.removeFromTop(44); const auto modeC = modeRow.getWidth() / 4; for (int i = 0; i < 4; ++i) lfoModes[i]->setBounds(modeRow.removeFromLeft(modeC).reduced(4, 0));
    auto routeRow = modulationStrip; const auto routeC = routeRow.getWidth() / 4; for (int i = 0; i < 4; ++i) { auto slot = routeRow.removeFromLeft(routeC).reduced(5, 0); auto depthArea = slot.removeFromRight(78); auto sourceArea = slot.removeFromTop(slot.getHeight() / 2); modSources[i]->setBounds(sourceArea); modDestinations[i]->setBounds(slot); modDepths[i]->setBounds(depthArea); }
}
