# Phase 10 — Premium Instrument UI

## Design target
OMNARIA must look like an established, expensive hardware-derived instrument from its first release: dense enough to invite sound design, but with an obvious reason for every visible controller.

## Non-negotiable family branding
- LATHAMAUDIO is always one word visually: **Latham** light weight, **Audio** bold weight.
- OMNARIA is the dominant centred product wordmark, with deliberate wide tracking and generous negative space, taking family discipline from VELORIA without copying its identity.
- One restrained OMNARIA accent colour. No gratuitous glow, fake screws, wood cheeks or decorative texture.

## Control hierarchy
Visible control size follows frequency of musical interaction, not DSP complexity.

### Performance — 46–50 px rotary target
Controls expected to be grabbed while producing/playing: Cutoff, Resonance, Drive where appropriate, four Macros, ARP Complexity, Master/Output.

### Sound design — 34–40 px rotary target
Osc mix/tune, Unison, Detune, Spread/Width, ADSR, filter envelope, LFO rate/amount and other normal patch-design controls.

### Tweak — 22–28 px rotary target
Fine setup and specialist-engine adjustment: phase, keytrack, fine tune, specialist amounts/coefficients and modulation depths. These remain available where they provide a distinct sonic advantage but do not visually compete with performance controls.

## Every-controller test
A front-panel controller must pass all three:
1. A musician can understand the musical result of changing it.
2. It has a worthwhile audible range.
3. It needs independent manipulation rather than being an internal coefficient or part of a preset behaviour.

Failing controls become contextual, preset-internal, consolidated or removed from screen one.

## Screen-one architecture
### Header
LATHAMAUDIO family mark / centred OMNARIA wordmark / preset navigation / SAVE USER / MONO / help.

### Source
SOURCE selects **SYNTH** or **SAMPLE**. These occupy the same physical bay; both full control sets are never shown simultaneously.

SYNTH mode exposes the normal synthesis source controls.

SAMPLE mode turns normal synthesis generation off. Drag/drop audio becomes the source and continues through OMNARIA behaviour, filter, modulation, NASTY and FX processing. Initial musical controls: drag/drop/name, Loop/Key behaviour, host Sync for loops, Start, Length/End, Tune and Reverse. Do not build a Kontakt-style sampler.

### NASTY
NASTY is selected by preset only. Detailed NASTY coefficients remain engine/preset-construction parameters and do not occupy normal screen-one knob positions.

### ARP
Dedicated compact screen-one section:
- ON
- Direction: UP/DOWN | UP | DOWN
- Motion: forward | forward+reverse
- Steps: 2 | 4 | 6 | 8
- COMPLEXITY rotary

ARP remains strictly locked to host tempo/transport. Complexity progressively introduces musically constrained mid-steps, reversals, rotations/grouping, x2/x3 activity, rests and octave movement. Variable phrase geometry must never mean tempo drift. Transport restart is deterministic.

### Shared processing
Filter, amp, movement/macros, modulation and FX remain available downstream of either source.

## Presets
- 48 factory sounds visible and selectable from OMNARIA itself.
- Previous / preset name / next always visible.
- Factory bank is read-only.
- SAVE USER writes the complete current instrument state to a separate LATHAMAUDIO/OMNARIA user-preset location.
- User presets must remember source selection, sample configuration/reference policy, NASTY selection, ARP state and all synthesis/processing settings.
- MONO sits next to the preset controls, like VELORIA. Poly mode is capped at 32 simultaneous MIDI notes.

## Layout logic
Follow musical signal flow and functional bays rather than uniform grids:
**SOURCE → SHAPE → FILTER → AMP → MOVEMENT/PROCESSING**
with specialist/tweak controls visually subordinate.

Use negative space deliberately. Do not enlarge the window merely to fit new features. Reclaim space by removing duplicate/ambiguous controls and preset-internal engineering controls.

## Immediate cleanup from current UI
- Current FILTER has two controls both visibly named CHARACTER. Rename/consolidate so type and amount cannot be confused.
- Remove detailed NASTY knob bank from normal interaction; replace with NASTY preset selection.
- Distinguish duplicated contextual words where they remain (e.g. Delay Feedback vs engine feedback, LFO Rate vs other rates).
- Replace the current SAMPLE tab concept with a true SOURCE: SYNTH | SAMPLE workflow and drag/drop target.
- Add visible factory/user preset browser, SAVE USER and MONO.
- Add compact ARP bay.
- Keep plenty of useful knobs: the objective is clarity, not minimalism.

## DSP gates before aesthetic sign-off
- One note must be internally gain-stable before preset judgement.
- Then verify 8, 16 and 32 simultaneous MIDI notes cleanly on the heaviest factory programs.
- Preset-design equations are design-time tools whose results are baked into presets; do not recompute the full sound-design equation per played note.
- Runtime protections/normalisation must be lightweight deterministic engine rules, not an adaptive quality solver.

## Build order
1. Control inventory: PERFORMANCE / SOUND DESIGN / TWEAK / SELECTOR / INTERNAL / DUPLICATE.
2. Header branding + preset/Save/Mono architecture.
3. Remove NASTY engineering clutter and duplicate naming.
4. SOURCE bay with SYNTH/SAMPLE contextual switching and drag/drop sampler.
5. Compact tempo-locked ARP.
6. Re-layout remaining synthesis/filter/envelope/mod/FX controls using three size classes.
7. Validate resizing/DAW opening before visual polish.
8. Sound/gain/polyphony validation before final factory-bank sign-off.
