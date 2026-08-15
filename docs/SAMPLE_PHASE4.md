# OMNARIA Phase 4 — SAMPLE / RESAMPLE

## Goal
SAMPLE is not a conventional playback lane. It is source material that participates in the same synthesis system as CORE and NASTY.

A loaded or resampled sound can supply a transient, body, texture, loop, vocal fragment, percussion fragment or deliberately damaged source, then pass through OMNARIA's envelopes, drive, NASTY cells, filters and shared modulation language.

## Implemented source modes

1. **ONE SHOT** — pitched sample playback across the keyboard. Best for attacks, vocal fragments, hits and transient layers.
2. **LOOP** — selected region repeats while the voice is alive. Best for body, sustaining texture and sampled oscillation-like material.
3. **TEXTURE** — playback is attracted toward a modulatable position while scan and jitter move through the selected region. This is intentionally synthesis-oriented rather than a simple loop.

## Producer controls

- MODE
- LEVEL
- ROOT NOTE
- TUNE
- START / END
- REVERSE
- POSITION
- SCAN
- JITTER

The compact flagship page exposes the most useful controls. ROOT remains host/preset accessible and can move into an advanced panel during the later UI pass.

## Shared modulation

Phase 4 adds normal Phase 2 destinations for:

- Sample Level
- Sample Position
- Sample Scan
- Sample Jitter
- Sample Tune

This means Brown can create tight sample motion, broader Stochastic can create organic texture drift, and LFO/envelope/velocity/aftertouch/macros can all reshape sample behaviour without a separate modulation system.

## Resampling

OMNARIA continuously retains a short stereo history of its post-output signal. **RESAMPLE** publishes the most recent two seconds back into the shared sample pool.

That enables iterative sound design:

`CORE -> NASTY -> filter -> RESAMPLE -> SAMPLE -> NASTY -> filter ...`

The point is not infinite destructive processing for its own sake. The producer can freeze one useful sonic moment and turn it into stable, playable material.

## Routing decision

SAMPLE is mixed with CORE before drive / NASTY / filter processing. This is deliberate: sample material should be able to excite and reshape the specialist engines, not bypass them as an unrelated audio player.

## UI rule

NASTY and SAMPLE share one specialist card with tabs. Adding Phase 4 therefore does not add another permanent strip to the 1280x980 flagship view.

## Engineering limits / later hardening

- loaded and captured audio lives in immutable shared sample data, so voices never read a buffer being modified by the UI thread
- file length is capped to one minute during this stage to avoid accidental huge allocations
- the resample history is four seconds; the RESAMPLE action currently takes the most recent two seconds
- sample payload embedding inside a saved preset/project state is reserved for preset/file-format hardening; parameter state itself is already saved
- higher-quality offline resampling, crossfade loops and true multi-grain clouds remain candidate refinements only if listening proves they improve the 48-preset target or the record-defining moment workflow

## Phase 4 acceptance

Engineering acceptance requires:

- external audio load
- per-note pitched playback
- one-shot / loop / texture behaviours
- start/end/reverse/tune controls
- shared modulation integration
- sample into NASTY/filter routing
- capture of OMNARIA output back into SAMPLE
- compact progressive-disclosure UI
- universal macOS VST3/AU/Standalone CI green

Musical acceptance is separate: the engine must later prove that sampled attacks/textures materially improve target presets and can reproduce the kind of one-off sample behaviour heard in the dance records being used as references. Features that do not improve matched listening should not survive merely because they exist.
