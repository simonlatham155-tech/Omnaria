# Normal Factory Preset Cleanup — Corrected Pass

This pass preserves the calculated sonic architecture of OMNARIA's 48 factory sounds while correcting gain, redundancy and stability problems. NASTY remains explicitly excluded from redesign until its own stability/calculation pass.

## Core principle
The earlier preset equations were design-time tools, not runtime synthesis algorithms. They were used to decide what each sound needed in terms of musical role, cut-through, engine choice, spectral occupation, movement, stereo behaviour, masking control and translation.

That design method remains valid.

The mistake exposed by the overload was not the use of calculation itself. The mistake was that the calculation did not impose a strong enough internal energy/stability boundary on the resulting architecture.

Therefore the cleanup must NOT flatten the 48 into generic conventional patches or remove specialist-engine contributions merely because Sylenth1/Spire do not use them.

## What must be preserved
For every normal factory preset preserve, unless evidence shows it is harmful or redundant:
- its intended musical role in a mix;
- the engine or combination of engines chosen because each contributes a specific sonic advantage;
- useful spectral separation between layers;
- transient/recognition carriers that make the sound cut through;
- controlled stereo behaviour and mono-safe low end;
- stochastic or specialist movement where it improves the sound rather than merely adding complexity;
- the preset's own identity, even when the result becomes cleaner or different from Sylenth1, Spire or Serum.

A cleaner OMNARIA sound does not need to sound more like another synthesizer.

## What may be removed or changed
Only remove or alter a contribution when one of these is true:
1. it duplicates energy already supplied by another layer without adding useful identity;
2. it creates masking/clouding greater than its musical benefit;
3. it introduces phase/stereo instability without a useful audible return;
4. it causes excessive peak/internal energy for the identity it contributes;
5. it is an accidental/default/obsolete modulation route rather than an intentional design choice;
6. it was a temporary engineering control used during development rather than part of the final sound concept.

## Design-time equation
Use the preset equation as a design and validation framework:

    Preset = musical target
           + best engine(s) for each sonic job
           + complementary layers
           + recognition / cut-through
           + useful movement
           + controlled width / translation
           - masking
           - redundant energy
           - instability
           - unnecessary CPU cost

The final term missing from the earlier work is an explicit safe-energy/stability constraint.

## Internal energy / stability boundary
Output trim alone is not a safety system. A sound can overload before its final `output` value is applied.

For each preset, evaluate the signal in stages:
1. CORE source contribution (Osc A/B, sub, noise);
2. unison contribution and its normalisation;
3. specialist-engine contribution and whether it occupies genuinely useful spectral/temporal territory;
4. modulation peaks that can raise drive, resonance, width or layer contribution;
5. nonlinear/filter stages and their gain compensation;
6. FX wet/feedback contribution;
7. final musical output trim.

The architecture is accepted only when every upstream stage remains finite and stable before final output trim.

A useful conceptual constraint is:

    E_useful = E_core + E_specialist + E_sub + E_effective_harmonic

subject to:

    internal_peak(stage_n) < safe_stage_limit

and

    benefit(layer_n) > masking(layer_n) + redundancy(layer_n)

This is a design-time test. Do not recalculate the whole equation every time a MIDI note is played.

## Layering rule
Multiple engines are not automatically better.

Each engine must justify itself by doing something the other layers do less well: cleaner fundamentals, richer upper identity, physical/resonant material, stochastic life, sharper transient recognition, dirtier nonlinear character, protected sub, etc.

If two layers do essentially the same job, prefer the one that gives the desired identity with less masking/energy/CPU cost.

## Unison and polyphony
Unison is internal to one MIDI voice and must be gain-stable before polyphony is considered.

Validation order:
- one note;
- representative 3–4 note chord;
- 8 notes;
- 16 notes;
- 32 notes on the heaviest appropriate presets.

The 32-note cap is a capacity target, not a demand that 32 maximum-velocity notes sum below 0 dBFS. The requirement is that increasing polyphony does not create internal nonlinear runaway, NaN/Inf, denormal storms, pathological CPU escalation or unexpected tonal collapse.

## NASTY isolation
Existing `nasty_*` values inside the 48 are legacy dependencies for now. Do not redesign them during this pass.

Inventory which normal sounds depend on NASTY, preserve the rest of those sounds, and later replace those low-level coefficients with a named, independently validated NASTY recipe/reference. NASTY gets its own nonlinear mathematics and stability analysis after the normal bank is clean.

## Corrected cleanup sequence
1. Inventory all 48 by category, musical purpose and engine contributions.
2. Flag every specialist layer with the specific sonic advantage it is meant to provide.
3. Flag legacy NASTY dependencies without modifying their maths yet.
4. Check for duplicate/obsolete/default parameter entries and accidental modulation routes.
5. Check masking/clouding and remove only contributions whose cost exceeds their audible benefit.
6. Check CORE/unison/sub/specialist energy before nonlinear/filter stages.
7. Check resonance/drive/filter-character and FX stages for internal peak growth.
8. Recalculate safe final output trims only after upstream stability is established.
9. Validate one-note behaviour first, then chord/polyphony behaviour up to the 32-note target.
10. Preserve OMNARIA identity throughout; do not homogenise the bank toward another synth.
11. After normal-bank sign-off, perform the separate NASTY mathematical/stability design pass.

## Acceptance test for each preset
A normal preset is finished when:
- its musical role is obvious;
- every active engine has a reason to be there;
- it cuts through where that sound type should cut through;
- no layer is creating avoidable clouding;
- low-frequency translation is controlled;
- stereo behaviour survives mono appropriately;
- one note is internally stable;
- polyphony does not expose runaway behaviour;
- final level is musically sensible;
- and the resulting sound still has OMNARIA's own identity.
