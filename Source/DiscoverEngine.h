#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

class DiscoverEngine
{
public:
    enum Lock : unsigned int
    {
        lockCore   = 1u << 0,
        lockNasty  = 1u << 1,
        lockSample = 1u << 2,
        lockMod    = 1u << 3
    };

    explicit DiscoverEngine(juce::AudioProcessorValueTreeState& s) : state(s) {}

    bool canUndo() const noexcept { return ! history.empty(); }

    void undo()
    {
        if (history.empty()) return;
        state.replaceState(history.back().createCopy());
        history.pop_back();
    }

    void discover(float wtf, unsigned int locks, bool sampleAvailable)
    {
        wtf = juce::jlimit(0.0f, 1.0f, wtf);
        const auto baseState = state.copyState();
        const auto base = captureNormalised();
        if (base.empty()) return;

        const auto profile = inferProfile(base);
        Candidate best;
        auto bestScore = std::numeric_limits<float>::max();
        auto& random = juce::Random::getSystemRandom();

        // A small quality-diversity search is more useful than one blind random mutation.
        // Each candidate starts from the current sound; WTF controls desired distance.
        for (int i = 0; i < 32; ++i)
        {
            auto candidate = base;
            mutateCandidate(candidate, base, profile, wtf, locks, sampleAvailable, random);
            const auto distance = candidateDistance(base, candidate, locks);
            const auto targetDistance = 0.018f + wtf * 0.135f;
            const auto quality = qualityPenalty(candidate, profile, wtf, sampleAvailable);
            const auto score = std::abs(distance - targetDistance) * 4.0f + quality;
            if (score < bestScore)
            {
                bestScore = score;
                best.values = std::move(candidate);
            }
        }

        if (best.values.empty()) return;
        history.push_back(baseState.createCopy());
        if (history.size() > maxHistory) history.erase(history.begin());
        applyNormalised(best.values);
    }

private:
    enum class Group { core, nasty, sample, mod };
    enum class Profile { bass, pluck, lead, pad };

    struct ParameterSpec
    {
        const char* id;
        Group group;
        float familiarWeight;
        float wtfWeight;
    };

    struct Candidate { std::vector<float> values; };

    static constexpr size_t maxHistory = 16;

