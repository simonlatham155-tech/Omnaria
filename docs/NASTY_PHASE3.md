# OMNARIA Phase 3 — NASTY DSP Laboratory

## Goal
Create record-defining sonic events: sounds that can become *the moment* listeners remember the first time they hear a track.

NASTY is not a distortion category. It is a stateful nonlinear sound-design layer that can move a familiar patch through tension, destabilisation, event and recovery.

## Implemented laboratory models

1. **Fold** — repeated nonlinear folding with controlled state feedback. Target: rip, tear, brittle bark.
2. **Feedback** — state-fed sinusoidal deformation. Target: rubber, squelch, vocal growl.
3. **Coupled** — left/right resonant states exchange energy through nonlinear coupling. Target: metallic movement, machinery, unstable resonance.
4. **Duffing** — cubic nonlinear restoring force with excitation and damping. Target: threshold behaviour, growl-to-scream transitions and controlled near-chaos.

None is claimed as a new synthesis method. They are established mathematical families used as ingredients in an OMNARIA-specific sound-design system. A new engine name is justified only after later prior-art review and repeatable evidence of meaningfully distinct behaviour.

## Producer controls

The Phase 3 surface is deliberately compact:

- MODEL
- AMOUNT
- DEFORM
- FEEDBACK
- COUPLING
- ENERGY
- DAMPING
- MOMENT

All controls are preset-stored and automatable. AMOUNT=0 bypasses the NASTY contribution. The controls are also destinations in the shared Phase 2 modulation matrix.

## Shared modulation integration

NASTY Amount, Deform, Feedback, Coupling, Energy, Damping and Moment can be driven by:

- LFO 1–4
- Env 1–3
- Velocity
- Key position
- Mod Wheel
- Aftertouch/channel pressure
- Macro 1–4
- Brown (tight/local stochastic motion)
- Stochastic (broader organic evolution)

This is deliberate: specialist engines do not invent private modulation systems.

## Moment trajectory

MOMENT is implemented as a per-note one-shot nonlinear trajectory:

`recognisable -> tension -> peak -> recovery`

At its peak it raises effective deform/feedback/coupling/energy and reduces damping while increasing wet contribution. It then returns toward the parent patch instead of leaving NASTY permanently pinned at maximum aggression.

This is the Phase 3 precursor to later DISCOVER moment exploration, history and KEEP MOMENT/state capture.

## Safety contract

- nonlinear cell outputs are checked for finite values
- resonator/velocity states are bounded
- high state energy triggers extra damping
- feedback is hard-limited below unity
- dry signal remains recoverable through AMOUNT
- output gain is independent from NASTY discovery mutation
- nonlinear antialiasing/oversampling remains an explicit promotion test for each model before final release tuning

## Audition gate

`docs/NASTY_RECIPES.md` defines 20 deliberately separated engineering recipes across Fold, Feedback, Coupled and Duffing, covering rubber, squelch, bark, tear, metallic, scream and WTF/near-chaos targets.

The code phase is complete when universal VST3/AU/Standalone CI is green. The **musical** gate still requires rendering/listening at matched level: recipes that collapse into the same audible family, depend on aliasing, or are merely louder are rejected or redesigned. We do not fake that listening result from code inspection.

## UI rule

Do not permanently expose the entire laboratory or future engine internals. Phase 3 uses one compact NASTY strip. The final flagship UI should continue toward progressive disclosure: familiar sound controls first, specialist depth available when requested.
