# OMNARIA Architecture

## Product thesis

OMNARIA must first be an exceptional flagship synthesizer for electronic music. Experimental shared-state ideas are infrastructure and research until they produce a demonstrably distinct synthesis mechanism.

## Layer model

### 1. CORE voice engine
High-quality polyphonic oscillator, unison, filter, envelope and drive path. This layer must be competitive on conventional bass, pluck, supersaw, pad and lead sounds before broader synthesis features are counted as advantages.

### 2. Shared performance state
A cheap control-rate layer tracks host tempo, beat/bar/phrase position, active notes, pitch classes, note density, performance energy, harmonic centre and limited history. It exists once globally so future features can use host/performance context without duplicating calculations in every voice.

This is not a claimed synthesis method. It is ordinary engine infrastructure unless future research proves otherwise.

### 3. Sound-generation families
Additional engines may be added only when independently strong. Candidate families include:

- virtual analogue / unison
- wavetable
- FM / phase modulation
- wave-terrain
- stochastic methods
- spectral/additive
- sample playback and multisampling
- granular
- physical/modal/resonator
- resynthesis

The design should avoid forcing all engines to run simultaneously.

### 4. Modulation and control
OMNARIA needs flagship modulation: envelopes, LFOs, macros, drag/drop assignment, visible modulation depth, MIDI learn, MPE/microtuning planning and deterministic automation/state recall.

### 5. Sampling and resynthesis
Sampling should be a first-class sound-design source rather than an afterthought. Basic sample playback/mapping must be solid before experimental transformations are presented as novel.

### 6. Multitimbral/shared-compute research
Multi-part operation and shared DSP may be explored where they create genuine workflow or CPU advantages. Conventional multitimbrality does not by itself constitute a new synthesis method.

## Centre globe contract

The globe remains the central visual mechanic, but it must show real values only: phrase phase, performance energy, active-note/harmonic state, history and spectral distribution. Future visual layers must correspond to real DSP or control state rather than invented animation.

## Current milestone — CORE + state foundation

- Playable polyphonic band-limited oscillator engine.
- Stereo unison with deterministic per-voice spread.
- Per-voice amp envelope and TPT filter.
- Host tempo/bar/16-bar phrase tracking.
- Active-note harmonic centre and energy tracking.
- Neutral Motion / History / Focus / Coupling experimental state values.
- Centre globe driven by live engine state.
- State recall through APVTS.

This milestone does not claim Serum/Spire/Sylenth/Pigments/Falcon parity, and it does not claim a new synthesis category. It establishes a testable engineering base from which both flagship quality and deeper mathematical DSP research can proceed.

## Research naming rule

The LatWorld concept has been moved to `docs/research/LATWORLD_CONCEPT.md`. It may only return to product naming if a formal, distinct and useful synthesis mechanism emerges and passes the revival gates recorded there.
