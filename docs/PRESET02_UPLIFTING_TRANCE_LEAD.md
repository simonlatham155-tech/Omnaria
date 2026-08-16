# Preset 02 — Uplifting Trance Lead

Status: BUILDING / EQUATION FIRST

## Musical job

Preset 02 is not a brighter copy of Progressive Supersaw. Its job is to project a clear, emotional lead line through a dense trance mix while retaining OMNARIA identity.

Target qualities:
- immediate pitch recognition
- strong transient projection without click
- high useful harmonic audibility through a busy mix
- narrower, more focused body than Preset 01
- enough stereo width for scale without sacrificing the centre
- long-note life without obvious periodic wobble
- clean mass-listening translation
- no clouding from redundant layers

Sylenth1, Spire and Serum are reference floors only. The final timbre may differ substantially if OMNARIA performs the musical role better.

## Physics model

A saw is useful because its harmonic amplitudes approximately follow:

A_n proportional to 1 / n

so it provides both even and odd harmonics across the audible spectrum. For this preset the goal is not maximum spectral energy; it is maximum useful audible information.

Define a mix-cut score:

C_mix = sum_b W_hearing(b) * E_useful(b) * [1 - M(b)]

where b is an auditory band, W_hearing is perceptual weighting, E_useful is lead-identifying energy and M is estimated masking/redundancy.

The preset objective is:

Q_02 = wC*C_mix + wT*T_projection + wP*P_certainty + wE*E_expression + wI*I_identity + wX*X_playback
       - lambdaM*P_masking - lambdaR*P_redundancy - lambdaPhi*P_phase - lambdaPeak*P_peak
       - lambdaAlias*P_alias - lambdaCloud*P_cloud

Hard constraints:
- no audible pitch wander on the centre
- no severe mono hollowing
- no upper-register alias/foldback character
- stochastic/residual layers must not mask transient or steady-state pitched structure

## Layer architecture

### Layer 1 — CORE harmonic spine

CORE supplies the saw-derived pitched identity and the majority of steady-state harmonic energy.

Unlike Preset 01, Preset 02 should be tighter:
- lower detune than Progressive Supersaw
- slightly narrower stereo spread
- strong centre weighting
- fast amplitude attack

The aim is projection rather than maximum cloud size.

### Layer 2 — octave brilliance population

Osc B is allowed at +12 semitones for this preset because the octave has a specific acoustic job: reinforce upper harmonic audibility and emotional lift without requiring broadband distortion.

It must remain lower in level than the main population so the fundamental/body is not hollowed out.

### Layer 3 — Brown micro-life

Brown motion may breathe the unison cloud at a smaller depth than Preset 01.

Its job is to prevent perfectly static beating while preserving a firmer lead pitch:

d_eff(t) = d0 + epsilon_B * B(t)

with epsilon_B deliberately below Preset 01 unless listening proves otherwise.

### Layer 4 — broad stochastic expression

Broad stochastic is not assigned to pitch. Its first candidate job is very small filter/cutoff or population-balance motion over a slower timescale than Brown.

Brown = micro-timescale
Broad stochastic = slow phrase/colour timescale
Envelope = note timescale

This temporal separation reduces modulation collision.

### Layer 5 — transient / edge candidate

A tiny nonlinear contribution may be tested only if the dry CORE lacks forward attack after level matching. NASTY is not automatically included.

If used, it must increase transient/steady-state audibility more than residual/noise-like energy.

## Clarity / clouding rule

Research on mix clarity motivates separating transient, steady-state and residual energy and penalising residual energy when it masks the pitched/transient components. For Preset 02:

Clarity_02 = Audibility(T + SS) - Masking(R -> T+SS)

where T is transient, SS is steady-state harmonic content and R is residual/noise-like content.

Any added engine layer is rejected or reduced if:
- its spectrum substantially overlaps an existing layer without new perceptual information
- its modulation spectrum duplicates another layer
- it raises peak headroom cost without proportional cut/expression gain
- it smears the attack

## Initial Candidate A — dry focused trance lead

- Osc A: saw, main pitch
- Osc B: saw, +12 semitones, minority contribution
- 7 unison voices
- tighter detune than Preset 01
- slightly narrower stereo spread than Preset 01
- LP12 first candidate to preserve useful upper harmonics
- fast amp attack, medium decay, high sustain, musical release
- modest positive filter envelope
- very low Brown cloud breathing
- broad stochastic assigned to slow colour, not pitch
- NASTY off initially
- SAMPLE off initially
- chorus/delay/reverb off for engineering comparison

## Acceptance tests

1. The lead must cut through a dense reference mix at equal loudness without simply being brighter everywhere.
2. Fundamental/pitch identity must remain obvious on phone/small-speaker playback.
3. Octave layer must add lift rather than thin the body.
4. Brown must add life without audible wander.
5. Broad stochastic must add phrase-scale expression without masking the attack.
6. Mono sum must preserve body and note identity.
7. C5/C6 must remain clean and non-metallic.
8. Residual/noise-like energy must remain subordinate to transient + harmonic information.
9. Peak cost must be justified by projection.
10. Final sound is free to differ from Sylenth1/Spire/Serum if it performs the role better.
