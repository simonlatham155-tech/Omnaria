# OMNARIA CORE Benchmark Matrix

## Purpose

OMNARIA does not start by assuming one competitor owns every sound category. For each core electronic-music sound we choose the strongest practical baseline from Sylenth1, Spire and Serum, document why it wins, identify strengths worth borrowing from the runners-up, then improve the OMNARIA version only where matched-level listening and measurements show a real benefit.

A benchmark patch is an engineering reference, not a release copy.

## Test rules

1. Compare at the same sample rate, buffer size and musical MIDI input.
2. Match perceived loudness before judging tone or impact.
3. Disable external mastering, bus compression and stereo widening.
4. Compare mono as well as stereo.
5. Record CPU for one note, an eight-note chord and high-unison use.
6. Keep a plain baseline render before every OMNARIA enhancement experiment.
7. An enhancement stays only if it improves the intended quality without damaging the identity that made the reference patch work.
8. Do not call a patch better merely because it is louder, wider or brighter.

## First benchmark set

| Category | Sylenth1 | Spire | Serum | Baseline winner | Borrow from runners-up | OMNARIA enhancement questions |
|---|---|---|---|---|---|---|
| Init saw | pending | pending | pending | pending | pending | aliasing, weight, note consistency |
| 7-9 voice supersaw | pending | pending | pending | pending | pending | centre weight, detune law, mono collapse, high-end smoothness |
| Trance pluck | pending | pending | pending | pending | pending | transient, independent filter contour, body, velocity response |
| Mono sub/bass | pending | pending | pending | pending | pending | fundamental stability, harmonics, attack, translation |
| Reese/bass | pending | pending | pending | pending | pending | beating, centre solidity, motion, aggression |
| Wide pad | pending | pending | pending | pending | pending | depth, drift, stereo correlation, sustained richness |
| Hard lead | pending | pending | pending | pending | pending | bite, centre focus, expressiveness, harshness control |
| Hoover/rave | pending | pending | pending | pending | pending | pitch motion, density, formant character, chorus/unison behaviour |
| Acid/resonant | pending | pending | pending | pending | pending | resonance character, nonlinear feedback, envelope snap |
| Short percussive | pending | pending | pending | pending | pending | envelope timing, click control, punch |

## Measurements to record

- integrated and short-term loudness for matching
- peak and true-peak level
- spectral centroid and broad low/mid/high energy distribution
- alias energy on high notes and sweeps
- transient rise time and early energy
- stereo correlation / side-to-mid energy
- mono level and tonal change
- fundamental stability
- release noise/click behaviour
- CPU at one, eight and sixteen notes where practical

## CORE enhancement vocabulary

These labels describe producer-facing qualities, not mandatory DSP blocks.

- **BODY**: resonant/modal or harmonic reinforcement that makes a sound feel physically fuller.
- **IMPACT**: transient and excitation behaviour that improves attack without relying on loudness.
- **BITE**: controlled extra harmonic complexity from PM/FM, shaping or nonlinear stages.
- **LIFE**: subtle note-to-note or time-varying behaviour that prevents sterile repetition while remaining deterministic when required.
- **MOTION**: musically coherent internal movement rather than generic LFO wobble.
- **WEIGHT**: stable fundamental and low-mid support.
- **NASTY**: later nonlinear/coupled behaviour for aggressive sound families; not part of the first neutral CORE comparison.

## Current CORE changes under test

The first Phase-2 implementation adds:

- an independent filter ADSR rather than forcing filter articulation to follow amp ADSR
- bipolar filter-envelope depth in octaves
- velocity-sensitive filter excursion and drive response
- a slightly curved symmetric unison detune/pan distribution to keep more usable centre energy while preserving wide outer voices
- DISCOVER participation in the new filter-expression controls while leaving output gain untouched

These changes are hypotheses until listening/measurement confirms which settings beat the chosen references.

## Pass condition

For a category to become an OMNARIA reference preset:

1. OMNARIA must first match the musical strengths of the selected baseline.
2. At least one deliberate improvement must survive matched-level A/B testing.
3. No improvement may introduce a larger weakness in mono, CPU, aliasing, transient behaviour or playability.
4. The producer must retain direct control of every enhancement used by the patch.
5. The reference must remain useful with experimental performance-state controls neutral.
