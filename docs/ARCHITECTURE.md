# OMNARIA / LatWorld Architecture

## Product thesis

OMNARIA must be excellent with LatWorld depth set to zero. LatWorld then adds a class of behaviour that fixed-architecture synths do not naturally provide: musical state, history, cooperative timbres, synthesis-architecture evolution and dynamic compute allocation.

## Layer model

### 1. Musical World (shared, control-rate)
Tracks host tempo, beat, bar, phrase, active notes, pitch classes, note density, energy, harmonic centre, tension and history. This layer is deliberately cheap and shared by every future timbre.

### 2. World Sound Field (shared representation)
A common description of the target sound: fundamental/harmonic energy, inharmonic energy, transient/noise content, spectral centroid/width, motion, stereo extent and density. Engines contribute to and render this field rather than behaving as permanently isolated synths.

### 3. Synthesis pool
CORE VA/unison, FM/PM, wavetable, wave-terrain, stochastic, spectral/additive, granular, physical/modal and sample/resynthesis engines. Engines are renderers/material sources, not separate products glued together.

### 4. Timbres / roles
Future milestone: 8 roles by default (bass, pluck, chords, pad, sequence, lead, texture, FX), each with independent MIDI/output routing but sharing Musical World, World Sound Field and compute budget.

### 5. Dynamic compute
Expensive audio-rate renderers are allocated to active musical needs. Idle timbres keep cheap world state but do not retain a full expensive voice engine. Quality modes will cap render budget deterministically.

## Four LatWorld laws

1. Everything inhabits a shared World.
2. The World has memory; past musical events may influence future synthesis.
3. Synthesis architecture may evolve over musical time.
4. Musical intention dominates randomness: notes, harmony, performance and producer constraints remain authoritative.

## Centre globe contract

The globe is not decorative. Its animation must be driven by engine values only. It exposes phrase phase, world energy, active-note gravity, memory, spectral distribution and evolution. Later it also shows timbral territories and compute migration.

## Sampling contract

Sampling is native material. PCM, granular and spectral playback are useful, but the LatWorld-specific operation is ABSORB: analyse a sample into transient, tonal, noise, temporal and spectral traits, then allow those traits to drive/render through other engines. Resynthesis must be able to move from recognisable source to World-derived instrument without relying on a simple A/B crossfade.

## Milestone 0.1 — CORE + World foundation

- Playable polyphonic band-limited oscillator engine.
- Stereo unison with deterministic per-voice spread.
- Per-voice amp envelope and TPT filter.
- Host tempo/bar/16-bar phrase tracking.
- Active-note harmonic centre and energy tracking.
- Stateful LatWorld memory/evolution/gravity values.
- Centre globe driven by live World state.
- State recall through APVTS.

This milestone deliberately does not claim final Serum/Spire/Sylenth/Pigments/Falcon parity. It creates the architecture on which that quality work can be measured rather than burying the project under early feature count.
