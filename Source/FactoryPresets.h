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

// Phase 8 rule: one equation-backed sound at a time. Preset 02 does not enter
// the bank until Progressive Supersaw passes its physical and listening gates.
inline const std::array<FactoryPreset, 1>& factoryPresets()
{
    static const std::array<FactoryPreset, 1> presets {{
        { "Progressive Supersaw", "Lead", {
            { "oscA_shape", 0 }, { "oscB_shape", 0 },
            // Osc B is not an octave-brightness cheat: it remains at the same
            // pitch and supplies a second independently phased saw population.
            { "osc_mix", 0.32f }, { "oscB_coarse", 0 },

            // Szabo's JP analysis supports free/random phase. This removes a
            // repeated fixed comb at note-on while preserving the centre pitch.
            { "phase_mode", 1 }, { "phase", 0.0f },
            { "unison", 7 }, { "detune", 13.2f }, { "spread", 0.82f },
            { "sub_level", 0.0f }, { "noise_level", 0.0f },

            // Dry subtractive reference. Character and FX remain neutral so the
            // oscillator population has to earn the perceived size itself.
            { "filter_mode", 1 }, { "filter_character", 0 },
            { "filter_character_amount", 0.0f }, { "cutoff", 10800.0f },
            { "resonance", 0.46f }, { "drive", 0.8f },
            { "filter_env_amt", 0.34f }, { "filter_attack", 0.003f },
            { "filter_decay", 0.42f }, { "filter_sustain", 0.24f },
            { "filter_release", 0.34f }, { "attack", 0.005f },
            { "decay", 0.38f }, { "sustain", 0.84f }, { "release", 0.58f },

            // OMNARIA Candidate B equation:
            // d_eff(t) = d0 + 18 * 0.035 * B(t) cents.
            // Therefore |delta d| <= 0.63 cent because |B| <= 1.
            // Brown changes cloud width, not global pitch. The centre voice has
            // zero detune position, so its frequency remains f0. This targets
            // long-term beat decorrelation without audible vibrato.
            { "mod1_source", 16 }, { "mod1_dest", 5 }, { "mod1_depth", 0.035f },

            // Candidate C is intentionally absent. We first establish whether
            // bounded Brown decorrelation improves the conventional reference.
            { "motion", 0.0f }, { "nasty_amount", 0.0f }, { "sample_level", 0.0f },
            { "mod2_source", 0 }, { "mod2_dest", 0 }, { "mod2_depth", 0.0f },
            { "mod3_source", 0 }, { "mod3_dest", 0 }, { "mod3_depth", 0.0f },
            { "mod4_source", 0 }, { "mod4_dest", 0 }, { "mod4_depth", 0.0f },

            { "fx_motion_mix", 0.0f }, { "fx_delay_mix", 0.0f },
            { "fx_space_mix", 0.0f }, { "fx_width", 1.0f }, { "output", -8.0f }
        }}
    }};
    return presets;
}
} // namespace omnaria
