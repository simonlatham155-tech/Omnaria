#!/usr/bin/env python3
"""Prospective Wavetable→Physical reconstructed research renderer for Part 6.

Research model only; not recovered production code.
Uses the same frozen shared handoff schedule established earlier:
  mix = alpha ** 1.6
  coherence = (1 - alpha) ** 0.65
No pair-specific handoff timing is fitted.
"""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np

SAMPLE_RATE = 48_000
N = SAMPLE_RATE
EPS = 1e-12


def _norm(x):
    x = np.asarray(x, float)
    x -= np.mean(x)
    return 0.8 * x / (np.max(np.abs(x)) + EPS)


@dataclass
class WavetablePhysicalRenderer:
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        c = np.clip(np.asarray(controls, float), 0, 1)
        bright, body, movement, density, transient, grain, scatter, harmonicity = c
        a = float(np.clip(alpha, 0, 1))
        mix = a ** 1.6
        coherence = (1.0 - a) ** 0.65
        t = np.arange(N) / SAMPLE_RATE
        f0 = 110.0

        # Wavetable endpoint: continuous table morph with phase-warped harmonic blend.
        phase = 2*np.pi*f0*t
        morph = 0.45*density + 0.35*bright + 0.20*movement
        table_a = np.sin(phase)
        table_b = (2/np.pi) * np.arcsin(np.sin(phase + 0.25*scatter*np.sin(2*np.pi*(0.2+movement)*t)))
        table_c = np.tanh((1.0 + 5.0*body) * np.sin(phase + 0.6*harmonicity*np.sin(2*phase)))
        w1 = 1.0 - morph
        w2 = morph * (1.0 - bright)
        w3 = morph * bright
        wavetable = w1*table_a + w2*table_b + w3*table_c
        wavetable += 0.10*transient*np.sin(3*phase + movement*np.sin(2*np.pi*1.7*t))
        wavetable *= 1.0 + 0.06*movement*np.sin(2*np.pi*(0.15+2.0*movement)*t)

        # Physical endpoint: resonant body excited by deterministic impulses.
        freqs = np.array([110., 176., 287., 402., 641.]) * (0.94 + 0.12*harmonicity)
        decays = np.array([1.7, 1.15, 0.82, 0.60, 0.40]) * (0.55 + 1.2*body)
        physical = np.zeros(N)
        for k, (f, d) in enumerate(zip(freqs, decays)):
            env = np.exp(-t / max(0.04, d))
            detune = 1.0 + (1-harmonicity) * 0.004 * (k+1)
            physical += (k+1)**(-(0.75+1.8*(1-bright))) * env * np.sin(2*np.pi*f*detune*t + 0.1*rng.normal())
        # deterministic excitations
        for p in [0.05, 0.31, 0.63, 0.84]:
            i = int(p*N)
            L = min(64, N-i)
            physical[i:i+L] += (0.18+0.35*transient) * np.hanning(L)
        physical *= 0.35 + 0.65*body

        # Frozen schedule only; endpoint mechanisms are the prospective variables.
        x = coherence*wavetable + (1.0-coherence)*((1.0-mix)*wavetable + mix*physical)
        x += 0.02*scatter*np.sin(2*np.pi*(f0*1.011)*t + 0.3*np.sin(2*np.pi*0.37*t))
        return _norm(np.tanh((1.0+0.6*body)*x))
