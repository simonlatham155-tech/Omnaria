# OMNARIA Phase 3 — NASTY DSP Laboratory

## Goal
Create record-defining sonic events: sounds that can become *the moment* listeners remember the first time they hear a track.

NASTY is not a distortion category. It is a stateful nonlinear sound-design layer that can move a familiar patch through tension, destabilisation, event and recovery.

## Laboratory models
The first lab contains four deliberately different mathematical behaviours:

1. **Fold** — repeated nonlinear folding with controlled state feedback. Target: rip, tear, brittle bark.
2. **Feedback** — state-fed sinusoidal deformation. Target: rubber, squelch, vocal growl.
3. **Coupled** — two channel resonant states exchange energy through nonlinear coupling. Target: metallic movement, machinery, unstable resonance.
4. **Duffing** — cubic nonlinear restoring force with excitation and damping. Target: threshold behaviour, growl-to-scream transitions and controlled near-chaos.

None of these is claimed as a new synthesis method. They are established families used as ingredients in an OMNARIA-specific coupled system. A new engine name is only justified if later testing reveals a repeatable topology with behaviour that is meaningfully distinct from known methods.

## Producer controls
The permanent NASTY interface should expose musical controls rather than equation coefficients:

- MODEL
- AMOUNT / WET
- DEFORM
- FEEDBACK
- COUPLING
- ENERGY
- DAMPING / CONTROL

All controls must be normal preset-stored/automatable parameters and later become modulation destinations in the shared Phase 2 modulation language.

## Safety contract
- no NaNs or infinities
- state energy is bounded and actively damped above a safe threshold
- pitch centre must remain recoverable
- output level must be constrained independently of apparent aggression
- nonlinear antialiasing/oversampling is evaluated per model before promotion from LAB to production engine

## Musical acceptance tests
A model survives only if it can repeatedly produce useful members of several different families:

- rubber
- squelch
- bark
- rip/tear
- metallic
- scream
- unstable/WTF

The Phase 3 gate is **20 clearly distinct, record-usable NASTY sounds** that do not collapse into variations of the same distortion curve.

## Moment behaviour
Later in Phase 3, NASTY will gain trajectory/state capture so a producer can create and save a short event path:

`recognisable -> tension -> destabilisation -> moment -> recovery`

This is the basis for later DISCOVER "moment" exploration and KEEP MOMENT/history features.

## UI rule
Do not permanently expose the whole laboratory. The flagship page should show a compact NASTY card; advanced model controls expand only when requested. The engine stays deep while the main surface stays readable.
