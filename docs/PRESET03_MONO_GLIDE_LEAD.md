# Preset 03 — Mono Glide Lead

Status: EQUATION / FIRST CANDIDATE

## Musical job
A single-note lead that feels immediate, vocal and physically connected between notes. It must remain intelligible in a dense mix without relying on supersaw width. Glide itself is part of the expression, not an effect pasted onto the sound.

Reference synths establish the competitive floor only. OMNARIA is free to become cleaner, more vocal or more dimensional if that performs the musical role better.

## Identity versus Presets 01/02
Preset 01 optimises large distributed density. Preset 02 optimises uplifting projection. Preset 03 optimises trajectory, articulation and harmonic focus.

The centre is intentionally mono and narrow. Width, if added later, must come from complementary upper information rather than detuned copies that weaken the pitch trajectory.

## Core equation
For played-note target frequency f_t and previous frequency f_0, use a continuous glide trajectory rather than instantaneous pitch jumps:

f(t) = f_t * 2^(d(t)/1200)

d(t) = d_0 exp(-t/tau_g)

d_0 = 1200 log2(f_0/f_t)

This exponential-in-cents law makes equal musical intervals follow comparable perceptual trajectories. tau_g is the expressive glide time constant.

The target sound is then

S(t) = A(t) [ C(f(t),t) + a_F F(f(t),t) + a_N N(t) ]

where:
- C = CORE harmonic/pitch body
- F = optional FM/PM upper-formant/detail layer
- N = optional nonlinear edge layer

Brown and broad stochastic are parameter processes, not mandatory audio layers.

## Engine advantage map

### CORE / analogue-derived oscillator
Job: pitch certainty, saw harmonic ladder, recognisable mono-lead body.

Use fewer voices than Presets 01/02. The pitch trajectory itself must remain obvious.

### FM/PM
Candidate job: upper harmonic/formant emphasis that follows pitch exactly. This can improve vocal-like cut without requiring more detuned saw energy.

FM/PM is accepted only if it increases useful mid/high identifying information without adding metallic alias-like haze.

### Brown stochastic
Candidate job: extremely small correlated movement in timbral colour or nonlinear amount, never global pitch. Glide is already deliberate pitch motion; random pitch motion would blur its identity.

### Broad stochastic
Candidate job: slower non-periodic evolution of upper spectral balance on held notes. It must be slower than the glide/articulation timescale.

### NASTY / nonlinear
Candidate job: controlled edge and transient bite. Nonlinearity may generate useful harmonics and intermodulation, but it must be restricted enough that low-mid IMD does not cloud the mono centre.

### Terrain / SAMPLE
No default role. Add only if later measurement/listening identifies a layer that these engines solve better.

## Cut-through objective
A mono lead cannot win through stereo size, so optimise perceptually useful harmonic information:

C_mix = sum_b W_H(b) E_useful(b) [1 - M(b)]

Reward harmonic audibility in sensitive mid/high auditory regions while preserving the fundamental/pitch centre.

Do not maximise brightness. Maximise identifiable information per unit energy:

eta_identity = I_audible_identity / E_total

## Clouding penalties

P_mask: overlapping engine energy in the same auditory bands.
P_redundancy: correlated layers carrying essentially the same information.
P_IMD: unwanted nonlinear difference/sum products, especially low-mid products.
P_peak: transient/phase stacking that costs headroom without greater projection.
P_mod_collision: stochastic motion operating on the same timescale as glide or envelope articulation.

## Temporal separation rule
Envelope = note articulation timescale.
Glide = deliberate inter-note trajectory.
Brown = microscopic timbral timescale.
Broad stochastic = slow held-note evolution.

These processes should not compete for the same temporal job.

## First dry candidate
- mono/legato priority
- saw-led CORE body
- minimal or no unison
- moderate resonance/formant emphasis rather than broad supersaw density
- fast amp attack
- controlled decay, high sustain, moderate release
- glide around 70–120 ms initial target, then optimise by interval
- FX off during engineering
- SAMPLE off
- terrain off
- Brown off global pitch
- broad stochastic off global pitch
- test small FM/PM upper layer and small nonlinear edge separately

## Acceptance
Preset 03 wins if it is:
1. clearly pitched throughout glide,
2. more expressive between notes than a static mono saw,
3. capable of cutting through without excessive brightness or level,
4. free of low-mid clouding,
5. stable in mono by design,
6. distinctive from Presets 01 and 02,
7. improved by specialist engines only where their own sonic advantage is audible/useful.

Similarity to Sylenth1, Spire or Serum is not scored. They are references for competitive quality and category expectations only.
