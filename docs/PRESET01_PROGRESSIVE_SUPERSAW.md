# Preset 01 — Progressive Supersaw

Status: BUILDING / NOT YET ACCEPTED

## Benchmark target

This preset is the first Phase 8 gate. It must compete directly with strong Sylenth1 and Spire supersaw/lead patches before preset 02 is allowed into the factory bank.

## Seven-stage Part 1 process

1. Reference equation — define the Sylenth1/Spire acoustic target.
2. Unison geometry — separate frequency spacing from stereo spacing and reduce repeated beat families.
3. Keyboard optimisation — make detune behaviour useful across low and high registers.
4. Energy architecture — preserve a strong centre while tapering and power-normalising side voices.
5. Multi-stochastic evolution — give Brown and broad stochastic processes different physical jobs.
6. Phase/transient optimisation — compare all-random, all-retriggered and hybrid centre/side phase laws.
7. Final acceptance — level-matched benchmark plus mono, streaming/small-speaker, stereo, range, peak, alias and CPU checks.

Current stage: 5 of 7.

## What the benchmark synths teach us

### Sylenth1
- four bandlimited full-stereo unison oscillators
- up to eight unison voices per oscillator
- nonlinear multi-stage filtering / drive
- very low CPU relative to voice density

Acoustic consequence: its strength is not exotic synthesis. It is dense, clean, phase-coherent virtual-analogue stacking with good gain staging and filters.

### Spire
- four multimode oscillators
- nine unison voices per oscillator
- explicit unison engine capable of supersaw/hypersaw-style spreads
- analogue/digital filter choices and strong built-in effects

Acoustic consequence: OMNARIA must not assume seven voices alone makes a competitive supersaw. Density, distribution and stereo behaviour matter.

## Supersaw physics

For a saw at fundamental f0, each detuned voice produces harmonic lines at n * fi. Summing several nearby fi creates time-varying interference. If offsets are regularly spaced, many oscillator pairs share similar difference frequencies, reinforcing an obvious common beat/flange rate. Irregular/nonlinear spacing distributes those difference frequencies and produces denser, less periodic motion.

Published JP-8000 supersaw analysis identifies a centre saw plus six asymmetrically/non-uniformly detuned side saws. Measured relative frequency offsets reported from that work are approximately -0.1100, -0.0629, -0.0195, 0, +0.0199, +0.0622, +0.1075 times the detune factor. The lesson is non-uniform beat distribution, not cloning the JP-8000.

OMNARIA now separates frequency position, stereo position and per-voice energy. The seven-voice law also uses pitch-aware detune so the same public detune value does not create excessively slow low-note beating and frantic high-note beating.

## Candidate A — clean CORE baseline

- saw + saw
- 7 unison voices
- 13.2-cent public detune setting
- broad but not maximum stereo spread
- random initial phase
- LP24
- no NASTY
- no SAMPLE
- no stochastic modulation
- no chorus/delay/reverb during the dry benchmark
- neutral filter character

This deliberately prevents specialist engines and FX from hiding weaknesses in CORE.

## Candidate B1/B2 — optimised CORE geometry and energy

The current CORE path uses:
- non-uniform seven-voice frequency placement
- separate stereo placement
- pitch-aware detune
- centre-weighted side-voice energy
- power matching against the previous equal-gain implementation

These changes are intended to improve useful beat density, centre-pitch certainty, mono survival and mass-playback translation without allowing extra loudness to win the comparison.

## Candidate B3 — multi-stochastic evolution

Brown and broad stochastic motion are intentionally not interchangeable.

### Brown job: detune-cloud breathing

b[n] = clamp(rho * b[n-1] + sigma * xi[n], -1, 1)

d_eff(t) = d0 + 18 * 0.035 * b(t) cents

Therefore the maximum change is approximately +/-0.63 cent. Brown changes cloud width rather than global pitch. The centre voice has zero detune position, so its fundamental stays fixed.

### Broad stochastic job: slow population-energy redistribution

The first audible B3 experiment routes the broader stochastic source to Osc Mix at depth 0.045. Osc A and Osc B are same-pitch, independently phased saw populations, so small slow movement between them changes interference density without deliberately moving the fundamental.

A more exact side-voice energy-shape law also exists in SupersawLaw. It redistributes energy between inner and outer beat families symmetrically and includes exact power correction. It remains an A/B candidate rather than being globally hard-wired into every preset.

The rule is: Brown solves local correlated micro-motion; broad stochastic solves slower non-repeating redistribution. Neither is included merely because it is available.

## Stage 6 hypothesis — hybrid phase law

Current candidate A/B uses all-random initial phase. Stage 6 will compare:
- all random: strongest non-repetition, least repeatable transient
- all retriggered: strongest repeatability, greatest risk of a fixed comb-like attack
- hybrid: deterministic centre phase with random outer phases

The hybrid hypothesis is attractive because the centre can supply a reliable transient and pitch cue while the outer population retains non-repeating interference. It must still win by listening and transient/peak efficiency, not by theory alone.

## Candidate C — conditional engine enrichment

Candidate C is not allowed merely because OMNARIA contains more engines. After Stage 6, FM/PM, nonlinear/NASTY, SAMPLE, wave-terrain or other synthesis mechanisms may enter only when a specific measured acoustic deficit remains and their contribution beats the best B result after loudness matching.

## Measurements / listening checks

1. Mono sum must not hollow out badly.
2. Repeated notes must not reveal one dominant flange/beat period.
3. Low notes must retain a defined pitch centre.
4. High notes must not acquire obvious aliasing/metallic foldback.
5. Seven voices must not simply win by loudness; reference levels must be matched.
6. Attack must feel immediate enough for progressive/trance lead work without a click.
7. Stereo image should be wide but centre energy must survive.
8. Filter opening must remain smooth and energetic.
9. Remove all FX when comparing oscillator/filter quality.
10. CPU cost must remain proportionate to the audible gain.
11. Brown must reduce stationary beating without producing audible vibrato.
12. Broad stochastic motion must add non-repetition without pitch drift, image wander or obvious pumping.
13. Peak/crest-factor cost must be justified by perceived density.
14. The sound must retain its identity through mono and mass-listening bandwidth/level conditions.

## Acceptance gate

Preset 01 is not finished because values exist in code. It passes only after:
- universal build is green
- dry OMNARIA CORE is compared against strong Sylenth1 and Spire supersaw references
- phase mode / hybrid phase A/B is judged
- Candidate A vs optimised B is judged level-matched
- Brown-only vs Brown + broad stochastic is judged
- mono, streaming/small-speaker, stereo and keyboard-range checks pass
- any Candidate C specialist-engine addition beats the best A/B result rather than merely sounding more complex

Only then may preset 02 be added.
