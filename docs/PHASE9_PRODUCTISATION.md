# Phase 9 — Productisation & Sonic Validation

Phase 8 proved that preset equations can expose missing synthesis physics. Phase 9 stops adding technology for its own sake and turns OMNARIA into a dependable instrument.

## Release gates

### Gate A — SOUND
For each of the 48 factory programs:
- audition dry first, then with production FX;
- level-match relevant Sylenth1/Spire references where appropriate;
- test the preset in a representative musical context, not only solo;
- measure peak/RMS/crest, spectral centroid, high-band energy, low-band side energy, stereo correlation, DC and transient ratio;
- inspect specialist-engine contribution for masking/redundancy;
- preserve the preset's own recognition carrier rather than applying a generic brightness/cut recipe;
- ears decide. SoundQA warnings identify what needs listening attention; they are not a quality score.

### Gate B — PERFORMANCE
- profile 1/4/8/16 voice CPU at 44.1, 48 and 96 kHz;
- rapid MIDI note-on/off and sustain abuse;
- automation stress on cutoff, resonance, macros and modulation depths;
- preset switching while notes are active;
- sample-rate/block-size changes;
- denormal/NaN/Inf checks;
- no specialist engine may run expensive work when its amount is effectively zero.

### Gate C — INTERACTION
- remove engineering use of Macro 1–4 and Coupling as hidden feature switches;
- give glide, resonator, gate, formant and PM/sync deliberate parameter ownership;
- return Macro 1–4 to per-preset performance controls;
- simplify modulation presentation: one active route plus add/select workflow, while retaining all routes underneath;
- make factory programs easy to browse by category and previous/next;
- Discover must respect protected bass, spectral territories and recognition carriers;
- Instruction Mode should map musical requests such as "less muddy" or "wider but keep bass mono" onto synthesis physics.

### Gate D — RELEASE
- VST3/AU/Standalone state recall;
- automation parameter stability;
- old preset/state compatibility policy;
- universal macOS validation;
- signing/notarisation/installer plan;
- version and build metadata;
- crash-safe sample handling;
- final documentation.

## Sound QA principle

Do not collapse engineering measurements into a fake universal quality number.

The useful objective is:

    useful audible identity surviving masking
    -----------------------------------------
       total energy + translation penalties

but every preset has a different recognition carrier. A pluck can legitimately have a high crest factor; a pad can legitimately have little transient ratio; a sub should have little stereo low-band energy. Metrics therefore generate contextual warnings for audition rather than pass/fail aesthetic judgements.

## Phase 9 order
1. SoundQA framework and render harness.
2. Run all 48 through technical QA.
3. Listening corrections and level matching.
4. CPU/performance profiling and zero-cost bypasses.
5. Dedicated controls replacing temporary macro/coupling switches.
6. UI simplification and preset browsing.
7. Discover/Instruction awareness of the new physics.
8. DAW/state/release hardening.

## Stop rule
A new synthesis feature is accepted only when an existing sound, workflow or measured defect demonstrates the need. Competitive feature parity alone is not a reason to add it.
