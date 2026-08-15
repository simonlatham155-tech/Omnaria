#pragma once

#include <JuceHeader.h>
#include <initializer_list>

namespace omnaria
{
struct FactoryPresetValue
{
    const char* id;
    float value;
};

struct FactoryPreset
{
    const char* name;
    const char* category;
    std::initializer_list<FactoryPresetValue> values;
};

// Phase 8 rule: add exactly one preset only after its maths/physics design and
// Sylenth1/Spire benchmark pass are complete. Do not pre-fill the 48-bank.
inline const std::array<FactoryPreset, 1>& factoryPresets()
{
    static const std::array<FactoryPreset, 1> presets {{
        { "Progressive Supersaw", "Lead", {
            { "oscA_shape", 0 },
            { "oscB_shape", 0 },
            { "osc_mix", 0.32f },
            { "oscB_coarse", 0 },
            // Free/random initial phase prevents an identical comb-like attack
            // relationship on every note while retaining a stable pitch centre.
            { "phase_mode", 1 },
            { "phase", 0.0f },
            { "unison", 7 },
            { "detune", 13.2f },
            { "spread", 0.86f },
            { "sub_level", 0.0f },
            { "noise_level", 0.0f },

            // Reference-grade dry subtractive core.
            { "filter_mode", 1 },
            { "filter_character", 0 },
            { "filter_character_amount", 0.0f },
            { "cutoff", 10800.0f },
            { "resonance", 0.46f },
            { "drive", 0.8f },
            { "filter_env_amt", 0.34f },
            { "filter_attack", 0.003f },
            { "filter_decay", 0.42f },
            { "filter_sustain", 0.24f },
            { "filter_release", 0.34f },
            { "attack", 0.005f },
            { "decay", 0.38f },
            { "sustain", 0.84f },
            { "release", 0.58f },

            // Candidate B: Brown drives only the unison detune magnitude.
            // In the current modulation law depth .035 gives about +/-0.63 cent
            // movement at the detune destination: small enough to preserve pitch
            // identity while decorrelating long-term beating of the side voices.
            { "mod1_source", 16 },
            { "mod1_dest", 5 },
            { "mod1_depth", 0.035f },

            // No broad stochastic motion or nonlinear engine yet. Those belong
            // to candidate C only if the acoustic equation predicts an advantage.
            { "motion", 0.0f },
            { "nasty_amount", 0.0f },
            { "sample_level", 0.0f },
            { "mod2_source", 0 },
            { "mod2_dest", 0 },
            { "mod2_depth", 0.0f },
            { "mod3_source", 0 },
            { "mod3_dest", 0 },
            { "mod3_depth", 0.0f },
            { "mod4_source", 0 },
            { "mod4_dest", 0 },
            { "mod4_depth", 0.0f },

            // Dry benchmark: FX cannot hide oscillator/filter weaknesses.
            { "fx_motion_mix", 0.0f },
            { "fx_delay_mix", 0.0f },
            { "fx_space_mix", 0.0f },
            { "fx_width", 1.0f },
            { "output", -8.0f }
        }}
    }};
    return presets;
}
} // namespace omnaria
