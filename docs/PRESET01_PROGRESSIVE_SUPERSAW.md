# Preset 01 — Progressive Supersaw

Status: BUILDING / NOT YET ACCEPTED

## Benchmark target

This preset is the first Phase 8 gate. It must compete directly with strong Sylenth1 and Spire supersaw/lead patches before preset 02 is allowed into the factory bank.

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

Published JP-8000 supersaw analysis identifies a centre saw plus six asymmetrically/non-uniformly detuned side saws. This is the important lesson, not the need to clone a JP-8000 exactly.

Random/free initial phase is being used for the first OMNARIA candidate because repeated identical phase starts can create a static comb-like attack. We will A/B this against retriggered phase during listening because transient consistency can sometimes be preferable in a lead.

## Candidate A — clean CORE baseline

- saw + saw
- 7 unison voices
- 13.2-cent current OMNARIA detune parameter
- broad but not maximum stereo spread
- random initial phase
- LP24
- no NASTY
- no SAMPLE
- no Brown/Stochastic modulation
- no chorus/delay/reverb during the dry benchmark
- neutral filter character

This deliberately prevents specialist engines and FX from hiding weaknesses in CORE.

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

## CORE issue discovered by preset 01

Current OMNARIA unison position uses a smooth symmetric spacing function. The next DSP experiment should compare this against a deliberately non-uniform 7-voice detune table inspired by measured supersaw behaviour. The winning method should be chosen by level-matched listening and mono/stereo analysis, not historical imitation.

## Acceptance gate

Preset 01 is not finished because values exist in code. It passes only after:
- universal build is green
- dry OMNARIA CORE is compared against strong Sylenth1 and Spire supersaw references
- phase mode A/B is judged
- current smooth detune distribution vs non-uniform distribution is judged
- any specialist-engine addition must beat CORE-only rather than merely sound more complex

Only then may preset 02 be added.
