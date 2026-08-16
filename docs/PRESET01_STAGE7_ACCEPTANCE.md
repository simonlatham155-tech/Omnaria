# Preset 01 — Stage 7 Acceptance Gate

Status: ACTIVE / NOT YET ACCEPTED

Stage 7 is the final gate for Progressive Supersaw. No new synthesis feature is added here. The task is to prove that the current equation-driven sound is at least reference-grade and that every OMNARIA-specific addition earns its place.

## Candidates under test

A — Reference CORE
- saw + saw
- 7 voices
- no Brown
- no broad stochastic modulation
- no NASTY
- no SAMPLE
- FX off

B — OMNARIA constrained stochastic
- non-uniform supersaw frequency law
- independent stereo law
- pitch-aware detune
- centre-weighted / power-matched voice gains
- Brown micro-motion on detune-cloud width

C — OMNARIA multi-stochastic
- B plus broad stochastic energy redistribution at sub-perceptual depth
- no global-pitch stochastic modulation
- NASTY and SAMPLE remain off unless a later measurement identifies a specific deficit

## Objective function

Minimise:

J = E_reference
  + E_loudness_time
  + E_masking
  + E_playback
  + E_mono
  + E_repeat
  + E_pitch
  + E_alias
  + E_true_peak
  + E_mud
  + E_cpu
  - G_spectral_efficiency
  - G_expression
  - G_identity

No candidate may win through higher loudness, wider stereo alone, longer FX tails, or added distortion.

## Test preparation

1. Disable chorus, delay, reverb and spatial enhancement.
2. Disable SAMPLE and NASTY.
3. Use identical MIDI notes, velocity and note lengths.
4. Render at the same sample rate and block size.
5. Loudness-match candidates before judgement.
6. Preserve unclipped 32-bit float renders for measurement.
7. Render both stereo and mono-summed versions.

## Keyboard test set

At minimum test:
- C2
- C3
- C4
- A4
- C5
- C6

This catches low-note pitch haze and high-note excessive beat speed / aliasing.

## Engineering gates

### 1. Pitch certainty
- centre voice remains exactly at the played note when no global pitch modulation is assigned
- Brown must change cloud width, not centre pitch
- broad stochastic must not modulate global pitch

PASS when sustained low notes retain a clearly stable pitch centre and stochastic additions do not create audible vibrato.

### 2. Repetition / beat density
Measure autocorrelation / repeated envelope behaviour over sustained notes.

PASS when B/C reduce one dominant repeated flange or beating cycle relative to A without smearing the note identity.

### 3. Loudness neutrality
A/B/C are compared level-matched.

PASS when any perceived gain survives loudness matching.

### 4. Peak efficiency
Measure sample peak, true peak and crest factor for identical MIDI renders.

PASS when B/C do not consume materially more peak headroom without a corresponding gain in density / expression.

### 5. Mono survival
Render stereo and L+R mono.

PASS when the body and pitch centre remain strong and the mono render does not hollow out badly relative to the stereo render.

The centre-weighted law is intended to improve this gate.

### 6. Stereo usefulness
Stereo width is judged independently from detune spacing.

PASS when the sound remains wide without relying on extreme decorrelation or unstable image movement.

Broad stochastic energy changes must remain mirror-symmetric.

### 7. Alias / high-note behaviour
Inspect high-note renders (C5/C6) for metallic foldback and non-harmonic contamination.

PASS when the pitch-aware detune and bandlimited oscillators maintain a clean useful spectrum.

### 8. Low-note behaviour
Inspect C2/C3 for excessive cents spread, beating smear and loss of fundamental certainty.

PASS when pitch-aware detune prevents the low range becoming an unfocused cloud.

### 9. Streaming / mass-listening survival
Test reduced-bandwidth / consumer-speaker conditions in addition to full-range monitoring.

PASS when the defining body and lead identity survive without depending primarily on extreme stereo air or sub-bass energy.

### 10. CPU value
Compare CPU cost of A/B/C.

PASS when any extra stochastic process provides an audible or measured benefit proportionate to its cost.

If an engine adds complexity without a meaningful gain, remove it.

## Reference comparison

Compare the winning OMNARIA candidate against strong Progressive Supersaw-class Sylenth1 and Spire references.

The comparison must be:
- dry or equivalently dry
- loudness matched
- equivalent note range
- equivalent musical role
- not judged from preset FX loudness

OMNARIA does not need to sound identical. It must equal or exceed the reference in the intended role while preserving its own advantages: pitch solidity, distributed beating, controlled non-repetition and playback survival.

## Phase / transient check

Stage 6 findings are still part of the final gate. Compare:
- all random phase
- all retriggered phase
- hybrid centre-coherent / outer-random phase if implemented

PASS when attack consistency improves without creating a repetitive comb-like transient.

## Decision rule

For every OMNARIA-specific addition j:

V_j = Delta Q_perceptual / (Delta CPU + Delta Peak + Delta Mud + Delta Instability)

Keep j only if V_j is meaningfully positive in level-matched listening and/or objective measurement.

## Acceptance result

Preset 01 may be marked ACCEPTED only when:
- universal build is green
- engineering gates above pass
- phase/transient decision is made
- A vs B vs C is level-matched
- a real Sylenth1/Spire reference A/B is completed
- the winning OMNARIA version is chosen on quality rather than feature count

Until all of those are true, preset 02 remains blocked.
