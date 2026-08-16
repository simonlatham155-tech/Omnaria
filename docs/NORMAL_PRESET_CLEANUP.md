# Normal Factory Preset Cleanup — Pass 1

This pass treats OMNARIA's 48 factory sounds as conventional musical synth presets. NASTY is explicitly excluded from redesign until its own stability/calculation pass.

## Rule
A normal factory preset may store normal musical patch state: oscillator/source selection, tuning, mix, unison, detune, width/spread, sub/noise, filter, envelopes, velocity/expression, modulation, macros, FX, mono/poly, arp and output trim.

The sound-design equations used while creating the bank are not runtime synthesis algorithms. Their results are baked into these ordinary parameter values.

## NASTY isolation
Existing `nasty_*` values in the 48 are considered legacy dependencies for now. Do not tune or reinterpret them during normal-preset cleanup. They must be inventoried and later replaced by a named, separately validated NASTY recipe/reference. Until that pass, preserving them is safer than silently changing the sound.

## Gain/stability guard
Normal-preset cleanup must not repeat the earlier overload mistake. Output trim alone is not a safety system.

For each normal preset evaluate:
1. source-layer contribution (Osc A/B + sub + noise),
2. unison count/detune/spread contribution,
3. filter resonance/drive gain,
4. modulation peaks that can increase level or resonance,
5. FX wet/feedback contribution,
6. final output trim.

A preset must be internally finite and stable before final output trim. Do not solve overload by simply lowering `output` after an upstream stage has clipped or become unstable.

## Polyphony validation
Validate normal presets at 1 note first, then representative chords/voice counts. The 32-note cap is a capacity target, not a requirement that 32 maximum-velocity voices sum below 0 dBFS. The requirement is that increasing polyphony does not cause internal nonlinear runaway, NaN/Inf, denorm storms, or pathological CPU escalation.

## First findings from current bank
- Several normal sounds are already clean conventional definitions, e.g. Progressive Supersaw, Uplifting Trance Lead, Clean Deep Sub, Rolling Trance Bass and Tight Offbeat Bass.
- Some sounds mix conventional design with legacy NASTY coefficients, e.g. Mono Glide Lead, Future-Rave Horn, Broken Digital Lead, Big-Room Festival Lead, Resonant Acid Lead, Distorted Rave Lead, Driven Melodic-Techno Bass and Slap-House Bass. Preserve their normal settings; defer the `nasty_*` part.
- Filter `drive` and `resonance` deserve particular attention on acid/rave patches because they can create internal peak growth before output trim.
- High-unison leads need source/unison normalisation checked independently of their output value.

## Cleanup sequence
1. Inventory all 48 by category and flag legacy NASTY references.
2. Audit ordinary parameter values for redundant/default entries and accidental modulation routes.
3. Audit layer/unison/filter gain structure.
4. Standardise sensible final output trims only after internal safety is established.
5. Preserve the intended identity of every sound; do not homogenise toward Serum, Spire or Sylenth1.
6. After normal bank sign-off, perform a separate NASTY mathematical/stability design pass.
