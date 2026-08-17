#!/usr/bin/env python3
"""Modal→WaveTerrain reconstructed renderer using the frozen Part-6 handoff law.

Research model only; not recovered production code.
The Additive→Granular handoff schedule is intentionally unchanged:
  mix = alpha ** 1.6
  coherence = (1 - alpha) ** 0.65
No pair-specific transition fitting is used here.
"""
from __future__ import annotations

from dataclasses import dataclass
import numpy as np

SAMPLE_RATE = 48_000
DURATION_S = 1.0
N_SAMPLES = int(SAMPLE_RATE * DURATION_S)
EPS = 1e-12


def _normalise(x: np.ndarray) -> np.ndarray:
    x = np.asarray(x, dtype=float)
    x -= np.mean(x)
    return 0.8 * x / (np.max(np.abs(x)) + EPS)


def _softclip(x: np.ndarray, drive: float) -> np.ndarray:
    d = 1.0 + 3.0 * drive
    return np.tanh(d * x) / np.tanh(d)


def _sigmoid(x: float | np.ndarray) -> float | np.ndarray:
    return 1.0 / (1.0 + np.exp(-np.clip(x, -60.0, 60.0)))


@dataclass
class ModalWaveTerrainRenderer:
    """Continuous Modal↔WaveTerrain model with the frozen handoff schedule."""

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        t = np.arange(N_SAMPLES, dtype=float) / SAMPLE_RATE
        bright, body, movement, density, transient, grain, scatter, harmonicity = np.clip(
            np.asarray(controls, dtype=float), 0.0, 1.0
        )
        a = float(np.clip(alpha, 0.0, 1.0))

        # Frozen from Additive→Granular. Do not tune these per pair.
        mix = a ** 1.6
        coherence = (1.0 - a) ** 0.65

        q = 0.55 * density + 0.45 * transient
        structural = (1.0 - mix) * density + mix * q
        dynamic = (1.0 - mix) * movement + mix * (0.60 * movement + 0.40 * scatter)
        f0 = 110.0

        # Modal endpoint: smoothly activated damped resonances.
        max_modes = 40
        phases = rng.uniform(0.0, 2.0 * np.pi, max_modes)
        modal = np.zeros_like(t)
        target_modes = 5.0 + 30.0 * structural
        for k in range(1, max_modes + 1):
            gate = float(_sigmoid((target_modes - k) / 1.1))
            ratio = k + (1.0 - harmonicity) * 0.08 * (k ** 1.15)
            freq = f0 * ratio * (1.0 + 0.004 * rng.normal())
            decay = 0.18 + 1.6 * body / (1.0 + 0.055 * k)
            env = np.exp(-t / max(0.03, decay))
            amp = gate * (k ** (-(0.7 + 2.3 * (1.0 - bright))))
            modal += amp * env * np.sin(2.0 * np.pi * freq * t + phases[k - 1])
        modal /= np.sqrt(max(1.0, target_modes))
        retrig = 0.5 + 0.5 * np.sin(2.0 * np.pi * (0.35 + 1.2 * movement) * t)
        modal *= 0.78 + 0.22 * retrig

        # WaveTerrain endpoint: one oscillator orbit traversing a continuous 2-D terrain.
        xcoord = np.sin(
            2.0 * np.pi * f0 * t
            + 0.8 * dynamic * np.sin(2.0 * np.pi * (0.2 + 1.4 * movement) * t)
        )
        ycoord = np.sin(
            2.0 * np.pi * (f0 * (1.0 + 0.5 * harmonicity)) * t
            + 2.0 * np.pi * scatter * np.sin(2.0 * np.pi * (0.13 + 0.9 * movement) * t)
        )
        terrain = (
            np.sin(np.pi * (1.0 + 3.0 * structural) * xcoord)
            + 0.6 * np.sin(np.pi * (2.0 + 4.0 * structural) * ycoord)
            + 0.35 * np.sin(np.pi * (xcoord * ycoord) * (2.0 + 6.0 * q))
        )
        terrain += 0.18 * transient * np.tanh(4.0 * xcoord * ycoord)
        terrain /= 1.95
        terrain = np.tanh((1.0 + 1.8 * body) * terrain)
        terrain += 0.10 * bright * np.sin(np.pi * (5.0 + 5.0 * bright) * (xcoord + ycoord))

        signal = coherence * modal + (1.0 - coherence) * terrain
        signal *= 1.0 + 0.06 * movement * np.sin(
            2.0 * np.pi * (0.18 + 2.7 * movement) * t
        )
        return _normalise(_softclip(signal, 0.08 + 0.22 * body))
