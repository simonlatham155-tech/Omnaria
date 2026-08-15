# OMNARIA Preset Architecture Audit — Presets 04–48

Purpose: every preset equation is allowed to expose a missing engine capability. When that happens, fix OMNARIA rather than weaken the sound. External synths are references for competitive quality, not sonic templates.

Flag meanings:
- READY: current architecture can express the core equation; still requires sound design/listening.
- ENGINE: equation exposes a reusable engine capability worth adding.
- A/B: current engines can probably solve it, but specialist contribution must be tested.

Shared improvement rule:
If several presets request the same capability, implement it once as reusable architecture and re-run the affected equations.

## Shared architecture flags discovered

A1 — True mono/legato/portamento voice allocation and gesture state. Preset 03 already exposed this.
A2 — Gesture-derived modulation sources: interval, direction, glide velocity, note transition energy.
A3 — Frequency-dependent layer masks/crossovers so specialist engines can occupy complementary spectral territories without clouding CORE.
A4 — Frequency-dependent stereo law: coherent low/body, freer upper structure.
A5 — Excitation/resonator path for mallet, guitar, wood and physical transient/body separation.
A6 — Transient layer with energy/peak normalisation and optional sample/excitation source.
A7 — Per-layer envelope/modulation timing so micro, note and macro motion do not collide.
A8 — Pitch-tracked FM/PM/formant layer with anti-alias safeguards.
A9 — Tempo/gate/step modulation architecture for sequences and trance gates.
A10 — Bass fundamental protection / upper-body split so width and nonlinear complexity cannot destabilise sub energy.
A11 — Spectral masking/redundancy measurement hooks for engine-layer acceptance.
A12 — Directional / interval-aware filter and nonlinear response for expressive leads.
A13 — Resonance/formant target control independent from ordinary filter resonance.
A14 — Controlled oscillator sync / hard-reset family for aggressive pulse/rave identities where musically useful.
A15 — Noise/laser/riser trajectory generators with pitch- and time-domain laws, not only FX automation.

## Leads

### 04 Future-Rave Horn — ENGINE: A3, A6, A8, A13
Musical job: brass/horn-like attack and body with electronic authority.
Equation implication: transient/formant layer must be separable from sustained body. FM/PM can provide upper formant energy; nonlinear edge can reinforce attack. Avoid making it merely a detuned saw stack.

### 05 Melodic-Techno Pulse — ENGINE: A7, A8, A9
Musical job: focused pulsing lead that locks to groove while retaining note identity.
Needs tempo-aware pulse/modulation architecture and clearly separated note envelope vs rhythmic modulation.

### 06 Organic Afro Lead — ENGINE: A5, A7, A12
Musical job: plucked/organic gesture with pitch expression and woody/noisy transient.
Needs excitation + resonant body and gesture-sensitive timbre. Brown/stochastic should affect material character, not global pitch.

### 07 Synth-Pop Analog Lead — READY / A/B
Current CORE can likely solve the foundation. Test subtle analogue drift, filter-character and controlled unison. Do not force exotic layers if classic analogue behaviour already wins.

### 08 Broken Digital Lead — ENGINE: A3, A7, A8
Musical job: intentionally fractured/digital identity without losing pitch readability.
Needs specialist upper-band digital/FM/stochastic territory while protecting CORE body.

### 09 Melodic Chord Lead — ENGINE: A4, A7
Needs chord-scale clarity, voice allocation and frequency-dependent stereo so chord body remains coherent while upper components spread.

### 10 Soft Emotional Lead — ENGINE: A7, A13
Needs expressive envelope/formant control and slow non-periodic colour changes without excessive brightness or width.

### 11 Big-Room Festival Lead — ENGINE: A3, A4, A6
Needs maximum projection with controlled peak efficiency, strong centre and upper-width layer. Reject brute-force density that only wins by level.

### 12 Resonant Acid Lead — ENGINE: A12, A13
Needs resonance behaviour as a first-class expressive layer, pitch/key tracking and nonlinear filter interaction. Current ordinary resonance may not be enough.

