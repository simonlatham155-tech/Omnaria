# OMNARIA Competitive Bar

OMNARIA is the LATHAMAUDIO flagship. Feature count is not success. The release standard is that a producer can choose OMNARIA for ordinary dance-music work because its tone, workflow and CPU behaviour are genuinely competitive with the best current instruments.

## Instruments we must respect

- Serum 2: hybrid oscillator breadth, sample/granular/spectral workflow, modulation clarity.
- Spire: dance-music immediacy, supersaw/unison character, fast trance sound design.
- Sylenth1: tone-per-CPU, simplicity, punch, preset immediacy.
- Vital: clean wavetable/unison, visual modulation, efficient workflow.
- Pigments: multi-engine depth, modern modulation, polished visual feedback.
- Phase Plant: open-ended routing and modular sound construction.
- Falcon: synthesis/sampling breadth and deep instrument construction.

## Gate A — CORE sound

Experimental performance-state controls set to neutral must still produce release-ready sounds.

Required A/B categories:

1. init saw across the keyboard
2. mono sub/bass
3. 7–9 voice supersaw
4. trance pluck
5. wide pad
6. hard lead
7. sync/FM-style attack
8. resonant filter sweep
9. short percussive envelope
10. long release / high polyphony

A patch only passes when OMNARIA is competitive on tone, stereo image, transient quality and CPU at matched loudness. "Different" is not an excuse for weaker.

## Gate B — DSP quality

- band-limited oscillators and anti-aliasing strategy must be measured, not assumed
- no avoidable DC, denormal or note-stealing clicks
- parameter smoothing for audible continuous controls
- deterministic offline render unless a preset explicitly requests free-running variation
- quality modes must trade CPU for quality predictably
- stereo unison must remain mono-compatible and controllable
- filters must remain stable at extreme resonance and modulation

## Gate C — workflow

A flagship patch should be playable quickly and deeply editable without exposing every subsystem at once.

Required before commercial release:

- searchable/tagged preset browser
- proper INIT preset
- Discover with locks and controlled mutation
- drag/drop modulation with visible depth
- MIDI learn on performance controls
- host automation for every user-facing control
- undo/redo for sound-design operations
- resizable interface
- MPE and microtuning plan
- DAW-safe state recall and preset migration/versioning

## Gate D — synthesis breadth

Additional synthesis engines are introduced only when their quality is independently strong.

Planned source/material families:

- CORE virtual analogue / unison
- wavetable
- FM / phase modulation
- wave-terrain
- stochastic methods
- spectral/additive
- sample playback and multisample mapping
- granular
- physical/modal/resonator
- sample analysis and resynthesis

The architecture must not require every engine to run simultaneously.

## Gate E — original research

OMNARIA is allowed to become a research platform, but no new synthesis label is used merely because multiple existing techniques are combined.

A genuinely new engine would need:

- a formal mathematical/algorithmic mechanism
- sonic behaviour not reducible to ordinary modulation, crossfading or effects
- useful musician control
- repeatable DAW-safe operation
- practical real-time CPU performance
- prior-art review against academic and commercial systems
- an A/B test showing the defining behaviour disappears when the new mechanism is removed

The preserved LatWorld hypothesis and its revival criteria are in `docs/research/LATWORLD_CONCEPT.md`.

## Gate F — performance

Benchmarks are relative, not marketing numbers. Every release candidate is profiled against matched patches in the comparison instruments on the same machine, sample rate and buffer.

Track at minimum:

- one-note CPU
- 8-note CPU
- 16-note CPU
- 7/9 voice unison CPU
- voice-steal behaviour
- memory allocation
- editor-open vs editor-closed CPU/GPU
- offline render speed
- Intel Mac vs Apple Silicon

Shared control-rate state should remain cheap; expensive audio-rate renderers should sleep when inactive where doing so is deterministic and safe.

## Current milestone

0.1 proves the skeleton: quality-conscious CORE oscillation, playable polyphony, shared host/performance state and a real-state globe. It is not a finished flagship and it is not a claim of a new synthesis method.
