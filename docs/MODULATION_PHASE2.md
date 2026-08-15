# OMNARIA Phase 2 — Modulation Language

Phase 2 gives OMNARIA one modulation language that CORE and every later specialist engine can share.

## Design rules

1. Modulation is visible and preset-stored. No hidden sound-changing route is allowed.
2. The producer can use simple routes without living in a giant matrix.
3. Specialist engines can become modulation/state sources later instead of being bolted on as post-effects.
4. Stochastic behaviour is not one generic random source.
5. DISCOVER may later create or mutate normal visible routes, but Phase 2 does not silently invent routes.

## Sources

- LFO 1–4
- Aux Envelope 1–3
- Velocity
- Keyboard position
- Mod wheel
- Aftertouch / channel pressure
- Macro 1–4
- Brown
- Stochastic

### Brown

Brown is the tighter bounded random-walk behaviour learned from VELORIA. It is intended for constrained motion where the sound must stay close to a target: basses, plucks, controlled leads and rhythmic material.

### Stochastic

Stochastic is the broader slow-changing state source learned from the original VELORIA behaviour. It is intended for dynamic and lush movement: pads, strings, atmospheres and evolving upper-spectrum motion.

A patch may use Brown, Stochastic, both through different routes, or neither.

## LFO behaviour

Each LFO has a free rate and trigger mode:

- Free
- Retrig
- One Shot

Tempo-sync is part of the Phase 2 design and uses the host BPM already present in `OmnariaState`; UI exposure is the next small completion item if a build iteration is required after CI.

## Current visible route destinations

- Pitch
- Cutoff
- Resonance
- Oscillator Mix
- Detune
- Stereo Spread
- Drive
- Pulse Width

The destination vocabulary will expand as BODY, NASTY, SAMPLE and other specialist systems become real controllable engines.

## Scaling

Routes use a bipolar `-1..+1` depth. Destination scaling remains destination-aware so a useful depth range is musical rather than arbitrary:

- Pitch: up to ±12 semitones per full-depth route
- Cutoff: up to ±4 octaves
- Resonance: producer-facing resonance offset
- Osc Mix: ±0.5
- Detune: ±18 cents
- Spread: ±0.5
- Drive: ±12 dB
- Pulse Width: ±0.35

Multiple routes accumulate and are clamped at the destination's safe range.

## Performance and MIDI

Pitch bend remains part of the voice engine. CC1 is exposed as Mod Wheel. Poly aftertouch and channel pressure feed the Aftertouch source. Macro controls are host-automatable APVTS parameters.

## Research basis

The architecture deliberately combines the strongest workflow lessons rather than cloning one synth:

- Sylenth1 demonstrates that a compact set of envelopes, LFOs and modulation slots can remain fast and musically effective.
- Serum demonstrates the value of per-control modulation visibility, MIDI learn and routes that remain part of the editable patch.
- OMNARIA adds distinct tight Brown and broad Stochastic state sources so stochastic motion can be selected according to the sound rather than represented by one randomness amount.

## Gate for Phase 3

Phase 2 is ready to feed NASTY when:

- the universal macOS build is green;
- route state recalls correctly;
- Brown and Stochastic can be heard as clearly different behaviours through the same destination;
- LFO/Env/performance sources can share the same routing system;
- no modulation route depends on hidden parameters.
