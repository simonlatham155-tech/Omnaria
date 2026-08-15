# LatWorld — preserved research hypothesis

**Status: research only. Not an OMNARIA product claim and not currently a named synthesis method.**

This document preserves the LatWorld idea so it is not lost while OMNARIA is developed. The name must not return to the product, UI, marketing or public synthesis description unless development produces a genuinely distinct mathematical sound-generation method.

## Original hypothesis

The proposed idea was a shared, stateful musical environment in which multiple synthesis processes could react to common musical state, history, spectral occupancy and compute resources. Possible components included:

- host-aware beat, bar and phrase state
- history-dependent behaviour
- harmonic attraction / constraint
- multiple timbres sharing state rather than behaving as unrelated plug-in instances
- synthesis renderers that can exchange or reinterpret a common sound representation
- dynamic allocation of expensive DSP resources
- sample analysis/resynthesis feeding the same shared state
- a centre globe that visualises real engine variables

Potential renderers/material sources discussed during ideation included virtual analogue, wavetable, FM/PM, wave-terrain, stochastic, spectral/additive, granular, physical/modal and sampling/resynthesis.

## What would NOT justify calling this a new synthesis method

LatWorld is not novel merely because OMNARIA contains many synthesis engines. It is also not a new synthesis method if the implementation reduces to any of the following:

- crossfading between existing engines
- macro control of conventional parameters
- ordinary modulation matrices driven by host position
- randomisation constrained to a scale or chord
- dynamic EQ or sidechain processing after sound generation
- conventional multitimbrality with shared effects
- standard voice allocation or CPU sleeping
- a visual globe that does not correspond to a new DSP model

Those may all be useful OMNARIA features, but they do not justify a new synthesis category.

## Mathematical direction worth investigating

A genuine candidate would need a persistent synthesis state whose evolution is itself part of sound generation, for example a state-space system of the general form:

`x[n+1] = F(x[n], u[n], h[n], r[n]; theta)`

where:

- `x` is the persistent internal synthesis state
- `u` is current musical/performance input
- `h` is accumulated musical history
- `r` is shared resource/spectral state across active timbres
- `theta` contains controllable synthesis rules

Audio would then be rendered from that evolving state:

`y[n] = G(x[n], u[n]; phi)`

The important research question is whether `F` and `G` can be designed so that the persistent coupled state creates musically controllable timbral behaviour that is not equivalent to a conventional oscillator/filter/modulation graph.

Possible mathematical areas to investigate while OMNARIA is built:

- nonlinear dynamical systems and attractors
- coupled oscillators / Kuramoto-style phase systems
- reaction-diffusion systems
- agent/population dynamics
- cellular automata with continuous state
- spectral particle fields
- energy-conserving coupled resonator networks
- differentiable or adaptive DSP where the topology/state itself changes
- graph-based synthesis in which nodes exchange energy or spectral mass

## Revival gates

The LatWorld name may be reconsidered only if all of these are satisfied:

1. **Formal model:** we can write down the core state equations/algorithm precisely.
2. **Distinct mechanism:** the result cannot reasonably be described as only conventional synthesis plus modulation, sequencing, crossfading or effects.
3. **Sonic consequence:** the mathematical mechanism produces a recognisable and useful class of sounds/behaviours.
4. **Control:** a musician can intentionally steer it; novelty alone is insufficient.
5. **Repeatability:** deterministic operation is available for DAW recall/offline rendering when required.
6. **Efficiency:** the method is practical in a polyphonic real-time instrument.
7. **Prior-art check:** relevant academic and commercial work is reviewed before making a novelty claim.
8. **A/B proof:** removing the new mechanism materially removes the defining behaviour.

Until those gates are met, OMNARIA is simply the LATHAMAUDIO flagship synthesizer and any experimental state mechanics are described by what they actually do rather than by a new synthesis name.

## Product relationship

OMNARIA may still provide the research environment that leads to such a method. Experimental performance-state tracking, mathematical oscillators, spectral models, sampling/resynthesis and shared DSP structures can be developed as ordinary engineering features. If a genuinely new synthesis principle emerges from that work, this document is the point from which LatWorld research can resume.
