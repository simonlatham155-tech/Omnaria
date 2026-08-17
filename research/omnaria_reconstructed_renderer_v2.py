#!/usr/bin/env python3
"""Reconstructed OMNARIA renderer v2 for Part 6.

Research model only; not recovered production code.

v2 removes integer partial/event-count discontinuities and introduces a shared
Granular microstructure coordinate so PartialDensity and TransientComplexity
can acquire the strong local response coupling observed in Part 5 while
retaining distinct residual behaviours.
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
class ReconstructedOmnariaRendererV2:
    """Continuous Additive↔Granular research model with smooth local geometry."""

    granular_distinctness: float = 12.0

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        t = np.arange(N_SAMPLES, dtype=float) / SAMPLE_RATE
        c = np.clip(np.asarray(controls, dtype=float), 0.0, 1.0)
        bright, body, movement, density, transient, grain, scatter, harmonicity = c
        a = float(np.clip(alpha, 0.0, 1.0))
        mix = a ** 1.6

        # Shared Granular microstructure coordinate. At alpha=0 the controls are
        # mostly independent; toward alpha=1 they increasingly act through q.
        q = 0.55 * density + 0.45 * transient
        structural = (1.0 - mix) * density + mix * q
        transient_drive = (1.0 - mix) * transient + mix * q
        distinct = self.granular_distinctness

        f0 = 110.0
        max_partials = 56
        phases = rng.uniform(0.0, 2.0 * np.pi, max_partials)
        wanders = rng.normal(size=max_partials)
        field = np.zeros_like(t)

        # Smooth partial activation replaces integer n_partials steps.
        target_partials = 8.0 + 48.0 * (
            structural + distinct * mix * (density - q) * 0.25
        )
        target_partials = float(np.clip(target_partials, 1.0, max_partials))
        for k in range(1, max_partials + 1):
            gate = float(_sigmoid((target_partials - k) / 1.2))
            tilt = 0.55 + 2.7 * (1.0 - bright)
            amp = gate * (k ** -tilt) * (0.55 + 0.65 * body)
            wander = wanders[k - 1] * (1.0 - harmonicity) * (0.004 + 0.020 * a)
            fm_depth = movement * (0.0004 + 0.0018 * a)
            fm = fm_depth * np.sin(
                2.0 * np.pi * (0.17 + 0.91 * movement + 0.013 * k) * t
            )
            phase = 2.0 * np.pi * f0 * k * (1.0 + wander) * t
            phase += 2.0 * np.pi * f0 * k * np.cumsum(fm) / SAMPLE_RATE
            field += amp * np.sin(phase + phases[k - 1])
        field /= np.sqrt(max(1.0, target_partials))

        grain_len = max(8, int((4.0 + 116.0 * grain) * 0.001 * SAMPLE_RATE))
        mask = np.full(N_SAMPLES, max(0.0, 1.0 - 1.08 * a), dtype=float)
        transient_layer = np.zeros(N_SAMPLES, dtype=float)

        # Fixed event pool + sigmoid activation replaces integer n_events.
        max_events = 220
        centers = rng.integers(0, N_SAMPLES, size=max_events)
        length_noise = rng.normal(size=max_events)
        weight_noise = rng.random(max_events)
        target_events = 4.0 + (12.0 + 190.0 * structural) * (a ** 1.35)

        for event_i in range(max_events):
            active = float(_sigmoid((target_events - (event_i + 1)) / 2.0))
            if active < 1e-5:
                continue
            local_len = int(grain_len * np.exp(length_noise[event_i] * 0.35 * scatter * a))
            local_len = int(np.clip(local_len, 8, N_SAMPLES))
            start = max(0, int(centers[event_i]) - local_len // 2)
            end = min(N_SAMPLES, start + local_len)
            length = end - start
            if length < 4:
                continue

            window = np.hanning(length)
            weight = active * (0.15 + 0.95 * a) * (0.7 + 0.6 * weight_noise[event_i])
            weight *= 1.0 + distinct * mix * 0.25 * (density - q)
            mask[start:end] += weight * window

            event_rng = np.random.default_rng(seed + 10_000 + event_i)
            noise = event_rng.normal(size=length)
            tau = max(4.0, (0.08 + distinct * mix * 0.04 * (q - transient)) * length)
            env = np.exp(-np.arange(length) / tau)
            burst_amp = transient_drive * (1.0 + distinct * mix * 0.35 * (transient - q))
            transient_layer[start:end] += active * burst_amp * a * 0.14 * noise * env

        if a > 0.0 and scatter > 0.0:
            jitter_rng = np.random.default_rng(seed + 999_999)
            max_shift = int((0.0003 + 0.010 * scatter * a) * SAMPLE_RATE)
            jitter = jitter_rng.integers(-max_shift, max_shift + 1, size=N_SAMPLES)
            idx = np.clip(np.arange(N_SAMPLES) + jitter, 0, N_SAMPLES - 1)
            scattered = field[idx]
        else:
            scattered = field

        coherence = (1.0 - a) ** 0.65
        x = coherence * field + (1.0 - coherence) * scattered * mask + transient_layer
        trem = 1.0 + (0.03 + 0.16 * a) * movement * np.sin(
            2.0 * np.pi * (0.18 + 2.7 * movement) * t
        )
        x *= trem
        return _normalise(_softclip(x, 0.08 + 0.22 * body))
