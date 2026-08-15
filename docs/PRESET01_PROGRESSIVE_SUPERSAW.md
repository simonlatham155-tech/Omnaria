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

Published JP-8000 supersaw analysis identifies a centre saw plus six asymmetrically/non-uniformly detuned side saws. Measured relative frequency offsets reported from that work are approximately -0.1100, -0.0629, -0.0195, 0, +0.0199, +0.0622, +0.1075 times the detune factor. The lesson is non-uniform beat distribution, not cloning the JP-8000.

Random/free initial phase is used for the current OMNARIA candidate because repeated identical phase starts can create a static comb-like attack. We will A/B this against retriggered phase during listening because transient consistency can sometimes be preferable in a lead.

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

## Candidate B — CORE plus sub-perceptual Brown decorrelation

Candidate B preserves the same CORE identity but uses Brown motion only to vary the unison detune magnitude by a very small amount.

Brown state:

b[n] = clamp(rho * b[n-1] + sigma * xi[n], -1, 1)

Current OMNARIA Brown source is bounded and heavily correlated in time. With modulation depth 0.035 at the Detune destination, the existing modulation law produces approximately +/-0.63 cent maximum variation around the 13.2-cent detune setting.

The centre frequency itself is not pitch-modulated. Brown changes only the width of the detuned cloud, so the intended acoustic effect is:
- reduce stationary/repeating beat relationships
- increase long-term spectral density
- preserve centre-pitch certainty
- remain below the point where the listener hears obvious pitch wobble

Candidate B is now the active factory-program implementation for preset 01.

## Candidate C — conditional engine enrichment

Candidate C is not encoded yet. It is allowed only if measurements show a remaining deficit after A/B. Possible tools:
- tiny nonlinear/coupled contribution for harmonic density
- broader stochastic evolution only in sustained upper-spectrum behaviour
- no SAMPLE unless a specific transient/texture deficit is identified

Candidate C must not be created merely because OMNARIA has more engines available.

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
11. Candidate B must reduce beat periodicity without producing audible vibrato.
12. Brown contribution must lose if it weakens pitch certainty or transient punch.

## CORE issue discovered by preset 01

Current OMNARIA unison position uses a smooth symmetric spacing function. A later DSP experiment must compare this against a deliberately non-uniform 7-voice detune table based on measured supersaw behaviour. Pitch-detune distribution and stereo-pan distribution must remain separate calculations: a useful frequency-spacing law is not automatically the best stereo law.

## Acceptance gate

Preset 01 is not finished because values exist in code. It passes only after:
- universal build is green
- dry OMNARIA CORE is compared against strong Sylenth1 and Spire supersaw references
- phase mode A/B is judged
- current smooth detune distribution vs non-uniform distribution is judged
- Candidate A vs Candidate B is judged level-matched
- any Candidate C specialist-engine addition must beat the best A/B result rather than merely sound more complex

Only then may preset 02 be added.
