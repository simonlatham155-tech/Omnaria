# OMNARIA Synthesis Physics Library

Phase 8 uses equations first, presets second.

## Core synthesis mechanisms

### 1. Subtractive / virtual analogue
Rich periodic source followed by a time-varying filter:

x(t) = sum_k A_k sin(k w0 t + phi_k)
y(t) = H(t){x(t)}

Best at: stable pitch centre, familiar saw/pulse/sine identity, strong envelopes, analogue-style filtering.

### 2. Unison / beating
Multiple nearby oscillators:

u(t) = sum_i a_i x(2*pi*f0*2^(d_i/1200)*t + phi_i)
beat_ij = |f_i - f_j|

Important consequence: regular detune spacing reinforces repeated beat frequencies; non-uniform spacing distributes beating more densely.

Best at: width, density, ensemble character. Penalties: phase cancellation, blur, CPU.

### 3. Additive / sinusoidal

x(t) = sum_k A_k(t) sin(2*pi*f_k(t)t + phi_k(t))

Best at: explicit harmonic/inharmonic control, spectral precision, clean synthetic construction.

### 4. FM / PM

x(t) = A sin(w_c t + I(t) sin(w_m t))

Sidebands occur at f_c +/- n f_m and their amplitudes depend on the modulation index through Bessel-function behaviour.

Best at: adding many controlled spectral components economically; bright, metallic, bell-like, reed/brass-like and evolving spectra.

### 5. AM / ring modulation

x(t) = c(t) m(t)

For sinusoidal carrier/modulator this produces sum/difference components f_c +/- f_m.

Best at: sideband generation, metallic/interlocking spectra, controlled inharmonicity.

### 6. Waveshaping / nonlinear distortion

 y(t) = F(x(t))

The transfer function redistributes energy into new harmonics. Symmetric functions favour odd-harmonic structures; asymmetry introduces even components/DC tendencies.

Best at: harmonic density, warmth, bite, clipping/folding, controlled dirt.

### 7. Nonlinear feedback / coupled systems

s[n] = F(x[n] + beta s[n-1], coupling, damping, energy)

Best at: state-dependent spectra, thresholds, growl, instability, record-defining transitions. Requires stability guards.

### 8. Brown / constrained stochastic motion

b[n] = clamp(rho b[n-1] + sigma xi[n])

with rho close to 1 and small sigma.

Best at: tight non-periodic micro-motion that remains near the current state; basses, plucks, leads, small analogue-like irregularity.

### 9. Broad stochastic evolution

q[n] approaches stochastic targets q* over longer random intervals rather than repeating a periodic LFO path.

Best at: slow organic non-repetition, pads, strings, atmospheres, evolving upper spectrum.

### 10. Wave-terrain synthesis

z(t) = T(x(t), y(t))

where x(t), y(t) define an orbit through a 2-D terrain T. Changing the orbit changes the resulting waveform/harmonic trajectory.

Best at: correlated nonlinear timbral motion, evolving spectra that are not simply wavetable scans.

### 11. Granular synthesis

x(t) = sum_g a_g w_g(t-t_g) s_g(t-t_g)

Best at: density from many short sound particles, time/pitch decoupling, clouds, texture, microsound.

### 12. Sampling / resampling

x(t) = s(alpha t + tau)

plus interpolation, looping, scanning and processing.

Best at: real transients, complex recorded spectra, unique moments, captured nonlinear events.

### 13. Modal / physical-model resonators

A simple modal component can be represented as a damped resonator:

r_k(t) = A_k exp(-d_k t) sin(2*pi*f_k t + phi_k)
y(t) = sum_k r_k(t)

Digital waveguide forms model travelling waves using delays, filters and scattering/nonlinear elements.

Best at: bodies, materials, strings/tubes/plates, mallet/bowed/plucked behaviours, energy exchange.

### 14. Deterministic + stochastic spectral decomposition

x(t) = d(t) + s(t)

where d(t) contains tracked deterministic partials and s(t) contains stochastic/noise-like residue.

Best at: separating stable pitched identity from air/noise/texture instead of forcing one synthesis method to do both.

## OMNARIA optimisation principle

For each preset define target acoustic vector:

T = [P,H,D,M,T,C,W,N,X]

P pitch/fundamental stability
H harmonic density/fullness
D dynamic expression
M spectral movement
T transient definition
C coherence/clarity
W stereo dimensionality
N nonlinear character
X non-repetition/complexity

Each engine contributes a vector E_j, and coefficients a_j are solved conceptually/experimentally:

S = sum_j a_j E_j

Minimise:

J = sum_i w_i (T_i - S_i)^2
    + lambda_alias A
    + lambda_phase Phi
    + lambda_mud MUD
    + lambda_pitch PD
    + lambda_cpu CPU

The reference from Sylenth1/Spire supplies the baseline target R. OMNARIA aims for:

T = R + Delta

Delta is near zero for mature analogue-derived sounds and deliberately larger where OMNARIA's additional synthesis physics can produce more movement, expression, texture or nonlinear identity.

## Non-negotiable rule

An engine is not included because it exists. It is included only when its mathematical behaviour addresses a target deficit more efficiently or musically than another copy of the existing source.
