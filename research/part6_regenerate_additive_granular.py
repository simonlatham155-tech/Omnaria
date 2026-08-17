#!/usr/bin/env python3
"""OMNARIA research — regenerate Additive→Granular path data for Part 6.

This file owns the *measurement pipeline*, not the synthesis claim.
It samples a renderer along a path, estimates finite-difference Jacobians in a
shared control space, extracts acoustic features, and writes the NPZ format
consumed by part6_chart_transition.py.

A renderer must implement:
    render(alpha: float, controls: np.ndarray, seed: int) -> np.ndarray
returning a mono floating-point waveform at SAMPLE_RATE.

The included ReferenceRenderer is deliberately marked NON-EVIDENCE. It exists
only to verify that regeneration, Jacobian estimation and bidirectional export
work end-to-end before the real OMNARIA renderer is connected.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

import numpy as np

SAMPLE_RATE = 48_000
DURATION_S = 1.0
N_SAMPLES = int(SAMPLE_RATE * DURATION_S)
EPS = 1e-12

CONTROL_NAMES = (
    "brightness",
    "body",
    "movement",
    "partial_density",
    "transient_complexity",
    "grain_size",
    "scatter",
    "harmonicity",
)

FEATURE_NAMES = (
    "rms",
    "spectral_centroid",
    "spectral_spread",
    "spectral_flatness",
    "high_band_ratio",
    "temporal_flux",
    "attack_ratio",
    "zero_crossing_rate",
    "partial_density_proxy",
    "transient_complexity_proxy",
)

BASE_CONTROLS = np.array([0.55, 0.55, 0.35, 0.50, 0.40, 0.45, 0.35, 0.75], dtype=float)


class Renderer(Protocol):
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray: ...


def _normalise_audio(x: np.ndarray) -> np.ndarray:
    x = np.asarray(x, dtype=float)
    x = x - np.mean(x)
    peak = np.max(np.abs(x)) + EPS
    return 0.8 * x / peak


def extract_features(x: np.ndarray) -> np.ndarray:
    x = np.asarray(x, dtype=float)
    window = np.hanning(len(x))
    mag = np.abs(np.fft.rfft(x * window)) + EPS
    power = mag * mag
    freqs = np.fft.rfftfreq(len(x), 1.0 / SAMPLE_RATE)
    psum = np.sum(power) + EPS

    centroid = np.sum(freqs * power) / psum
    spread = np.sqrt(np.sum(((freqs - centroid) ** 2) * power) / psum)
    flatness = np.exp(np.mean(np.log(mag))) / (np.mean(mag) + EPS)
    high = np.sum(power[freqs >= 5000.0]) / psum

    frame = 512
    hop = 256
    frames = []
    for start in range(0, max(1, len(x) - frame + 1), hop):
        seg = x[start:start + frame]
        if len(seg) < frame:
            seg = np.pad(seg, (0, frame - len(seg)))
        frames.append(np.abs(np.fft.rfft(seg * np.hanning(frame))))
    frames = np.asarray(frames)
    flux = float(np.mean(np.linalg.norm(np.diff(frames, axis=0), axis=1))) if len(frames) > 1 else 0.0

    env = np.abs(x)
    attack_n = max(1, int(0.05 * SAMPLE_RATE))
    attack_ratio = float(np.mean(env[:attack_n]) / (np.mean(env) + EPS))
    zcr = float(np.mean(np.signbit(x[:-1]) != np.signbit(x[1:])))

    # Peak-count proxy: number of locally dominant spectral peaks above a
    # relative floor, normalised to a practical range.
    m = mag[1:]
    peaks = (m[1:-1] > m[:-2]) & (m[1:-1] > m[2:]) & (m[1:-1] > 0.03 * np.max(m))
    partial_density = min(1.0, float(np.count_nonzero(peaks)) / 120.0)

    # Transient proxy: robust concentration of first-difference energy.
    d = np.abs(np.diff(x))
    transient_complexity = float(np.percentile(d, 95) / (np.mean(d) + EPS))

    return np.array([
        np.sqrt(np.mean(x * x)),
        centroid / (SAMPLE_RATE / 2),
        spread / (SAMPLE_RATE / 2),
        flatness,
        high,
        flux / (np.mean(frames) + EPS) / 100.0,
        attack_ratio,
        zcr,
        partial_density,
        transient_complexity / 10.0,
    ], dtype=float)


def estimate_jacobian(renderer: Renderer, alpha: float, controls: np.ndarray, seed: int, step: float = 0.015) -> np.ndarray:
    base_f = extract_features(renderer.render(alpha, controls, seed))
    j = np.zeros((len(base_f), len(controls)), dtype=float)
    for p in range(len(controls)):
        lo = controls.copy(); hi = controls.copy()
        lo[p] = max(0.0, lo[p] - step)
        hi[p] = min(1.0, hi[p] + step)
        denom = hi[p] - lo[p]
        if denom <= EPS:
            continue
        f_lo = extract_features(renderer.render(alpha, lo, seed))
        f_hi = extract_features(renderer.render(alpha, hi, seed))
        j[:, p] = (f_hi - f_lo) / denom
    return j


def response_coupling(j: np.ndarray, a: int, b: int) -> float:
    va = j[:, a]
    vb = j[:, b]
    na = np.linalg.norm(va); nb = np.linalg.norm(vb)
    if na <= EPS or nb <= EPS:
        return 0.0
    return float(np.dot(va, vb) / (na * nb))


@dataclass
class ReferenceRenderer:
    """NON-EVIDENCE deterministic reference renderer for pipeline validation.

    The two mechanisms are explicit textbook-style approximations. Results from
    this renderer must never be reported as evidence about OMNARIA's engine.
    """

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        t = np.arange(N_SAMPLES) / SAMPLE_RATE
        c = np.clip(np.asarray(controls, dtype=float), 0.0, 1.0)
        bright, body, movement, density, transient, grain, scatter, harmonicity = c
        f0 = 110.0

        # Additive side: harmonic bank with density and harmonicity controls.
        additive = np.zeros_like(t)
        n_partials = int(6 + 42 * density)
        for k in range(1, n_partials + 1):
            tilt = 0.7 + 2.3 * (1.0 - bright)
            amp = (k ** -tilt) * (0.6 + 0.4 * body)
            detune = (1.0 - harmonicity) * 0.012 * rng.normal()
            phase = rng.uniform(0, 2 * np.pi)
            additive += amp * np.sin(2 * np.pi * f0 * k * (1.0 + detune) * t + phase)
        additive *= 1.0 + 0.10 * movement * np.sin(2 * np.pi * (0.3 + 2.0 * movement) * t)

        # Granular side: deterministic grain cloud built from a harmonic source.
        source = np.sin(2 * np.pi * f0 * t) + 0.45 * np.sin(2 * np.pi * 2.01 * f0 * t)
        granular = np.zeros_like(t)
        grain_len = int((0.008 + 0.090 * grain) * SAMPLE_RATE)
        event_rate = 18 + int(150 * density)
        n_events = max(1, int(DURATION_S * event_rate))
        for _ in range(n_events):
            start = int(rng.integers(0, max(1, N_SAMPLES - grain_len)))
            out_start = int(rng.integers(0, max(1, N_SAMPLES - grain_len)))
            gl = min(grain_len, N_SAMPLES - start, N_SAMPLES - out_start)
            if gl < 4:
                continue
            win = np.hanning(gl)
            jitter = 1.0 + scatter * 0.15 * rng.normal()
            idx = np.clip((start + np.arange(gl) * jitter).astype(int), 0, N_SAMPLES - 1)
            burst = source[idx] * win
            if transient > 0:
                burst += transient * 0.08 * rng.normal(size=gl) * np.exp(-np.arange(gl) / max(8.0, gl * 0.12))
            granular[out_start:out_start + gl] += burst
        granular *= 0.5 + 0.7 * body

        # Smooth blend only; no planted transition law.
        a = np.clip(alpha, 0.0, 1.0)
        x = np.sqrt(max(0.0, 1.0 - a)) * additive + np.sqrt(a) * granular
        return _normalise_audio(x)


def sample_path(renderer: Renderer, n: int, reverse: bool, seed: int) -> dict[str, np.ndarray]:
    axis = np.linspace(0.0, 1.0, n)
    alpha = axis[::-1] if reverse else axis
    features = []
    jacobians = []
    coupling = []
    pd = CONTROL_NAMES.index("partial_density")
    tc = CONTROL_NAMES.index("transient_complexity")

    for i, a in enumerate(alpha):
        # Same deterministic seed for the base point and its finite differences
        # suppresses stochastic renderer variance in the Jacobian estimate.
        local_seed = seed + i
        x = renderer.render(float(a), BASE_CONTROLS, local_seed)
        f = extract_features(x)
        j = estimate_jacobian(renderer, float(a), BASE_CONTROLS, local_seed)
        features.append(f)
        jacobians.append(j)
        coupling.append(response_coupling(j, pd, tc))

    return {
        "t": axis,
        "sound_features": np.asarray(features),
        "jacobians": np.asarray(jacobians),
        "coupling": np.asarray(coupling),
        "alpha": np.asarray(alpha),
        "control_names": np.asarray(CONTROL_NAMES),
        "feature_names": np.asarray(FEATURE_NAMES),
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--samples", type=int, default=41)
    ap.add_argument("--seed", type=int, default=618033)
    ap.add_argument("--out-dir", type=Path, default=Path("research/data/part6"))
    ap.add_argument("--reference-only", action="store_true", help="run the NON-EVIDENCE reference renderer")
    args = ap.parse_args()

    if not args.reference_only:
        raise SystemExit(
            "No real OMNARIA renderer is connected yet. Provide a Renderer implementation, "
            "or pass --reference-only strictly to validate the measurement pipeline."
        )

    args.out_dir.mkdir(parents=True, exist_ok=True)
    renderer = ReferenceRenderer()
    forward = sample_path(renderer, args.samples, reverse=False, seed=args.seed)
    reverse = sample_path(renderer, args.samples, reverse=True, seed=args.seed + 100_000)

    np.savez(args.out_dir / "additive_granular_forward_REFERENCE_ONLY.npz", **forward)
    np.savez(args.out_dir / "granular_additive_reverse_REFERENCE_ONLY.npz", **reverse)
    print("Wrote reference-only forward/reverse paths; these are pipeline tests, not OMNARIA evidence.")


if __name__ == "__main__":
    main()