    static const std::vector<ParameterSpec>& specs()
    {
        static const std::vector<ParameterSpec> all {
            { "oscA_shape", Group::core, 0.12f, 0.42f }, { "oscB_shape", Group::core, 0.12f, 0.42f },
            { "osc_mix", Group::core, 0.50f, 0.85f }, { "oscB_coarse", Group::core, 0.18f, 0.72f },
            { "pulse_width", Group::core, 0.35f, 0.72f }, { "phase_mode", Group::core, 0.08f, 0.28f },
            { "phase", Group::core, 0.20f, 0.55f }, { "unison", Group::core, 0.28f, 0.65f },
            { "detune", Group::core, 0.45f, 0.82f }, { "spread", Group::core, 0.42f, 0.78f },
            { "sub_level", Group::core, 0.30f, 0.72f }, { "sub_octave", Group::core, 0.08f, 0.30f },
            { "noise_level", Group::core, 0.25f, 0.66f }, { "filter_mode", Group::core, 0.08f, 0.36f },
            { "cutoff", Group::core, 0.65f, 0.95f }, { "resonance", Group::core, 0.42f, 0.78f },
            { "keytrack", Group::core, 0.28f, 0.62f }, { "drive", Group::core, 0.36f, 0.82f },
            { "filter_env_amt", Group::core, 0.44f, 0.82f }, { "velocity_timbre", Group::core, 0.28f, 0.58f },
            { "filter_attack", Group::core, 0.42f, 0.74f }, { "filter_decay", Group::core, 0.46f, 0.76f },
            { "filter_sustain", Group::core, 0.38f, 0.66f }, { "filter_release", Group::core, 0.44f, 0.76f },
            { "attack", Group::core, 0.52f, 0.82f }, { "decay", Group::core, 0.48f, 0.80f },
            { "sustain", Group::core, 0.42f, 0.74f }, { "release", Group::core, 0.52f, 0.84f },
            { "motion", Group::core, 0.40f, 0.80f }, { "history", Group::core, 0.36f, 0.72f },
            { "focus", Group::core, 0.32f, 0.70f }, { "coupling", Group::core, 0.30f, 0.72f },

            { "nasty_model", Group::nasty, 0.08f, 0.46f }, { "nasty_amount", Group::nasty, 0.30f, 0.92f },
            { "nasty_deform", Group::nasty, 0.34f, 0.88f }, { "nasty_feedback", Group::nasty, 0.28f, 0.86f },
            { "nasty_coupling", Group::nasty, 0.28f, 0.84f }, { "nasty_energy", Group::nasty, 0.30f, 0.88f },
            { "nasty_damping", Group::nasty, 0.30f, 0.78f }, { "nasty_moment", Group::nasty, 0.24f, 0.92f },

            { "sample_mode", Group::sample, 0.06f, 0.42f }, { "sample_level", Group::sample, 0.24f, 0.80f },
            { "sample_tune", Group::sample, 0.26f, 0.80f }, { "sample_start", Group::sample, 0.20f, 0.72f },
            { "sample_end", Group::sample, 0.20f, 0.72f }, { "sample_reverse", Group::sample, 0.05f, 0.30f },
            { "sample_position", Group::sample, 0.28f, 0.88f }, { "sample_scan", Group::sample, 0.30f, 0.90f },
            { "sample_jitter", Group::sample, 0.25f, 0.92f },

            { "lfo1_rate", Group::mod, 0.32f, 0.76f }, { "lfo1_mode", Group::mod, 0.04f, 0.34f }, { "lfo1_sync", Group::mod, 0.03f, 0.38f },
            { "lfo2_rate", Group::mod, 0.32f, 0.76f }, { "lfo2_mode", Group::mod, 0.04f, 0.34f }, { "lfo2_sync", Group::mod, 0.03f, 0.38f },
            { "lfo3_rate", Group::mod, 0.32f, 0.76f }, { "lfo3_mode", Group::mod, 0.04f, 0.34f }, { "lfo3_sync", Group::mod, 0.03f, 0.38f },
            { "lfo4_rate", Group::mod, 0.32f, 0.76f }, { "lfo4_mode", Group::mod, 0.04f, 0.34f }, { "lfo4_sync", Group::mod, 0.03f, 0.38f },
            { "env1_attack", Group::mod, 0.28f, 0.68f }, { "env1_decay", Group::mod, 0.28f, 0.68f }, { "env1_sustain", Group::mod, 0.24f, 0.62f }, { "env1_release", Group::mod, 0.28f, 0.68f },
            { "env2_attack", Group::mod, 0.28f, 0.68f }, { "env2_decay", Group::mod, 0.28f, 0.68f }, { "env2_sustain", Group::mod, 0.24f, 0.62f }, { "env2_release", Group::mod, 0.28f, 0.68f },
            { "env3_attack", Group::mod, 0.28f, 0.68f }, { "env3_decay", Group::mod, 0.28f, 0.68f }, { "env3_sustain", Group::mod, 0.24f, 0.62f }, { "env3_release", Group::mod, 0.28f, 0.68f },
            { "macro1", Group::mod, 0.30f, 0.70f }, { "macro2", Group::mod, 0.30f, 0.70f },
            { "macro3", Group::mod, 0.30f, 0.70f }, { "macro4", Group::mod, 0.30f, 0.70f },
            { "mod1_source", Group::mod, 0.02f, 0.62f }, { "mod1_dest", Group::mod, 0.02f, 0.62f }, { "mod1_depth", Group::mod, 0.24f, 0.88f },
            { "mod2_source", Group::mod, 0.02f, 0.62f }, { "mod2_dest", Group::mod, 0.02f, 0.62f }, { "mod2_depth", Group::mod, 0.24f, 0.88f },
            { "mod3_source", Group::mod, 0.02f, 0.62f }, { "mod3_dest", Group::mod, 0.02f, 0.62f }, { "mod3_depth", Group::mod, 0.24f, 0.88f },
            { "mod4_source", Group::mod, 0.02f, 0.62f }, { "mod4_dest", Group::mod, 0.02f, 0.62f }, { "mod4_depth", Group::mod, 0.24f, 0.88f }
        };
        return all;
    }

    bool groupLocked(Group group, unsigned int locks) const noexcept
    {
        if (group == Group::core) return (locks & lockCore) != 0;
        if (group == Group::nasty) return (locks & lockNasty) != 0;
        if (group == Group::sample) return (locks & lockSample) != 0;
        return (locks & lockMod) != 0;
    }

    std::vector<float> captureNormalised() const
    {
        std::vector<float> values;
        values.reserve(specs().size());
        for (const auto& spec : specs())
        {
            if (auto* parameter = state.getParameter(spec.id)) values.push_back(parameter->getValue());
            else values.push_back(0.0f);
        }
        return values;
    }

    void applyNormalised(const std::vector<float>& values)
    {
        for (size_t i = 0; i < specs().size() && i < values.size(); ++i)
        {
            if (auto* parameter = state.getParameter(specs()[i].id))
            {
                const auto v = juce::jlimit(0.0f, 1.0f, values[i]);
                if (std::abs(parameter->getValue() - v) > 0.00001f)
                {
                    parameter->beginChangeGesture();
                    parameter->setValueNotifyingHost(v);
                    parameter->endChangeGesture();
                }
            }
        }
    }

