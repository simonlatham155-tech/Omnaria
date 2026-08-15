#pragma once

#include <JuceHeader.h>
#include <vector>

class InstructionEngine
{
public:
    enum class Mode { off = 0, help = 1, teach = 2 };

    struct Suggestion
    {
        juce::String title;
        juce::String body;
        juce::String actionLabel;
        juce::String actionParameter;
        float actionValue { 0.0f };
        bool hasAction { false };
    };

    explicit InstructionEngine(juce::AudioProcessorValueTreeState& s) : state(s) {}

    void setMode(Mode m) noexcept { mode = m; }
    Mode getMode() const noexcept { return mode; }

    Suggestion helpFor(const juce::String& id) const
    {
        if (id == "cutoff") return { "CUTOFF", "Sets the filter edge. Lower it to remove more high-frequency energy." };
        if (id == "resonance") return { "RESONANCE", "Emphasises frequencies around cutoff. It becomes much clearer when cutoff is inside the audible spectrum." };
        if (id == "unison") return { "UNISON", "Adds oscillator voices. More voices create density; Detune and Spread determine how far they separate." };
        if (id == "detune") return { "DETUNE", "Separates unison voices in pitch. With one unison voice there is nothing to detune." };
        if (id == "spread") return { "SPREAD", "Moves unison voices across the stereo field. It is strongest when several voices are active." };
        if (id == "drive") return { "DRIVE", "Adds nonlinear energy before the later character stages. Use it for density before reaching for NASTY." };
        if (id == "filter_env_amt") return { "FILTER ENV", "Controls how strongly the filter envelope moves cutoff. The envelope shape determines when that movement happens." };
        if (id == "nasty_amount") return { "NASTY AMOUNT", "Blends the selected nonlinear NASTY model into the voice. At zero the NASTY engine is bypassed." };
        if (id == "nasty_feedback") return { "NASTY FEEDBACK", "Feeds energy back into the selected NASTY model. High values can cross into unstable, record-defining behaviour." };
        if (id == "nasty_moment") return { "MOMENT", "Pushes NASTY through a one-shot tension, peak and recovery trajectory instead of leaving aggression permanently on." };
        if (id == "sample_level") return { "SAMPLE LEVEL", "Blends loaded or resampled material into the synthesis path before NASTY and filtering." };
        if (id == "sample_scan") return { "SAMPLE SCAN", "Moves through SAMPLE material in Texture mode. Add Jitter or modulation for less repetitive movement." };
        if (id == "sample_jitter") return { "SAMPLE JITTER", "Adds irregular movement around the current sample position. Brown gives tighter movement; Stochastic gives broader evolution." };
        if (id.startsWith("lfo")) return { "LFO", "Creates cyclic modulation. It only changes the sound when a modulation route uses this LFO as its source with non-zero depth." };
        if (id.startsWith("macro")) return { "MACRO", "A performance control. Route the macro to useful destinations so one gesture can reshape several aspects of the patch." };
        return { "INSTRUCTION", "Change a sound control and OMNARIA will explain what it contributes and, in TEACH mode, point out useful parameter relationships." };
    }

    Suggestion teach() const
    {
        const auto value = [this] (const char* id) -> float
        {
            if (auto* p = state.getParameter(id)) return p->convertFrom0to1(p->getValue());
            return 0.0f;
        };

        if (value("unison") <= 1.1f && value("detune") > 2.0f)
            return { "Detune is currently ineffective", "Detune needs multiple unison voices. Increase UNISON to hear the separation.", "TRY 4 VOICES", "unison", 4.0f, true };

        if (value("resonance") > 3.5f && value("cutoff") > 15000.0f)
            return { "Not hearing much resonance?", "Cutoff is very high, so the resonant peak is near the top of the audible range. Lower CUTOFF to make it obvious.", "TRY 6 kHz", "cutoff", 6000.0f, true };

        if (std::abs(value("filter_env_amt")) > 0.8f && value("filter_attack") < 0.01f && value("filter_decay") < 0.03f && value("filter_sustain") > 0.90f)
            return { "Filter envelope is almost static", "The envelope amount is significant but the envelope shape changes very little. Lower sustain or increase decay to hear the movement.", "TRY SUSTAIN 35%", "filter_sustain", 0.35f, true };

        if (value("nasty_feedback") > 0.35f && value("nasty_amount") < 0.02f)
            return { "NASTY feedback is hidden", "Feedback is raised but NASTY Amount is effectively zero, so the nonlinear engine is bypassed.", "TRY 25% NASTY", "nasty_amount", 0.25f, true };

        if (value("nasty_moment") > 0.15f && value("nasty_amount") < 0.03f)
            return { "MOMENT needs NASTY", "The trajectory is active but NASTY Amount is too low for the peak to become a meaningful event.", "TRY 35% NASTY", "nasty_amount", 0.35f, true };

        if (value("sample_scan") > 0.15f && value("sample_level") < 0.01f)
            return { "Sample movement is inaudible", "SCAN is moving the sample source, but Sample Level is zero.", "TRY 30% SAMPLE", "sample_level", 0.30f, true };

        if (value("sample_jitter") > 0.25f && value("sample_level") < 0.01f)
            return { "Jitter has no audible source", "Sample Jitter is active while the sample layer is muted.", "TRY 25% SAMPLE", "sample_level", 0.25f, true };

        for (int i = 1; i <= 4; ++i)
        {
            const auto s = juce::String(i);
            const auto source = value(("mod" + s + "_source").toRawUTF8());
            const auto depth = value(("mod" + s + "_depth").toRawUTF8());
            if (source > 0.5f && std::abs(depth) < 0.005f)
                return { "Modulation route has zero depth", "A modulation source is selected, but this route cannot affect its destination until Depth moves away from zero.", "TRY DEPTH 20%", "mod" + s + "_depth", 0.20f, true };
        }

        return { "Patch relationships look active", "Nothing obvious is being cancelled by another setting. Keep shaping the sound; OMNARIA will flag relationships when they become useful." };
    }

    bool applyTry(const Suggestion& suggestion)
    {
        if (! suggestion.hasAction || suggestion.actionParameter.isEmpty()) return false;
        auto* p = state.getParameter(suggestion.actionParameter);
        if (p == nullptr) return false;
        undoParameter = suggestion.actionParameter;
        undoNormalised = p->getValue();
        hasUndo = true;
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->convertTo0to1(suggestion.actionValue));
        p->endChangeGesture();
        return true;
    }

    bool undoTry()
    {
        if (! hasUndo) return false;
        if (auto* p = state.getParameter(undoParameter))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost(undoNormalised);
            p->endChangeGesture();
        }
        hasUndo = false;
        undoParameter.clear();
        return true;
    }

    bool canUndoTry() const noexcept { return hasUndo; }

private:
    juce::AudioProcessorValueTreeState& state;
    Mode mode { Mode::off }; // Factory/default behaviour: no instruction UI or evaluation.
    juce::String undoParameter;
    float undoNormalised { 0.0f };
    bool hasUndo { false };
};
