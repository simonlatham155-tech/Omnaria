#pragma once

#include <JuceHeader.h>

class Phase7Discover
{
public:
    static void evolve(juce::AudioProcessorValueTreeState& state, float wtf, bool coreLocked)
    {
        if (coreLocked) return;
        wtf = juce::jlimit(0.0f, 1.0f, wtf);
        auto& random = juce::Random::getSystemRandom();

        const auto actual = [&state] (const juce::String& id) -> float
        {
            if (auto* p = state.getParameter(id)) return p->convertFrom0to1(p->getValue());
            return 0.0f;
        };
        const auto set = [&state] (const juce::String& id, float value)
        {
            if (auto* p = state.getParameter(id))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost(p->convertTo0to1(value));
                p->endChangeGesture();
            }
        };
        const auto mutate = [&] (const juce::String& id, float radius, float lo, float hi, float chance)
        {
            if (random.nextFloat() > chance) return;
            set(id, juce::jlimit(lo, hi, actual(id) + (random.nextFloat() * 2.0f - 1.0f) * radius));
        };

        const auto familiar = 1.0f - wtf;
        mutate("filter_character_amount", 0.08f + 0.22f * wtf, 0.0f, 1.0f, 0.28f + 0.42f * wtf);
        if (random.nextFloat() < 0.04f + 0.32f * wtf)
            set("filter_character", static_cast<float>(random.nextInt(4)));

        // Near FAMILIAR this mostly refines existing ambience. Toward WTF it may introduce an effect that was previously dry.
        mutate("fx_motion_mix", 0.05f + 0.20f * wtf, 0.0f, 0.70f, 0.12f + 0.48f * wtf);
        mutate("fx_motion_rate", 0.10f + 1.20f * wtf, 0.03f, 8.0f, 0.10f + 0.35f * wtf);
        mutate("fx_motion_depth", 0.08f + 0.24f * wtf, 0.0f, 1.0f, 0.12f + 0.38f * wtf);
        mutate("fx_delay_mix", 0.04f + 0.22f * wtf, 0.0f, 0.62f, 0.14f + 0.46f * wtf);
        mutate("fx_delay_feedback", 0.06f + 0.22f * wtf, 0.0f, 0.82f, 0.12f + 0.40f * wtf);
        mutate("fx_space_mix", 0.04f + 0.24f * wtf, 0.0f, 0.68f, 0.16f + 0.46f * wtf);
        mutate("fx_space_size", 0.08f + 0.28f * wtf, 0.05f, 1.0f, 0.14f + 0.38f * wtf);
        mutate("fx_space_damping", 0.08f + 0.24f * wtf, 0.0f, 1.0f, 0.12f + 0.36f * wtf);
        mutate("fx_width", 0.08f + 0.36f * wtf, 0.35f, 1.65f, 0.16f + 0.42f * wtf);

        if (random.nextFloat() < 0.03f + 0.25f * wtf) set("fx_delay_div", static_cast<float>(random.nextInt(8)));
        if (random.nextFloat() < 0.02f + 0.18f * wtf) set("fx_order", static_cast<float>(random.nextInt(2)));

        // Familiar search should not turn a completely dry patch into an effects preset too often.
        if (familiar > 0.75f)
        {
            if (actual("fx_motion_mix") < 0.01f && random.nextFloat() < 0.70f) set("fx_motion_mix", 0.0f);
            if (actual("fx_delay_mix") < 0.01f && random.nextFloat() < 0.70f) set("fx_delay_mix", 0.0f);
            if (actual("fx_space_mix") < 0.01f && random.nextFloat() < 0.70f) set("fx_space_mix", 0.0f);
        }
    }
};
