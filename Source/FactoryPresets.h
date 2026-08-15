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

// Phase 8 rule: each sound enters only after its own acoustic equation exists.
inline const std::array<FactoryPreset, 2>& factoryPresets()
{
    static const std::array<FactoryPreset, 2> presets {{
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

            // Candidate B2/B3 stochastic split.
            // Brown: d_eff(t) = d0 + 18 * 0.035 * B(t) cents, so |delta d| <= 0.63 cent.
            // It breathes cloud width only; the centre detune position is zero.
            { "mod1_source", 16 }, { "mod1_dest", 5 }, { "mod1_depth", 0.035f },

            // Broad stochastic is deliberately assigned a different physical job.
            // It slowly cross-distributes energy between the two independently phased
            // same-pitch saw populations. This changes interference/spectral density
            // without modulating the fundamental.
            { "mod2_source", 17 }, { "mod2_dest", 4 }, { "mod2_depth", 0.045f },

            { "motion", 0.0f }, { "nasty_amount", 0.0f }, { "sample_level", 0.0f },
            { "mod3_source", 0 }, { "mod3_dest", 0 }, { "mod3_depth", 0.0f },
            { "mod4_source", 0 }, { "mod4_dest", 0 }, { "mod4_depth", 0.0f },

            { "fx_motion_mix", 0.0f }, { "fx_delay_mix", 0.0f },
            { "fx_space_mix", 0.0f }, { "fx_width", 1.0f }, { "output", -8.0f }
        }},

        { "Uplifting Trance Lead", "Lead", {
            // Preset 02 optimises projection, pitch certainty and useful harmonic
            // information rather than maximum cloud size. Osc B is a deliberate
            // low-level octave-brilliance layer, not a duplicate of preset 01.
            { "oscA_shape", 0 }, { "oscB_shape", 0 },
            { "osc_mix", 0.18f }, { "oscB_coarse", 12 },
            { "phase_mode", 1 }, { "phase", 0.0f },
            { "unison", 7 }, { "detune", 9.4f }, { "spread", 0.74f },
            { "sub_level", 0.0f }, { "noise_level", 0.0f },

            // LP12 keeps more useful upper harmonic information than the broader
            // progressive patch while a restrained envelope supplies the classic
            // opening lead gesture without relying on FX brightness.
            { "filter_mode", 0 }, { "filter_character", 0 },
            { "filter_character_amount", 0.0f }, { "cutoff", 12500.0f },
            { "resonance", 0.38f }, { "keytrack", 0.38f }, { "drive", 0.65f },
            { "velocity_timbre", 0.28f },
            { "filter_env_amt", 0.48f }, { "filter_attack", 0.002f },
            { "filter_decay", 0.30f }, { "filter_sustain", 0.18f },
            { "filter_release", 0.28f }, { "attack", 0.003f },
            { "decay", 0.26f }, { "sustain", 0.90f }, { "release", 0.46f },

            // Brown is tighter than preset 01: max detune movement is about
            // 18 * 0.020 = 0.36 cent. Its only job is micro-life in the cloud.
            { "mod1_source", 16 }, { "mod1_dest", 5 }, { "mod1_depth", 0.020f },

            // Broad stochastic gets a slower colour job, not pitch. At this depth
            // Cutoff motion is subtle enough to animate sustained notes without
            // turning the lead into an obvious random-filter effect.
            { "mod2_source", 17 }, { "mod2_dest", 2 }, { "mod2_depth", 0.018f },

            // Specialist layers stay off until the clarity/cut equation proves a deficit.
            { "motion", 0.0f }, { "nasty_amount", 0.0f }, { "sample_level", 0.0f },
            { "mod3_source", 0 }, { "mod3_dest", 0 }, { "mod3_depth", 0.0f },
            { "mod4_source", 0 }, { "mod4_dest", 0 }, { "mod4_depth", 0.0f },

            // Dry engineering candidate. Production FX are added only after the
            // oscillator/filter/transient architecture has earned the sound.
            { "fx_motion_mix", 0.0f }, { "fx_delay_mix", 0.0f },
            { "fx_space_mix", 0.0f }, { "fx_width", 1.0f }, { "output", -8.5f }
        }}
    }};
    return presets;
}
} // namespace omnaria