    int indexOf(const juce::String& id) const
    {
        const auto& s = specs();
        for (size_t i = 0; i < s.size(); ++i)
            if (juce::String(s[i].id) == id) return static_cast<int>(i);
        return -1;
    }

    float actual(const std::vector<float>& values, const juce::String& id) const
    {
        const auto index = indexOf(id);
        if (index < 0 || static_cast<size_t>(index) >= values.size()) return 0.0f;
        if (auto* parameter = state.getParameter(id)) return parameter->convertFrom0to1(values[static_cast<size_t>(index)]);
        return 0.0f;
    }

    void setActual(std::vector<float>& values, const juce::String& id, float value) const
    {
        const auto index = indexOf(id);
        if (index < 0 || static_cast<size_t>(index) >= values.size()) return;
        if (auto* parameter = state.getParameter(id)) values[static_cast<size_t>(index)] = parameter->convertTo0to1(value);
    }

    Profile inferProfile(const std::vector<float>& base) const
    {
        const auto attack = actual(base, "attack");
        const auto decay = actual(base, "decay");
        const auto sustain = actual(base, "sustain");
        const auto release = actual(base, "release");
        const auto cutoff = actual(base, "cutoff");
        const auto sub = actual(base, "sub_level");
        if (attack > 0.12f || release > 1.6f) return Profile::pad;
        if (sub > 0.18f || cutoff < 1100.0f) return Profile::bass;
        if (decay < 0.45f && sustain < 0.42f && release < 0.9f) return Profile::pluck;
        return Profile::lead;
    }

    void mutateCandidate(std::vector<float>& values, const std::vector<float>& base, Profile profile,
                         float wtf, unsigned int locks, bool sampleAvailable, juce::Random& random) const
    {
        const auto gaussianish = [&random]() { return (random.nextFloat() + random.nextFloat() + random.nextFloat() - 1.5f) / 1.5f; };
        const auto& all = specs();
        for (size_t i = 0; i < all.size(); ++i)
        {
            const auto& spec = all[i];
            if (groupLocked(spec.group, locks)) continue;
            if (spec.group == Group::sample && ! sampleAvailable) continue;

            const auto weight = juce::jmap(wtf, spec.familiarWeight, spec.wtfWeight);
            if (random.nextFloat() > 0.38f + 0.48f * weight) continue;
            const auto radius = (0.018f + 0.20f * wtf) * weight;
            values[i] = juce::jlimit(0.0f, 1.0f, values[i] + gaussianish() * radius);
        }

        if (! groupLocked(Group::core, locks))
        {
            if (random.nextFloat() < 0.08f + 0.38f * wtf) setActual(values, "oscA_shape", static_cast<float>(random.nextInt(3)));
            if (random.nextFloat() < 0.08f + 0.38f * wtf) setActual(values, "oscB_shape", static_cast<float>(random.nextInt(3)));
            if (random.nextFloat() < 0.05f + 0.42f * wtf)
            {
                constexpr std::array<int, 7> intervals { -12, -7, 0, 5, 7, 12, 19 };
                setActual(values, "oscB_coarse", static_cast<float>(intervals[static_cast<size_t>(random.nextInt(static_cast<int>(intervals.size())))]));
            }
            if (random.nextFloat() < 0.03f + 0.30f * wtf) setActual(values, "filter_mode", static_cast<float>(random.nextInt(4)));
        }

        if (! groupLocked(Group::nasty, locks))
        {
            if (random.nextFloat() < 0.04f + 0.45f * wtf) setActual(values, "nasty_model", static_cast<float>(random.nextInt(4)));
            if (actual(base, "nasty_amount") < 0.03f && wtf < 0.45f) setActual(values, "nasty_amount", 0.0f);
            else if (actual(base, "nasty_amount") < 0.03f && random.nextFloat() < 0.55f * wtf) setActual(values, "nasty_amount", 0.08f + random.nextFloat() * 0.55f * wtf);
        }

        if (! groupLocked(Group::sample, locks) && sampleAvailable)
        {
            if (random.nextFloat() < 0.03f + 0.38f * wtf) setActual(values, "sample_mode", static_cast<float>(random.nextInt(3)));
            if (random.nextFloat() < 0.01f + 0.22f * wtf) setActual(values, "sample_reverse", random.nextBool() ? 1.0f : 0.0f);
            auto start = actual(values, "sample_start");
            auto end = actual(values, "sample_end");
            if (end - start < 0.06f)
            {
                const auto centre = juce::jlimit(0.08f, 0.92f, (start + end) * 0.5f);
                start = juce::jmax(0.0f, centre - 0.04f);
                end = juce::jmin(1.0f, centre + 0.04f);
                setActual(values, "sample_start", start);
                setActual(values, "sample_end", end);
            }
        }

        if (! groupLocked(Group::mod, locks)) mutateRoutes(values, wtf, sampleAvailable, random);
        preserveProfile(values, profile, wtf);
    }

