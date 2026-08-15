# OMNARIA CORE Benchmark Matrix

## Purpose

OMNARIA does not assume one competitor owns every sound category. For each core electronic-music sound we choose the strongest practical baseline from Sylenth1, Spire and Serum, document why it wins, identify strengths worth borrowing from the runners-up, then improve the OMNARIA version only where matched-level listening and measurements show a real benefit.

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
9. Keep Motion, History, Focus and Coupling neutral for the first CORE comparison.
10. Use Retrig phase for transient tests and Random phase only when the reference intentionally free-runs.

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

## Reproducible OMNARIA Phase 1 starting recipes

These are neutral test recipes, not finished release presets.

### CORE Supersaw
- Osc A/B: Saw / Saw
- Mix: 0.35
- B Tune: 0
- Phase Mode: Retrig
- Unison: 9
- Detune: 13 cents
- Spread: 0.82
- Sub: 0.00, Noise: 0.00
- Filter: LP12, Cutoff 16 kHz, Resonance 0.35, Keytrack 0.20
- Amp: A 5 ms / D 0.5 s / S 0.85 / R 0.8 s

### CORE Trance Pluck
- Osc A/B: Saw / Saw
- Mix: 0.22
- Phase Mode: Retrig
- Unison: 7
- Detune: 8 cents
- Spread: 0.68
- Filter: LP24, Cutoff 650 Hz, Resonance 0.75, Keytrack 0.35
- Filter Env: +4.2 oct, A 1 ms / D 240 ms / S 0.02 / R 180 ms
- Amp: A 1 ms / D 420 ms / S 0.18 / R 260 ms
- Velocity Timbre: 0.35

### CORE Mono Bass
- Osc A: Saw, Osc B: Pulse
- Mix: 0.32, Pulse Width 0.46
- Unison: 1, Spread 0
- Sub: 0.55 at -1 octave
- Filter: LP24, Cutoff 1.6 kHz, Resonance 0.40, Keytrack 0.45
- Filter Env: +1.4 oct, short decay
- Drive: 2.5 dB

### CORE Reese
- Osc A/B: Saw / Saw
- B Tune: 0
- Unison: 3
- Detune: 19 cents
- Spread: 0.36
- Sub: 0.25
- Filter: LP12, Cutoff 4.5 kHz
- Drive: 3.5 dB
- Phase Mode: Random for sustained movement tests

### CORE Wide Pad
- Osc A/B: Saw / Pulse
- Pulse Width: 0.42
- Unison: 9
- Detune: 16 cents
- Spread: 0.95
- Noise: 0.03
- Filter: LP12, Cutoff 6.5 kHz, Keytrack 0.25
- Amp: A 0.55 s / D 1.8 s / S 0.82 / R 2.6 s

### CORE Hard Lead
- Osc A/B: Saw / Saw
- B Tune: +12
- Unison: 5
- Detune: 7 cents
- Spread: 0.48
- Filter: LP24, Cutoff 4.2 kHz, Resonance 0.60, Keytrack 0.40
- Filter Env: +1.2 oct
- Drive: 4 dB
- Velocity Timbre: 0.45

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

## Phase 1 CORE implementation

The current Phase 1 engine now contains:

- polyBLEP band-limited saw and pulse sources plus sine
- two independently selectable oscillator source shapes
- producer-visible pulse width
- deterministic Retrig and explicit Random phase modes
- phase start control
- 1-9 voice unison with curved symmetric detune/pan distribution
- detune-dependent level normalisation to protect centre weight
- centred sine sub oscillator with octave selection
- pre-filter noise source
- independent amp and filter ADSRs
- bipolar filter-envelope amount in octaves
- velocity-to-filter/drive expression
- filter keytracking
- LP12, cascaded LP24, HP12 and BP12 modes
- per-voice pre-filter soft saturation
- smoothing for cutoff, resonance, drive, oscillator mix and stereo spread
- host-safe parameter/state exposure through APVTS

## Remaining empirical work inside the Phase 1 gate

The code can provide the benchmark capability, but the winner cells above remain deliberately **pending** until the actual Sylenth1, Spire and Serum reference patches are rendered and level-matched. We do not invent winners from reputation.

## Pass condition

For a category to become an OMNARIA reference preset:

1. OMNARIA must first match the musical strengths of the selected baseline.
2. At least one deliberate improvement must survive matched-level A/B testing.
3. No improvement may introduce a larger weakness in mono, CPU, aliasing, transient behaviour or playability.
4. The producer must retain direct control of every enhancement used by the patch.
5. The reference must remain useful with experimental performance-state controls neutral.
