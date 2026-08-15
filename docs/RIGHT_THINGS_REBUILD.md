# OMNARIA — Right Things Rebuild

## Product rule

OMNARIA does not win by containing the most synthesis methods. It wins by containing the right sound-making systems, implementing them deeply, and giving the producer complete control over every result.

## Four pillars

### 1. CORE
Conventional dance-music sounds must compete directly with the strongest reference sounds from Sylenth1, Spire and Serum before experimental enhancement is counted.

For each sound category, choose the strongest starting reference first. The winner may differ by category. OMNARIA then preserves the reference sound's useful identity, borrows measurable strengths from the other references, and only then tests additional DSP enhancements.

### 2. NASTY
A dedicated research path for aggressive, unstable, rubbery, tearing, barking, resonant and machine-like sounds. Primary musical references include the design language heard across Dylan Rhymes, The Prodigy and The Chemical Brothers.

NASTY is not a distortion macro. Candidate mechanisms include nonlinear oscillator coupling, stateful phase deformation, resonant feedback, coupled modal systems, controlled bifurcation/instability, waveshaping and internal resampling. No new synthesis name is claimed unless a distinct mechanism is formally demonstrated.

### 3. SAMPLE
Sampling is a synthesis source, not only playback. The target path is:

sample -> slice/loop/scrub/pitch -> deform/resonate/couple -> feedback/resample -> editable result

Future research may allow sample read position, grain state and derived buffers to participate in the same state equations as oscillators/resonators. The original source must remain recoverable.

### 4. DISCOVER
DISCOVER behaves like an expert sound designer operating OMNARIA's real controls.

Every result must be represented by ordinary editable state:

- real parameter values move
- new modulation routes are visible and editable
- envelopes remain normal envelopes
- distortion/feedback/coupling remain normal controls
- no hidden sound-only parameters unavailable to the producer
- changes are undoable
- locks prevent protected sound DNA from moving

DISCOVER evolves from the current sound. It is not unrestricted randomisation.

## Benchmark workflow

For each important sound family:

1. Compare Sylenth1, Spire and Serum reference patches at matched loudness.
2. Select the best baseline for that specific sound.
3. Document why it wins: spectrum, unison, transient, filter/envelope interaction, stereo image, movement, CPU and mix placement.
4. Identify useful traits in the two runners-up.
5. Build an OMNARIA baseline that matches the winner before adding experimental DSP.
6. Test targeted enhancements such as BODY, LIFE, BITE, IMPACT, MOTION or NASTY.
7. Keep an enhancement only when matched A/B testing says it improves the musical result.
8. Add expression mappings only where they improve performance rather than decorate it.
9. Use the finished parent sound as DISCOVER DNA.

## Producer-facing enhancement vocabulary

Internal maths may be complex; producer controls should describe musical behaviour.

- BODY — modal/resonant reinforcement
- AIR — controlled high-frequency excitation
- MOTION — useful internal evolution
- BITE — PM/FM/nonlinear harmonic edge
- IMPACT — transient reinforcement/excitation
- WEIGHT — controlled fundamental/low-mode support
- LIFE — subtle per-note/state variation
- NASTY — controlled nonlinear interaction/instability

These are candidate behaviours, not mandatory knobs. A control is added only when it survives musical testing.

## DISCOVER contract

A patch is represented by its synthesis/routing graph plus its parameter state. DISCOVER may modify both, but only through producer-visible structures.

Initial implementation stages:

1. Relative parameter evolution from the current patch.
2. Parameter/domain locks.
3. Per-Discover change history and selective revert.
4. Visible modulation-route creation/removal.
5. Candidate generation plus audio-feature ranking.
6. FAMILIAR <-> WTF exploration distance.
7. Quality-diversity search constrained by playability, safe level, CPU and locked sound DNA.

## Reference categories

The initial benchmark library should cover at least:

- init saw
- mono sub
- rolling dance bass
- Reese
- 7/9 voice supersaw
- trance lead
- trance pluck
- short stab
- wide pad
- evolving pad
- acid/resonant sequence
- Hoover/rave sound
- hard sync/PM attack
- NASTY bass
- NASTY stab/noise

Reference patches are engineering targets, not release copies.

## Definition of improvement

"More complex" is not improvement. An OMNARIA variant wins only if it produces a useful audible advantage such as stronger body, cleaner highs, greater punch, better mono compatibility, more controllable width, richer motion, more expressive response, lower CPU, better mix placement, or a genuinely valuable new sound behaviour.
