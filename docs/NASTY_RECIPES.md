# OMNARIA Phase 3 — NASTY Audition Recipes

These are engineering starting recipes, not release presets. They deliberately spread across four nonlinear models and different stability regimes so Phase 3 can be judged on vocabulary rather than on twenty distortion variants.

All values are 0..1 except Feedback (0..0.985) and Damping (0.02..1). Amount is dry/wet contribution. Moment is the one-shot tension -> peak -> recovery trajectory strength.

| # | Working name | Model | Amount | Deform | Feedback | Coupling | Energy | Damping | Moment | Target behaviour |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---|
| 1 | Rubber Bite | Feedback | .42 | .28 | .46 | .05 | .31 | .68 | .18 | rubber edge |
| 2 | Acid Animal | Feedback | .58 | .45 | .67 | .12 | .52 | .51 | .30 | squelch / vocal |
| 3 | Hydraulic Bark | Feedback | .72 | .70 | .76 | .15 | .61 | .43 | .46 | bark |
| 4 | Feedback Scream | Feedback | .78 | .82 | .88 | .10 | .79 | .31 | .68 | scream event |
| 5 | Paper Rip | Fold | .36 | .31 | .18 | .00 | .22 | .72 | .12 | dry tearing transient |
| 6 | Razor Fold | Fold | .59 | .61 | .31 | .00 | .48 | .58 | .25 | bright rip |
| 7 | Broken Speaker | Fold | .76 | .84 | .48 | .00 | .67 | .46 | .52 | collapsing fold |
| 8 | Rave Tear | Fold | .82 | .93 | .59 | .00 | .78 | .37 | .70 | record-event tear |
| 9 | Iron Throat | Coupled | .45 | .32 | .16 | .43 | .38 | .70 | .16 | resonant throat |
| 10 | Metal Dog | Coupled | .61 | .47 | .24 | .63 | .51 | .57 | .28 | metallic bark |
| 11 | Hydraulic Machine | Coupled | .69 | .59 | .32 | .79 | .66 | .48 | .42 | machinery motion |
| 12 | Resonator Fight | Coupled | .77 | .72 | .38 | .91 | .78 | .39 | .56 | modes fighting |
| 13 | Factory Collapse | Coupled | .83 | .81 | .51 | .96 | .88 | .28 | .78 | peak then recovery |
| 14 | Soft Duff | Duffing | .34 | .24 | .13 | .00 | .29 | .75 | .14 | organic nonlinear body |
| 15 | Duff Growl | Duffing | .53 | .46 | .28 | .00 | .51 | .58 | .28 | growl |
| 16 | Cubic Bark | Duffing | .66 | .62 | .43 | .00 | .68 | .47 | .43 | threshold bark |
| 17 | Duff Scream | Duffing | .75 | .78 | .57 | .00 | .82 | .35 | .63 | nonlinear scream |
| 18 | Near Chaos | Duffing | .81 | .89 | .69 | .00 | .91 | .25 | .82 | controlled near-chaos |
| 19 | Brown Machine | Coupled | .64 | .55 | .28 | .72 | .59 | .52 | .35 | add Brown -> Coupling/Energy for tight living motion |
| 20 | Stochastic Creature | Feedback | .68 | .64 | .72 | .14 | .70 | .41 | .55 | add Stochastic -> Deform/Feedback for broad evolving behaviour |

## Modulation stress tests

- **Tight movement:** Brown -> NASTY Coupling at +0.20 to +0.45. It should move locally without wandering away from the patch identity.
- **Lush/organic movement:** Stochastic -> NASTY Deform or Energy at +0.15 to +0.35. It should breathe rather than chatter.
- **Performance bite:** Velocity or Aftertouch -> NASTY Amount/Energy at +0.15 to +0.50.
- **Deliberate event:** one-shot LFO/Env -> NASTY Moment or Feedback. This should produce a repeatable record-defining event rather than permanent aggression.
- **Recovery check:** Moment > 0 must return toward the parent sound after its peak instead of leaving the nonlinear system numerically latched.

## Acceptance protocol

1. Render the same MIDI phrase and source patch through all 20 recipes at matched output level.
2. Reject any recipe that is merely a louder version of another.
3. Reject recipes whose useful identity is aliasing or numerical instability rather than intended nonlinear behaviour.
4. Keep at least one recognisably different useful family from Fold, Feedback, Coupled and Duffing.
5. The final Phase 3 listening gate is 20 genuinely distinct, record-usable aggressive moments across rubber, squelch, bark, tear, metallic, scream and WTF territory.
6. Do not claim a new synthesis method from these known ingredients. A new-engine claim requires a later prior-art review and a demonstrably distinct coupled topology.
