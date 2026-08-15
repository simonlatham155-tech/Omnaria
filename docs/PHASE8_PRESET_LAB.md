# OMNARIA Phase 8 — 48 Preset Benchmark Lab

Phase 8 is sound engineering, not preset filling.

## Acceptance rule

Each preset starts from a musically strong CORE identity. Specialist engines are added only when they improve a specific acoustic property in matched listening. More engines is not a higher score.

For every preset compare:
1. CORE-only baseline
2. specialist-enhanced OMNARIA version
3. strongest relevant external reference (Sylenth1 / Spire / Serum)

Level-match before judging. Reject an enhancement if it only sounds better because it is louder or wider.

## Acoustic dimensions

- transient definition
- fundamental stability / pitch centre
- harmonic density
- spectral movement
- stereo correlation / width
- low-mid body
- high-frequency air
- envelope articulation
- dynamic non-repetition
- nonlinear character
- spatial depth
- CPU cost

## Engine roles

CORE — stable musical identity, pitch, phase/unison, envelopes, filter.
Brown — tight constrained stochastic movement; useful for bass/pluck/lead/rhythmic detail.
Stochastic — broader organic non-repetition; useful for pads/strings/atmospheric evolution.
NASTY — nonlinear/coupled threshold events and memorable record-defining moments; use sparingly.
SAMPLE — real/complex transients, texture and captured/resampled events that elementary oscillators cannot efficiently recreate.
FILTER CHARACTER — harmonic redistribution/drive after the voice filter; Clean must remain a valid answer.
FX — time/space/decorrelation; never use wetness to disguise a weak dry patch.

## The 48 targets

### Leads — 14
1. Progressive Supersaw
2. Uplifting Trance Lead
3. Mono Glide Lead
4. Future-Rave Horn
5. Melodic-Techno Pulse
6. Organic Afro Lead
7. Synth-Pop Analog Lead
8. Broken Digital Lead
9. Melodic Chord Lead
10. Soft Emotional Lead
11. Big-Room Festival Lead
12. Resonant Acid Lead
13. Retro Neon Lead
14. Distorted Rave Lead

### Basses — 12
15. Clean Deep Sub
16. Rolling Trance Bass
17. Tight Offbeat Bass
18. Round Deep-House Bass
19. Wooden Afro Pluck Bass
20. Driven Melodic-Techno Bass
21. Slap-House Bass
22. Acid Bass
23. Wide Reese Bass
24. Psytrance Bass
25. Synthwave Bass
26. Aggressive Bass-House Bass

### Plucks and keys — 8
27. Bright Progressive Pluck
28. Glass/Ice Pluck
29. Warm Emotional Pluck
30. Wooden Mallet Pluck
31. Gated Trance Pluck
32. Soft House Key
33. Crystal Bell Pluck
34. Muted Guitar Pluck

### Chords, stabs and synths — 5
35. Deep-House Chord Stab
36. Wide Rave Chord
37. Dark Techno Stab
38. Future-Bass Chord
39. Warm Analog Poly

### Pads — 4
40. Fifths Prophet Pad
41. Airy Evolving Pad
42. Dark Cinematic Pad
43. Warm Vintage Pad

### Arps and sequences — 3
44. Raindrop Melodic Arp
45. Driving 1/16 Techno Sequence
46. Melodic-House Sequence

### FX — 2
47. Tonal Riser/Downer
48. Noise Sweep/Laser

## First ten diagnostic presets

Build these first because together they exercise different physics:
1. Progressive Supersaw — unison/phase/stereo/filter/envelope benchmark.
15. Clean Deep Sub — pitch/fundamental/mono stability; specialist engines should normally stay off.
27. Bright Progressive Pluck — transient and filter-envelope geometry.
39. Warm Analog Poly — phase/drift/filter-character test.
41. Airy Evolving Pad — Stochastic vs Brown proof case.
16. Rolling Trance Bass — constrained Brown movement and envelope precision.
3. Mono Glide Lead — legato/pitch behaviour with minimal masking.
30. Wooden Mallet Pluck — SAMPLE/excitation plus synthesized pitched body.
23. Wide Reese Bass — controlled interference, stereo upper body, stable low end.
14. Distorted Rave Lead — NASTY threshold/event benchmark.

## Preset-specific idealisation rule

Every finished preset must document an engine recipe. Example only:

Progressive Supersaw:
CORE 100%
Brown 0–small if it improves microscopic motion
Stochastic 0–small
NASTY normally 0
SAMPLE 0
Filter Character by matched test
FX only after dry sound passes

Airy Evolving Pad:
CORE establishes pitch/body
Stochastic supplies broad organic evolution
Brown only if tighter local detail is useful
SAMPLE may supply air/texture
NASTY only if a specific transition improves identity

Clean Deep Sub:
CORE dominates
Brown/Stochastic/NASTY/SAMPLE default to zero unless blind listening proves an improvement

## Pass criteria

A preset passes only if:
- it is distinct from all other 47 targets
- it works dry enough to expose the synthesis quality
- added engines have an identifiable acoustic purpose
- specialist-enhanced version wins or ties CORE-only in level-matched listening
- it remains playable across a useful keyboard/velocity range
- it does not depend on accidental clipping
- CPU use is proportionate to the audible gain
- its identity survives removal of excessive reverb/delay

Phase 8 musical validation cannot be declared from CI alone. CI validates code/preset infrastructure; listening validates sound.