### 13 Retro Neon Lead — READY / A/B
Likely solvable with CORE, pulse/saw mix, controlled unison, chorus/width after dry pass. Specialist engines only if they add authentic evolving character without modernising it into another sound.

### 14 Distorted Rave Lead — ENGINE: A3, A6, A14
NASTY benchmark. Needs controlled transient, sync/reset option, nonlinear layer territory and peak/alias management so aggression stays intelligible.

## Basses

### 15 Clean Deep Sub — ENGINE: A10
Fundamental protection is the architecture. Any extra engine must be excluded from the protected sub band unless measurement proves value.

### 16 Rolling Trance Bass — ENGINE: A7, A9, A10
Needs precise rhythmic envelope/gate relationship, constrained micro-motion and protected fundamental.

### 17 Tight Offbeat Bass — ENGINE: A7, A9, A10
Transient timing and gate duration are the sound. Requires tempo/gate precision and fundamental protection.

### 18 Round Deep-House Bass — ENGINE: A10, A13
Needs stable low body with upper warmth/formant shaping independent from sub. Nonlinear warmth should not create low-mid fog.

### 19 Wooden Afro Pluck Bass — ENGINE: A5, A6, A10
Excitation + resonant body + protected low fundamental. Strong candidate for sample/excitation plus synthesized pitched body.

### 20 Driven Melodic-Techno Bass — ENGINE: A3, A10, A12
Keep sub coherent while upper body receives nonlinear movement/drive. Needs band-limited specialist processing.

### 21 Slap-House Bass — ENGINE: A5, A6, A10
Needs attack/body separation, strong transient identity and tightly protected sub. Could use excitation or sampled transient layer.

### 22 Acid Bass — ENGINE: A10, A12, A13
Filter resonance/drive/key tracking are core; protect fundamental while resonance and nonlinear harmonics animate upper body.

### 23 Wide Reese Bass — ENGINE: A4, A10, A11
Classic proof case for split-band stereo: mono/coherent low end, evolving stereo upper interference. Needs masking/redundancy checks to avoid width fog.

### 24 Psytrance Bass — ENGINE: A6, A7, A9, A10
Transient and phase consistency are critical. Needs sample-accurate envelope/gate timing and fundamental stability across rapid notes.

### 25 Synthwave Bass — READY / A/B, A10
Likely CORE-led with analogue character; protect low end and only add specialist motion where it improves identity.

### 26 Aggressive Bass-House Bass — ENGINE: A3, A6, A10
Protected sub plus independently distorted/nonlinear upper body; transient layer must remain punchy after processing.

## Plucks and keys

### 27 Bright Progressive Pluck — ENGINE: A6, A7, A13
Transient/filter-envelope geometry is primary. Needs transient energy efficiency and possibly resonant/formant target independent of sustain body.

### 28 Glass/Ice Pluck — ENGINE: A5, A8
FM/PM/modal-like upper partial structure plus short excitation. Needs clean inharmonic brightness without alias haze.

### 29 Warm Emotional Pluck — ENGINE: A5, A7
Excitation/body separation with softer upper spectrum and expressive decay.

### 30 Wooden Mallet Pluck — ENGINE: A5, A6
Strong excitation/resonator benchmark. Sample or noise impulse may excite a synthesized resonant body rather than serve as the whole sound.

### 31 Gated Trance Pluck — ENGINE: A7, A9
Needs tempo-synchronous gate architecture independent from the pluck envelope.

### 32 Soft House Key — ENGINE: A5, A7
Needs key-like transient/body separation and velocity-sensitive spectral response without excessive attack click.

### 33 Crystal Bell Pluck — ENGINE: A5, A8
Inharmonic/modal/FM partial architecture with controlled decay ratios and clean high-frequency behaviour.

### 34 Muted Guitar Pluck — ENGINE: A5, A6
Excitation plus resonant/string-like body, damping and velocity response. Sample transient may be useful, but pitched body should remain controllable.

## Chords, stabs and polys

