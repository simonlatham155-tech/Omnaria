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

inline const std::array<FactoryPreset, 48>& factoryPresets()
{
    static const std::array<FactoryPreset, 48> presets {{
        { "Progressive Supersaw", "Lead", {{ "oscA_shape",0},{"oscB_shape",0},{"osc_mix",0.34f},{"unison",7},{"detune",14.5f},{"spread",0.88f},{"cutoff",11800},{"resonance",0.58f},{"drive",1.2f},{"attack",0.006f},{"decay",0.42f},{"sustain",0.82f},{"release",0.62f},{"filter_env_amt",0.42f},{"filter_decay",0.48f},{"filter_sustain",0.22f},{"filter_character",0},{"nasty_amount",0},{"sample_level",0},{"fx_motion_mix",0.08f},{"fx_delay_mix",0.10f},{"fx_space_mix",0.08f},{"fx_width",1.08f} }},
        { "Uplifting Trance Lead", "Lead", {{ "osc_mix",0.42f},{"oscB_coarse",12},{"unison",7},{"detune",17},{"spread",0.92f},{"cutoff",13200},{"filter_env_amt",0.62f},{"drive",1.8f},{"attack",0.004f},{"release",0.72f},{"fx_delay_mix",0.16f},{"fx_space_mix",0.12f} }},
        { "Mono Glide Lead", "Lead", {{ "osc_mix",0.22f},{"unison",1},{"detune",0},{"spread",0},{"cutoff",7600},{"resonance",1.1f},{"drive",2.2f},{"attack",0.003f},{"release",0.18f},{"fx_motion_mix",0},{"fx_space_mix",0.04f} }},
        { "Future-Rave Horn", "Lead", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.48f},{"oscB_coarse",-12},{"unison",5},{"detune",9},{"cutoff",6200},{"resonance",1.8f},{"filter_env_amt",2.4f},{"filter_decay",0.18f},{"filter_sustain",0.08f},{"drive",4.5f},{"filter_character",3},{"filter_character_amount",0.32f} }},
        { "Melodic-Techno Pulse", "Lead", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.46f},{"pulse_width",0.38f},{"unison",3},{"detune",5.5f},{"cutoff",5200},{"filter_env_amt",1.4f},{"filter_decay",0.32f},{"drive",2.8f},{"mod1_source",16},{"mod1_dest",8},{"mod1_depth",0.10f} }},
        { "Organic Afro Lead", "Lead", {{ "oscA_shape",2},{"oscB_shape",1},{"osc_mix",0.36f},{"unison",2},{"detune",2.5f},{"noise_level",0.025f},{"cutoff",6800},{"velocity_timbre",0.72f},{"attack",0.012f},{"release",0.32f},{"mod1_source",16},{"mod1_dest",2},{"mod1_depth",0.08f} }},
        { "Synth-Pop Analog Lead", "Lead", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.40f},{"pulse_width",0.44f},{"unison",3},{"detune",4.2f},{"cutoff",8400},{"filter_character",1},{"filter_character_amount",0.24f},{"drive",1.6f},{"fx_motion_mix",0.10f} }},
        { "Broken Digital Lead", "Lead", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.58f},{"oscB_coarse",7},{"unison",3},{"detune",8},{"cutoff",9600},{"nasty_model",0},{"nasty_amount",0.22f},{"nasty_deform",0.68f},{"mod1_source",17},{"mod1_dest",10},{"mod1_depth",0.22f} }},
        { "Melodic Chord Lead", "Lead", {{ "osc_mix",0.48f},{"oscB_coarse",7},{"unison",5},{"detune",10},{"spread",0.78f},{"cutoff",10400},{"attack",0.008f},{"release",0.58f},{"fx_space_mix",0.08f} }},
        { "Soft Emotional Lead", "Lead", {{ "oscA_shape",2},{"oscB_shape",0},{"osc_mix",0.22f},{"unison",3},{"detune",3.8f},{"cutoff",5400},{"attack",0.045f},{"decay",0.62f},{"sustain",0.72f},{"release",1.15f},{"fx_space_mix",0.13f} }},
        { "Big-Room Festival Lead", "Lead", {{ "osc_mix",0.38f},{"oscB_coarse",12},{"unison",9},{"detune",18},{"spread",0.94f},{"cutoff",14500},{"drive",3.2f},{"filter_env_amt",0.55f},{"fx_delay_mix",0.12f},{"fx_space_mix",0.09f} }},
        { "Resonant Acid Lead", "Lead", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.15f},{"unison",1},{"cutoff",2100},{"resonance",8.8f},{"filter_env_amt",3.6f},{"filter_decay",0.24f},{"filter_sustain",0.02f},{"drive",5.5f},{"filter_character",2},{"filter_character_amount",0.48f} }},
        { "Retro Neon Lead", "Lead", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.36f},{"oscB_coarse",12},{"unison",3},{"detune",6},{"cutoff",7200},{"filter_character",1},{"filter_character_amount",0.20f},{"fx_motion_mix",0.18f},{"fx_space_mix",0.08f} }},
        { "Distorted Rave Lead", "Lead", {{ "osc_mix",0.45f},{"unison",3},{"detune",7},{"cutoff",8200},{"drive",6.0f},{"filter_character",3},{"filter_character_amount",0.44f},{"nasty_model",1},{"nasty_amount",0.34f},{"nasty_deform",0.62f},{"nasty_feedback",0.38f},{"nasty_energy",0.46f},{"nasty_moment",0.24f} }},

        { "Clean Deep Sub", "Bass", {{ "oscA_shape",2},{"oscB_shape",2},{"osc_mix",0},{"unison",1},{"detune",0},{"spread",0},{"sub_level",0.18f},{"cutoff",3800},{"resonance",0.25f},{"drive",0},{"attack",0.004f},{"release",0.12f},{"nasty_amount",0},{"sample_level",0},{"fx_motion_mix",0},{"fx_delay_mix",0},{"fx_space_mix",0},{"fx_width",1} }},
        { "Rolling Trance Bass", "Bass", {{ "osc_mix",0.28f},{"unison",2},{"detune",2.2f},{"spread",0.16f},{"sub_level",0.22f},{"cutoff",2400},{"filter_env_amt",2.2f},{"filter_decay",0.16f},{"filter_sustain",0.04f},{"drive",3.0f},{"mod1_source",16},{"mod1_dest",2},{"mod1_depth",0.035f} }},
        { "Tight Offbeat Bass", "Bass", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.18f},{"unison",1},{"sub_level",0.25f},{"cutoff",3100},{"filter_env_amt",1.8f},{"filter_decay",0.11f},{"attack",0.002f},{"decay",0.18f},{"sustain",0.62f},{"release",0.08f},{"drive",2.2f} }},
        { "Round Deep-House Bass", "Bass", {{ "oscA_shape",2},{"oscB_shape",0},{"osc_mix",0.14f},{"unison",1},{"sub_level",0.20f},{"cutoff",1900},{"resonance",0.55f},{"drive",2.0f},{"filter_character",1},{"filter_character_amount",0.18f},{"release",0.22f} }},
        { "Wooden Afro Pluck Bass", "Bass", {{ "oscA_shape",1},{"oscB_shape",2},{"osc_mix",0.26f},{"unison",1},{"noise_level",0.035f},{"cutoff",3300},{"filter_env_amt",2.8f},{"filter_decay",0.10f},{"filter_sustain",0},{"decay",0.20f},{"sustain",0.20f},{"release",0.12f},{"velocity_timbre",0.82f} }},
        { "Driven Melodic-Techno Bass", "Bass", {{ "osc_mix",0.34f},{"unison",2},{"detune",2.8f},{"sub_level",0.20f},{"cutoff",2600},{"filter_env_amt",1.5f},{"drive",5.0f},{"filter_character",3},{"filter_character_amount",0.30f},{"mod1_source",16},{"mod1_dest",7},{"mod1_depth",0.08f} }},
        { "Slap-House Bass", "Bass", {{ "oscA_shape",2},{"oscB_shape",0},{"osc_mix",0.32f},{"oscB_coarse",-12},{"unison",2},{"sub_level",0.18f},{"cutoff",3900},{"filter_env_amt",2.6f},{"filter_decay",0.08f},{"drive",3.8f} }},
        { "Acid Bass", "Bass", {{ "oscA_shape",0},{"osc_mix",0},{"unison",1},{"cutoff",1100},{"resonance",9.6f},{"filter_env_amt",4.1f},{"filter_decay",0.18f},{"drive",5.8f},{"filter_character",2},{"filter_character_amount",0.52f} }},
        { "Wide Reese Bass", "Bass", {{ "osc_mix",0.52f},{"unison",2},{"detune",10.5f},{"spread",0.58f},{"sub_level",0.24f},{"cutoff",4200},{"drive",3.2f},{"nasty_model",2},{"nasty_amount",0.12f},{"nasty_coupling",0.28f},{"nasty_damping",0.70f},{"mod1_source",16},{"mod1_dest",12},{"mod1_depth",0.10f} }},
        { "Psytrance Bass", "Bass", {{ "oscA_shape",0},{"osc_mix",0},{"phase_mode",0},{"phase",0},{"unison",1},{"sub_level",0.10f},{"cutoff",2800},{"filter_env_amt",3.0f},{"filter_decay",0.07f},{"filter_sustain",0},{"attack",0.001f},{"decay",0.12f},{"sustain",0.48f},{"release",0.035f},{"drive",3.5f} }},
        { "Synthwave Bass", "Bass", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.42f},{"oscB_coarse",-12},{"unison",2},{"detune",3.5f},{"cutoff",3200},{"filter_character",1},{"filter_character_amount",0.22f},{"fx_motion_mix",0.07f} }},
        { "Aggressive Bass-House Bass", "Bass", {{ "osc_mix",0.44f},{"unison",2},{"detune",4.5f},{"sub_level",0.18f},{"cutoff",3900},{"drive",5.5f},{"nasty_model",1},{"nasty_amount",0.28f},{"nasty_feedback",0.34f},{"nasty_deform",0.52f},{"filter_character",3},{"filter_character_amount",0.32f} }},

        { "Bright Progressive Pluck", "Pluck", {{ "osc_mix",0.32f},{"unison",5},{"detune",8.5f},{"spread",0.72f},{"cutoff",9200},{"filter_env_amt",2.8f},{"filter_decay",0.16f},{"filter_sustain",0},{"attack",0.002f},{"decay",0.22f},{"sustain",0.12f},{"release",0.30f},{"fx_delay_mix",0.10f} }},
        { "Glass/Ice Pluck", "Pluck", {{ "oscA_shape",2},{"oscB_shape",1},{"osc_mix",0.30f},{"oscB_coarse",12},{"unison",3},{"detune",3.5f},{"cutoff",12800},{"filter_env_amt",1.4f},{"filter_decay",0.10f},{"decay",0.18f},{"sustain",0.05f},{"release",0.42f},{"fx_space_mix",0.12f} }},
        { "Warm Emotional Pluck", "Pluck", {{ "osc_mix",0.26f},{"unison",4},{"detune",5.5f},{"cutoff",6200},{"filter_env_amt",2.1f},{"filter_decay",0.22f},{"decay",0.30f},{"sustain",0.08f},{"release",0.65f},{"filter_character",1},{"filter_character_amount",0.16f},{"fx_space_mix",0.10f} }},
        { "Wooden Mallet Pluck", "Pluck", {{ "oscA_shape",2},{"oscB_shape",1},{"osc_mix",0.20f},{"unison",1},{"noise_level",0.055f},{"cutoff",5200},{"filter_env_amt",3.2f},{"filter_decay",0.07f},{"decay",0.16f},{"sustain",0.02f},{"release",0.18f},{"velocity_timbre",0.90f},{"mod1_source",16},{"mod1_dest",2},{"mod1_depth",0.025f} }},
        { "Gated Trance Pluck", "Pluck", {{ "osc_mix",0.36f},{"unison",5},{"detune",9},{"cutoff",7600},{"filter_env_amt",2.4f},{"filter_decay",0.12f},{"decay",0.15f},{"sustain",0.08f},{"release",0.12f},{"lfo1_sync",6},{"mod1_source",1},{"mod1_dest",2},{"mod1_depth",0.12f},{"fx_delay_mix",0.12f} }},
        { "Soft House Key", "Key", {{ "oscA_shape",2},{"oscB_shape",0},{"osc_mix",0.18f},{"unison",2},{"detune",2.5f},{"cutoff",4800},{"attack",0.008f},{"decay",0.42f},{"sustain",0.48f},{"release",0.55f},{"filter_character",1},{"filter_character_amount",0.14f} }},
        { "Crystal Bell Pluck", "Pluck", {{ "oscA_shape",2},{"oscB_shape",2},{"osc_mix",0.46f},{"oscB_coarse",12},{"unison",2},{"detune",1.2f},{"cutoff",15000},{"resonance",1.4f},{"decay",0.72f},{"sustain",0.02f},{"release",1.2f},{"fx_space_mix",0.16f} }},
        { "Muted Guitar Pluck", "Pluck", {{ "oscA_shape",1},{"oscB_shape",2},{"osc_mix",0.22f},{"unison",1},{"noise_level",0.025f},{"cutoff",4300},{"filter_env_amt",2.0f},{"filter_decay",0.055f},{"decay",0.12f},{"sustain",0.03f},{"release",0.10f},{"velocity_timbre",0.78f} }},

        { "Deep-House Chord Stab", "Chord", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.34f},{"oscB_coarse",7},{"unison",3},{"detune",4.5f},{"cutoff",3600},{"filter_env_amt",1.8f},{"filter_decay",0.20f},{"decay",0.32f},{"sustain",0.22f},{"release",0.42f},{"filter_character",1},{"filter_character_amount",0.18f} }},
        { "Wide Rave Chord", "Chord", {{ "osc_mix",0.48f},{"oscB_coarse",7},{"unison",7},{"detune",14},{"spread",0.94f},{"cutoff",9800},{"drive",2.6f},{"fx_motion_mix",0.08f},{"fx_space_mix",0.08f} }},
        { "Dark Techno Stab", "Stab", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.42f},{"oscB_coarse",-12},{"unison",2},{"cutoff",2200},{"resonance",2.2f},{"filter_env_amt",2.5f},{"filter_decay",0.11f},{"drive",4.2f},{"filter_character",3},{"filter_character_amount",0.26f},{"nasty_amount",0.10f} }},
        { "Future-Bass Chord", "Chord", {{ "osc_mix",0.50f},{"oscB_coarse",12},{"unison",7},{"detune",16},{"spread",0.95f},{"cutoff",9200},{"attack",0.025f},{"release",0.75f},{"lfo1_sync",5},{"mod1_source",1},{"mod1_dest",2},{"mod1_depth",0.18f},{"fx_space_mix",0.12f} }},
        { "Warm Analog Poly", "Poly", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.38f},{"pulse_width",0.46f},{"unison",2},{"detune",3.2f},{"spread",0.28f},{"cutoff",6100},{"resonance",0.62f},{"filter_character",1},{"filter_character_amount",0.20f},{"attack",0.018f},{"release",0.58f},{"mod1_source",16},{"mod1_dest",5},{"mod1_depth",0.025f} }},

        { "Fifths Prophet Pad", "Pad", {{ "osc_mix",0.48f},{"oscB_coarse",7},{"unison",5},{"detune",7.5f},{"spread",0.80f},{"cutoff",5200},{"attack",0.85f},{"decay",1.4f},{"sustain",0.78f},{"release",2.6f},{"filter_character",1},{"filter_character_amount",0.14f},{"fx_motion_mix",0.10f},{"fx_space_mix",0.16f} }},
        { "Airy Evolving Pad", "Pad", {{ "oscA_shape",0},{"oscB_shape",2},{"osc_mix",0.30f},{"unison",5},{"detune",6.5f},{"spread",0.86f},{"noise_level",0.035f},{"cutoff",6800},{"attack",1.15f},{"sustain",0.82f},{"release",3.4f},{"mod1_source",17},{"mod1_dest",2},{"mod1_depth",0.14f},{"mod2_source",17},{"mod2_dest",4},{"mod2_depth",0.08f},{"mod3_source",16},{"mod3_dest",5},{"mod3_depth",0.035f},{"fx_motion_mix",0.12f},{"fx_space_mix",0.20f} }},
        { "Dark Cinematic Pad", "Pad", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.52f},{"oscB_coarse",-12},{"unison",4},{"detune",5.5f},{"spread",0.72f},{"cutoff",2600},{"resonance",1.4f},{"attack",1.4f},{"sustain",0.86f},{"release",4.0f},{"nasty_model",2},{"nasty_amount",0.07f},{"nasty_coupling",0.20f},{"mod1_source",17},{"mod1_dest",12},{"mod1_depth",0.07f},{"fx_space_mix",0.24f} }},
        { "Warm Vintage Pad", "Pad", {{ "oscA_shape",0},{"oscB_shape",1},{"osc_mix",0.40f},{"unison",4},{"detune",5.0f},{"spread",0.68f},{"cutoff",4700},{"attack",0.72f},{"sustain",0.80f},{"release",2.8f},{"filter_character",1},{"filter_character_amount",0.22f},{"mod1_source",16},{"mod1_dest",5},{"mod1_depth",0.035f},{"fx_motion_mix",0.14f},{"fx_space_mix",0.14f} }},

        { "Raindrop Melodic Arp", "Arp", {{ "oscA_shape",2},{"oscB_shape",1},{"osc_mix",0.28f},{"oscB_coarse",12},{"unison",3},{"detune",3.5f},{"cutoff",9800},{"filter_env_amt",1.8f},{"filter_decay",0.10f},{"decay",0.16f},{"sustain",0.04f},{"release",0.38f},{"lfo1_sync",7},{"mod1_source",1},{"mod1_dest",2},{"mod1_depth",0.10f},{"fx_delay_mix",0.16f},{"fx_space_mix",0.10f} }},
        { "Driving 1/16 Techno Sequence", "Sequence", {{ "oscA_shape",1},{"oscB_shape",0},{"osc_mix",0.44f},{"unison",2},{"detune",2.5f},{"cutoff",3600},{"filter_env_amt",1.4f},{"drive",3.8f},{"lfo1_sync",7},{"mod1_source",1},{"mod1_dest",2},{"mod1_depth",0.24f},{"mod2_source",16},{"mod2_dest",7},{"mod2_depth",0.05f} }},
        { "Melodic-House Sequence", "Sequence", {{ "osc_mix",0.34f},{"unison",3},{"detune",5.5f},{"cutoff",6200},{"filter_env_amt",1.5f},{"lfo1_sync",6},{"mod1_source",1},{"mod1_dest",2},{"mod1_depth",0.15f},{"mod2_source",16},{"mod2_dest",5},{"mod2_depth",0.025f},{"fx_delay_mix",0.12f},{"fx_space_mix",0.08f} }},

        { "Tonal Riser/Downer", "FX", {{ "oscA_shape",0},{"oscB_shape",2},{"osc_mix",0.32f},{"unison",5},{"detune",11},{"spread",0.82f},{"noise_level",0.06f},{"cutoff",7200},{"attack",0.55f},{"release",1.8f},{"lfo1_rate",0.10f},{"mod1_source",1},{"mod1_dest",1},{"mod1_depth",0.55f},{"mod2_source",17},{"mod2_dest",2},{"mod2_depth",0.18f},{"fx_space_mix",0.18f} }},
        { "Noise Sweep/Laser", "FX", {{ "oscA_shape",2},{"osc_mix",0},{"unison",1},{"noise_level",0.24f},{"cutoff",4800},{"resonance",5.2f},{"filter_env_amt",4.2f},{"filter_attack",0.12f},{"filter_decay",1.4f},{"filter_sustain",0},{"nasty_model",0},{"nasty_amount",0.12f},{"nasty_deform",0.48f},{"mod1_source",17},{"mod1_dest",2},{"mod1_depth",0.24f},{"fx_delay_mix",0.08f},{"fx_space_mix",0.12f} }}
    }};
    return presets;
}
} // namespace omnaria
