# Preset 001 — Progressive Supersaw

Status: EQUATION DEFINED. Implementation follows this model; no preset 002 work begins before 001 is validated.

## Reference class

Sylenth1 / Spire progressive-trance supersaw.

Reference strengths to meet:
- stable pitch centre
- dense but musical beating
- strong mono centre despite stereo width
- clean high-frequency saw energy
- fast readable attack
- smooth filter opening
- low CPU relative to perceived size

## Ambition

This is a mature analogue-derived sound. OMNARIA does not need to make it unrecognisable. The target is top-of-field reference quality plus a small measurable increase in fullness/life without adding blur.

Target vector, conceptual:

T_001 = [P .97, H .96, D .70, M .50, T .82, C .92, W .94, N .16, X .48]

The baseline reference R_001 is Sylenth1/Spire-class supersaw performance. Delta is deliberately small:

Delta_001 = [0, +fullness, +micro-expression, +nonperiodic micro-motion, 0, >=0 clarity, >=0 width, tiny nonlinear colour, +non-repetition]

## Signal equation

Let s(f,t,phi) be a bandlimited saw. The conventional core is

U(t) = g0 s(f0,t,phi0)
     + sum_i g_i s(f0*2^(d_i/1200), t, phi_i)

with detune offsets d_i chosen non-uniformly and symmetrically around 0 cents so that the set of pairwise beat frequencies |f_i-f_j| is distributed rather than concentrated.

OMNARIA enhancement is not another large saw stack. Define a bounded Brown process

b[n] = clamp(rho b[n-1] + sigma xi[n])

and use it only as microscopic perturbation of detune/phase-related state:

d_i[n] = d_i + epsilon_b * q_i * b[n]

where q_i weights outer voices more than the centre voice, preserving pitch centre.

Optional nonlinear enrichment is

C(t) = (1-alpha) U(t) + alpha F(U(t))

where F is a gentle bounded saturation/waveshaping function and alpha must remain small enough that the ear reads harmonic density, not distortion.

Final dry voice before production FX:

y(t) = H_LP24(t){C(t)}

The filter envelope is part of the transient identity. FX are not allowed to determine whether this preset passes.

## Engine allocation hypothesis

CORE: dominant. Supplies saw identity, pitch centre, envelope, filter, stereo unison.
Brown: microscopic only. Job = break strict periodic recurrence in beating without pitch wander.
Stochastic broad engine: initially OFF. Its timescale is probably too broad for the baseline supersaw; test only after CORE+Brown is strong.
NASTY: initially OFF or extremely low waveshaping contribution only. No threshold/feedback behaviour in baseline 001.
SAMPLE: OFF. There is no acoustic deficit here that sampling solves better than synthesis.
FILTER CHARACTER: Clean baseline, then Warm at very low amount as candidate B.
FX: OFF for dry validation; later only modest delay/space for production version.

## Three candidate programs

A — Reference-grade conventional
7-voice non-uniform unison, no specialist engine.

B — OMNARIA Full
5 or 7 CORE voices plus microscopic Brown detune perturbation and very low nonlinear colour. Goal: equal or higher perceived density with less obvious periodic beating, without extra smear.

C — OMNARIA Evolved
Candidate B plus tiny slow stochastic spectral/mix movement only if it improves sustained-note life while retaining attack and pitch centre.

## Objective measurements

1. Fundamental drift: centre-frequency error over sustained note.
2. Pairwise beat distribution: concentration/variance of |f_i-f_j|.
3. Stereo correlation and mono energy loss.
4. Spectral centroid mean and variance.
5. Harmonic-to-noise ratio / unintended inharmonic energy.
6. Crest factor during first 100 ms.
7. High-note alias energy near Nyquist.
8. RMS/LUFS level match against reference before judgement.
9. CPU per active note.
10. Autocorrelation of amplitude/spectral motion: Brown-enhanced version should reduce obvious repetition without becoming unstable.

## Pass rule

A must first compete with the Sylenth1/Spire reference. B replaces A only if it wins level-matched on fullness/life while maintaining or improving pitch stability, clarity and mono compatibility. C replaces B only if its additional evolution is audible as quality rather than movement for movement's sake.

No preset 002 equation or code before this program is heard and accepted.
