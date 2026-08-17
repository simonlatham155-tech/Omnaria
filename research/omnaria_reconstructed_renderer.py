#!/usr/bin/env python3
"""Reconstructed OMNARIA research renderer for Part 6.

This is not recovered production code. It is an explicit research model built
from the current unified-engine hypothesis so that the Additive→Granular
transition experiment can be run reproducibly.

The renderer exposes one continuous parameterisation. Additive and granular
behaviour emerge from continuously varying excitation, event density and
micro-windowing rather than from a hard engine switch.
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
    peak = np.max(np.abs(x)) + EPS
    return 0.8 * x / peak


def _softclip(x: np.ndarray, drive: float) -> np.ndarray:
    d = 1.0 + 3.0 * drive
    return np.tanh(d * x) / np.tanh(d)


@dataclass
class ReconstructedOmnariaRenderer:
    """Continuous Additive↔Granular research model.

    controls order:
      brightness, body, movement, partial_density,
      transient_complexity, grain_size, scatter, harmonicity
    """

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        t = np.arange(N_SAMPLES, dtype=float) / SAMPLE_RATE
        c = np.clip(np.asarray(controls, dtype=float), 0.0, 1.0)
        bright, body, movement, density, transient, grain, scatter, harmonicity = c
        a = float(np.clip(alpha, 0.0, 1.0))

        f0 = 110.0
        # One shared oscillator field. As alpha rises, its phase coherence falls
        # and energy is increasingly re-windowed into micro-events.
        n_partials = int(8 + 48 * density)
        field = np.zeros_like(t)
        phases = rng.uniform(0.0, 2.0 * np.pi, n_partials)
        wander = np.zeros(n_partials)
        for k in range(1, n_partials + 1):
            tilt = 0.55 + 2.7 * (1.0 - bright)
            amp = k ** (-tilt)
            amp *= 0.55 + 0.65 * body
            inharm = (1.0 - harmonicity) * (0.004 + 0.020 * a)
            wander[k - 1] = rng.normal() * inharm
            fm_depth = movement * (0.0004 + 0.0018 * a)
            fm = fm_depth * np.sin(2.0 * np.pi * (0.17 + 0.91 * movement + 0.013 * k) * t)
            phase = 2.0 * np.pi * f0 * k * (1.0 + wander[k - 1]) * t
            phase += 2.0 * np.pi * f0 * k * np.cumsum(fm) / SAMPLE_RATE
            field += amp * np.sin(phase + phases[k - 1])

        field /= np.sqrt(max(1, n_partials))

        # Continuous event mask. At alpha=0 it remains near unity (additive-like).
        # Increasing alpha turns the same field into overlapping micro-events.
        event_rate = 4.0 + (12.0 + 190.0 * density) * (a ** 1.35)
        grain_ms = 4.0 + 116.0 * grain
        grain_len = max(8, int(grain_ms * 0.001 * SAMPLE_RATE))
        n_events = max(1, int(event_rate * DURATION_S))
        mask = np.full(N_SAMPLES, max(0.0, 1.0 - 1.08 * a), dtype=float)
        transient_layer = np.zeros(N_SAMPLES, dtype=float)

        for _ in range(n_events):
            center = int(rng.integers(0, N_SAMPLES))
            local_len = int(grain_len * np.exp(rng.normal(0.0, 0.35 * scatter * a)))
            local_len = int(np.clip(local_len, 8, N_SAMPLES))
            start = max(0, center - local_len // 2)
            end = min(N_SAMPLES, start + local_len)
            length = end - start
            if length < 4:
                continue

            w = np.hanning(length)
            weight = (0.15 + 0.95 * a) * (0.7 + 0.6 * rng.random())
            mask[start:end] += weight * w

            if transient > 0.0:
                tau = max(4.0, 0.08 * length)
                env = np.exp(-np.arange(length) / tau)
                noise = rng.normal(size=length)
                transient_layer[start:end] += transient * a * 0.14 * noise * env

        # Scatter acts as local time displacement of the shared oscillator field,
        # not as a second independent synthesis engine.
        if a > 0.0 and scatter > 0.0:
            max_shift = int((0.0003 + 0.010 * scatter * a) * SAMPLE_RATE)
            jitter = rng.integers(-max_shift, max_shift + 1, size=N_SAMPLES)
            idx = np.clip(np.arange(N_SAMPLES) + jitter, 0, N_SAMPLES - 1)
            scattered = field[idx]
        else:
            scattered = field

        coherence = (1.0 - a) ** 0.65
        eventised = scattered * mask
        x = coherence * field + (1.0 - coherence) * eventised + transient_layer

        # Body and movement remain shared global coordinates across the path.
        trem_rate = 0.18 + 2.7 * movement
        trem = 1.0 + (0.03 + 0.16 * a) * movement * np.sin(2.0 * np.pi * trem_rate * t)
        x *= trem
        x = _softclip(x, 0.08 + 0.22 * body)
        return _normalise(x)
