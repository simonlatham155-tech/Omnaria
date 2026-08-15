# Preset 03 — Mono Glide Lead

Status: ARCHITECTURE FIX IN PROGRESS

## Musical job
A single-note lead that feels immediate, vocal and physically connected between notes. It must remain intelligible in a dense mix without relying on supersaw width. Glide itself is part of the expression, not an effect pasted onto the sound.

Reference synths establish the competitive floor only. OMNARIA is free to become cleaner, more vocal or more dimensional if that performs the musical role better.

## Identity versus Presets 01/02
Preset 01 optimises large distributed density. Preset 02 optimises uplifting projection. Preset 03 optimises trajectory, articulation and harmonic focus.

The centre is intentionally mono and narrow. Width, if added later, must come from complementary upper information rather than detuned copies that weaken the pitch trajectory.

## Core glide equation
For played-note target frequency f_t and previous frequency f_0, use a continuous glide trajectory rather than instantaneous pitch jumps:

f(t) = f_t * 2^(d(t)/1200)

d(t) = d_0 exp(-t/tau_g)

d_0 = 1200 log2(f_0/f_t)

This exponential-in-cents law makes equal musical intervals follow comparable perceptual trajectories. tau_g is the expressive glide time constant.

## Gesture becomes synthesis
The lead must not merely slide between two static timbres. OMNARIA derives synthesis control signals from the movement itself.

Interval:

I = 1200 log2(f_t/f_0)

Direction:

q = sign(df/dt)

Glide velocity / movement energy:

v_g(t) = |d(log2 f(t))/dt|

Bounded interval excitation:

G(I) = 1 - exp(-|I|/I_c)

The engine now has a reusable GlideLaw implementation for the cents trajectory, interval excitation and normalised gesture velocity. Shared performance state also records previous note, target note, interval, direction and note-on serial.

These signals are synthesis inputs. They do not automatically alter every parameter.

## Target sound equation

S(t) = A(t) [ C(f(t),t) + a_F(t) F(f(t),t) + a_N(t) N(t) + a_T(t) T(t) ]

where:
- C = CORE harmonic/pitch body
- F = FM/PM upper-formant/detail layer
- N = controlled nonlinear edge/friction layer
- T = optional terrain-derived upper movement layer

Brown and broad stochastic are parameter processes, not mandatory audio layers.

## Engine advantage map

### CORE / analogue-derived oscillator
Job: pitch certainty, saw harmonic ladder, recognisable mono-lead body.

Use fewer voices than Presets 01/02. The pitch trajectory itself must remain obvious.

### FM/PM — movement-dependent harmonic excitement
Candidate job: upper harmonic/formant emphasis that follows pitch exactly. The key OMNARIA extension is to let glide motion control its intensity:

I_FM(t) = I_0 + k_v v_g(t) + k_I G(I)

A fast or large glide may temporarily expose more upper structure; as the note settles, FM/PM returns toward its base level. This gives the journey its own timbre without clouding the fundamental.

FM/PM is accepted only if it increases useful mid/high identifying information without metallic haze.

### Brown stochastic — microscopic life after arrival
Brown must not randomise global pitch. Glide already owns the pitch dimension.

Preferred jobs:
- microscopic filter colour
- microscopic nonlinear amount
- microscopic upper-layer balance

Brown should become more perceptible only where the sound would otherwise feel mathematically frozen.

### Broad stochastic — slow held-note evolution
Candidate job: slower non-periodic evolution of upper spectral balance after the deliberate glide gesture has largely settled.

It must operate on a clearly slower timescale than portamento and envelope articulation.

### NASTY / nonlinear — gesture friction
Instead of static distortion, movement can increase controlled nonlinear bite:

N_amt(t) = N_0 + k_N v_g(t) + k_D max(0,q) G(I)

This can make a fast upward glide feel more energetic than a stationary note. The low-mid centre must remain protected from excessive intermodulation.

### Terrain — gesture trajectory through a surface
Terrain is not included as a generic extra layer. A useful future mapping is:

x(t) = G(I)
y(t) = v_g(t)

T(t) = Terrain[x(t), y(t)]

The player's interval and motion then travel through the terrain. If this is used, terrain should primarily contribute complementary upper spectral information, not duplicate CORE.

### SAMPLE
No default role. Add only if a specific transient/acoustic-information deficit is identified.

## Direction-dependent behaviour
Upward and downward movement do not have to be acoustically identical.

q = +1 may allow a little more upper harmonic exposure / nonlinear excitement.
q = -1 may favour a rounder arrival.

This must be subtle enough that the same musical phrase remains coherent, but it gives the instrument a physical sense of gesture.

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

Each specialist engine must contribute a unique sonic advantage. If two layers fill the same perceptual territory, relocate, reduce or remove one.

## Temporal separation rule
Envelope = note articulation timescale.
Glide = deliberate inter-note trajectory.
FM/NASTY gesture response = glide-linked timescale.
Brown = microscopic timbral timescale.
Broad stochastic = slow held-note evolution.

These processes should not compete for the same temporal job.

## Implementation status
Implemented infrastructure:
- `GlideLaw.h`: exponential-in-cents glide maths, interval, direction, gesture velocity, interval excitation
- shared state: previous note-on, target note-on, interval cents, direction, interval excitation, note-on serial
- state engine: MIDI note-ons now populate that gesture telemetry

Still required before preset 03 is accepted:
- real monophonic/legato voice policy
- audible portamento path using GlideLaw
- glide-time control
- expose gesture-derived modulation signals to selected synthesis destinations
- implement/test FM/PM or terrain layer only where the current engine supports it properly

## First dry candidate target
- true mono/legato priority
- saw-led CORE body
- minimal or no unison
- moderate resonance/formant emphasis rather than broad supersaw density
- fast amp attack
- controlled decay, high sustain, moderate release
- glide around 70–120 ms initial target, then optimise by interval
- FX off during engineering
- SAMPLE off
- Brown off global pitch
- broad stochastic off global pitch
- test FM/PM upper layer and nonlinear gesture edge separately

## Acceptance
Preset 03 wins if it is:
1. clearly pitched throughout glide,
2. more expressive between notes than a static mono saw,
3. capable of cutting through without excessive brightness or level,
4. free of low-mid clouding,
5. stable in mono by design,
6. distinctive from Presets 01 and 02,
7. audibly responsive to interval, glide speed and direction,
8. improved by specialist engines only where their own sonic advantage is useful.

Similarity to Sylenth1, Spire or Serum is not scored. They are references for competitive quality and category expectations only.