### 35 Deep-House Chord Stab — ENGINE: A4, A6, A7
Chord coherence, transient projection and controlled upper stereo are more important than maximum width.

### 36 Wide Rave Chord — ENGINE: A3, A4, A11
Needs strong centre chord body with specialist upper width; masking checks are essential because stacked chord harmonics cloud quickly.

### 37 Dark Techno Stab — ENGINE: A6, A12, A13
Transient/nonlinear/resonant identity. Needs short spectral event without low-mid pile-up.

### 38 Future-Bass Chord — ENGINE: A4, A7, A9
Needs chord voice clarity, macro envelope movement and controlled stereo, likely with tempo-linked pumping/gating as a separate layer.

### 39 Warm Analog Poly — READY / A/B, A4
CORE benchmark for analogue drift, voice phase and filter character. Do not overcomplicate; frequency-dependent stereo may still improve translation.

## Pads

### 40 Fifths Prophet Pad — ENGINE: A4, A7
Needs coherent harmonic centre, controlled fifth layering and slow stereo evolution without phase wash.

### 41 Airy Evolving Pad — ENGINE: A3, A4, A7, A11
Broad stochastic benchmark. Needs separate spectral territories and timescales so stochastic/terrain/air layers evolve without fog.

### 42 Dark Cinematic Pad — ENGINE: A3, A4, A7
Needs low-mid restraint, slow spectral evolution and possibly terrain/sample texture in upper bands while preserving pitch/body.

### 43 Warm Vintage Pad — READY / A/B, A4
Analogue CORE may already be close to optimum. Add Brown/stochastic only if it improves organic life without losing warmth/centre.

## Arps and sequences

### 44 Raindrop Melodic Arp — ENGINE: A5, A7, A9
Needs short excitation, tempo-aware note/gate behaviour and controlled per-step variation.

### 45 Driving 1/16 Techno Sequence — ENGINE: A7, A9, A12
Requires deterministic tempo-synchronised modulation plus controlled nonlinear evolution over bars.

### 46 Melodic-House Sequence — ENGINE: A7, A9
Needs sequenced/gated movement distinct from note articulation, with slow macro evolution over repeated patterns.

## FX

### 47 Tonal Riser/Downer — ENGINE: A3, A8, A15
Pitch trajectory, spectral density and energy must be calculated over time. Specialist engines can occupy complementary bands as the riser evolves.

### 48 Noise Sweep/Laser — ENGINE: A7, A12, A15
Needs dedicated trajectory law for noise colour, resonance and pitch/laser sweep rather than treating the sound as automated filter noise.

## Priority shared fixes before individual tuning

1. A1/A2 mono-legato + gesture modulation — already started by Preset 03.
2. A10 fundamental-protected bass split — affects 15–26 and should be reusable.
3. A5/A6 excitation + transient/body architecture — affects plucks, mallets, guitar, house keys and several basses/leads.
4. A3/A4 spectral territories + frequency-dependent stereo — affects almost every large/wide multi-engine patch.
5. A9 tempo/gate sequencing — required for 05, 16, 17, 24, 31, 38, 44–46.
6. A8 pitch-tracked FM/PM/formant layer — required for horns, glass/bells, broken digital, risers and gesture leads.
7. A11 masking/redundancy metrics — acceptance infrastructure for complex layered patches.
8. A13 dedicated formant/resonance target — important for acid, horn, emotional leads and dark stabs.
9. A15 trajectory generators — required for the two FX targets.

## Process rule for all 48

For preset p:
1. Define musical job and strongest reference qualities.
2. Decompose into acoustic layers.
3. Assign each layer to the engine with the strongest sonic advantage.
4. Calculate masking, redundancy, phase, IMD, peak and modulation-collision penalties.
5. If the ideal equation cannot be represented, raise an architecture flag A# and improve OMNARIA.
6. Re-run the equation after the shared fix.
7. Only then encode/tune the preset.
8. Listening remains the final acceptance gate.

The preset bank is therefore also an architecture test suite: by preset 48, every recurring missing capability should either be implemented or deliberately rejected with a documented acoustic reason.