    void mutateRoutes(std::vector<float>& values, float wtf, bool sampleAvailable, juce::Random& random) const
    {
        for (int slot = 1; slot <= 4; ++slot)
        {
            const auto s = juce::String(slot);
            const auto depthId = "mod" + s + "_depth";
            const auto sourceId = "mod" + s + "_source";
            const auto destId = "mod" + s + "_dest";
            if (wtf > 0.30f && random.nextFloat() < (wtf - 0.22f) * 0.42f)
            {
                // Source actual indices: 1..17 are real sources; Brown=16, Stochastic=17.
                const auto source = random.nextFloat() < 0.44f * wtf ? (random.nextBool() ? 16 : 17) : 1 + random.nextInt(15);
                // Destination actual indices 1..15 are core/NASTY; 16..20 are SAMPLE.
                const auto maxExclusive = sampleAvailable ? 21 : 16;
                const auto dest = 1 + random.nextInt(maxExclusive - 1);
                setActual(values, sourceId, static_cast<float>(source));
                setActual(values, destId, static_cast<float>(dest));
                setActual(values, depthId, (random.nextFloat() * 2.0f - 1.0f) * (0.18f + 0.62f * wtf));
            }
            else
            {
                const auto currentDepth = actual(values, depthId);
                setActual(values, depthId, juce::jlimit(-1.0f, 1.0f, currentDepth + (random.nextFloat() * 2.0f - 1.0f) * (0.04f + 0.18f * wtf)));
            }
        }
    }

    void preserveProfile(std::vector<float>& values, Profile profile, float wtf) const
    {
        if (wtf > 0.72f) return;
        const auto blend = 1.0f - wtf / 0.72f;
        auto constrain = [&] (const juce::String& id, float low, float high)
        {
            const auto v = actual(values, id);
            if (v < low) setActual(values, id, juce::jmap(blend, v, low));
            if (v > high) setActual(values, id, juce::jmap(blend, v, high));
        };

        switch (profile)
        {
            case Profile::pad:
                constrain("attack", 0.035f, 6.0f); constrain("release", 0.65f, 12.0f); constrain("sustain", 0.42f, 1.0f); break;
            case Profile::bass:
                constrain("attack", 0.001f, 0.12f); constrain("release", 0.03f, 1.8f); constrain("cutoff", 45.0f, 6500.0f); break;
            case Profile::pluck:
                constrain("attack", 0.001f, 0.04f); constrain("decay", 0.03f, 1.4f); constrain("sustain", 0.0f, 0.58f); break;
            case Profile::lead:
                constrain("attack", 0.001f, 0.35f); constrain("release", 0.05f, 3.5f); break;
        }
    }

    float candidateDistance(const std::vector<float>& base, const std::vector<float>& candidate, unsigned int locks) const
    {
        float sum = 0.0f;
        float weight = 0.0f;
        const auto& all = specs();
        for (size_t i = 0; i < all.size(); ++i)
        {
            if (groupLocked(all[i].group, locks)) continue;
            const auto w = 0.5f * (all[i].familiarWeight + all[i].wtfWeight);
            sum += std::abs(candidate[i] - base[i]) * w;
            weight += w;
        }
        return weight > 0.0f ? sum / weight : 0.0f;
    }

    float qualityPenalty(const std::vector<float>& c, Profile profile, float wtf, bool sampleAvailable) const
    {
        float penalty = 0.0f;
        const auto nastyAmount = actual(c, "nasty_amount");
        const auto nastyFeedback = actual(c, "nasty_feedback");
        const auto nastyEnergy = actual(c, "nasty_energy");
        if (nastyAmount > 0.78f && nastyFeedback > 0.90f) penalty += (nastyFeedback - 0.90f) * 4.0f;
        if (nastyAmount > 0.82f && nastyEnergy > 0.90f) penalty += 0.35f;
        if (actual(c, "resonance") > 10.5f && actual(c, "drive") > 18.0f) penalty += 0.28f;
        if (! sampleAvailable && actual(c, "sample_level") > 0.001f) penalty += 0.5f;
        if (sampleAvailable && actual(c, "sample_end") - actual(c, "sample_start") < 0.05f) penalty += 0.45f;

        if (wtf < 0.55f)
        {
            if (profile == Profile::pad && actual(c, "release") < 0.35f) penalty += 0.35f;
            if (profile == Profile::pluck && actual(c, "sustain") > 0.72f) penalty += 0.30f;
            if (profile == Profile::bass && actual(c, "sub_level") > 0.85f) penalty += 0.20f;
        }
        return penalty;
    }

    juce::AudioProcessorValueTreeState& state;
    std::vector<juce::ValueTree> history;
};
