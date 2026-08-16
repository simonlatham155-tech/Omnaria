# OMNARIA Instruction Mode

## Product intent
Instruction Mode teaches synthesis relationships inside the instrument without forcing experienced producers to use it. It is optional and must never obstruct normal sound design.

## Modes

### OFF
No instructional prompts, highlights or suggestions.

### HELP
User requests help for a control by hover, touch, focus or a `?` affordance. OMNARIA shows a concise definition, plus one relationship that is currently relevant to the patch when possible.

### TEACH
OMNARIA observes parameter changes and may offer contextual guidance when the audible impact is likely to be limited or when another parameter relationship is the more useful next move. Prompts must be rate-limited and non-modal.

## Core principle
Teach relationships, not definitions.

Examples:
- `resonance changed && cutoff very high` -> suggest lowering Cutoff into a more audible range.
- `detune changed && unison == 1` -> explain that multiple unison voices are required to hear detune.
- `filter_env_amount changed && filter ADSR is effectively static` -> suggest adjusting attack/decay/sustain.
- `lfo_rate changed && modulation depth == 0` -> explain that the LFO has no audible destination yet.
- `delay_feedback changed && delay_mix == 0` -> suggest increasing delay mix.
- `movement changed && related influence/depth == 0` -> suggest increasing influence/depth.

## TRY IT actions
Context cards may offer a tiny reversible action, for example:

`Detune is subtle because Unison = 1.  TRY 4 VOICES`

Rules:
- TRY IT uses normal exposed parameters only.
- Changes are undoable.
- It never changes hidden DSP state unavailable to the producer.
- It must show exactly which parameter(s) changed.
- It should use conservative musically sensible values rather than extreme presets.

## Highlighting
References in instructional text can target real UI controls. Activating the reference briefly highlights the destination control without moving layout or stealing keyboard focus.

## Rule engine
Version 1 should be deterministic rather than AI-dependent. A rule receives current parameter state, the recently changed parameter, and optionally measured engine state, then returns zero or one suggestion.

Conceptually:

```text
Suggestion evaluate(Change c, ParameterState p, EngineState e)
```

Rules should include:
- trigger condition
- priority
- cooldown
- short message
- related control IDs
- optional TRY IT parameter changes
- deeper HELP text

## Noise control
TEACH must not become a stream of popups.

Requirements:
- global cooldown between unsolicited suggestions
- no repeated suggestion until relevant state materially changes
- suppress low-confidence rules
- one suggestion at a time
- user can dismiss a suggestion for the current session or permanently
- OFF instantly disables the system

## Relationship to DISCOVER
Instruction Mode and DISCOVER share the same transparency rule.

When DISCOVER moves a control or creates a modulation route, Instruction Mode can explain why that change matters. It must not hide or reinterpret the change.

Example:

`DISCOVER increased Filter Env to +42% because this patch had little transient spectral movement. Lower it for a softer pluck.`

This should remain a factual explanation of the actual patch state, not generated marketing copy.

## Advanced depth
A `?` or expandable detail view can explain the underlying synthesis relationship for users who want it. Basic prompts remain concise; advanced explanations may discuss filter resonance, envelope interaction, FM ratios, phase, stochastic influence, terrain movement, spectral processing or future OMNARIA engines.

## Release gate
Instruction Mode is not complete until:
- OFF / HELP / TEACH exist
- at least 25 high-value deterministic relationship rules are tested
- TRY IT is undoable
- control highlighting works
- prompts are rate-limited and suppressible
- all advice references actual current parameter state
- advanced users can permanently leave it OFF
