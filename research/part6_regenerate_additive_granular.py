#!/usr/bin/env python3
"""OMNARIA research — regenerate Additive→Granular path data for Part 6.

This file owns the measurement pipeline. It samples a renderer along a path,
estimates finite-difference Jacobians in a shared control space, extracts
acoustic features, and writes the NPZ format consumed by
part6_chart_transition.py.

Renderer modes:
  reconstructed  current continuous OMNARIA research hypothesis
  reference      NON-EVIDENCE pipeline validation model

Neither mode is recovered production code. The reconstructed mode is evidence
about the explicit research model committed beside this file, not proof about a
previous unpublished OMNARIA implementation.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

import numpy as np

from omnaria_reconstructed_renderer import ReconstructedOmnariaRenderer

SAMPLE_RATE = 48_000
DURATION_S = 1.0
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

    m = mag[1:]
    peaks = (m[1:-1] > m[:-2]) & (m[1:-1] > m[2:]) & (m[1:-1] > 0.03 * np.max(m))
    partial_density = min(1.0, float(np.count_nonzero(peaks)) / 120.0)

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
    """NON-EVIDENCE deterministic reference renderer for pipeline validation."""
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng = np.random.default_rng(seed)
        n = int(SAMPLE_RATE * DURATION_S)
        t = np.arange(n) / SAMPLE_RATE
        c = np.clip(np.asarray(controls, dtype=float), 0.0, 1.0)
        bright, body, movement, density, transient, grain, scatter, harmonicity = c
        f0 = 110.0
        additive = np.zeros_like(t)
        n_partials = int(6 + 42 * density)
        for k in range(1, n_partials + 1):
            tilt = 0.7 + 2.3 * (1.0 - bright)
            amp = (k ** -tilt) * (0.6 + 0.4 * body)
            detune = (1.0 - harmonicity) * 0.012 * rng.normal()
            additive += amp * np.sin(2 * np.pi * f0 * k * (1.0 + detune) * t + rng.uniform(0, 2*np.pi))
        source = np.sin(2*np.pi*f0*t) + 0.45*np.sin(2*np.pi*2.01*f0*t)
        granular = np.zeros_like(t)
        grain_len = int((0.008 + 0.090 * grain) * SAMPLE_RATE)
        event_rate = 18 + int(150 * density)
        for _ in range(max(1, int(event_rate * DURATION_S))):
            start = int(rng.integers(0, max(1, n-grain_len)))
            out_start = int(rng.integers(0, max(1, n-grain_len)))
            gl = min(grain_len, n-start, n-out_start)
            if gl < 4: continue
            win = np.hanning(gl)
            idx = np.clip((start + np.arange(gl)*(1.0 + scatter*0.15*rng.normal())).astype(int), 0, n-1)
            burst = source[idx]*win
            burst += transient*0.08*rng.normal(size=gl)*np.exp(-np.arange(gl)/max(8.0, gl*0.12))
            granular[out_start:out_start+gl] += burst
        a = np.clip(alpha, 0.0, 1.0)
        x = np.sqrt(max(0.0,1.0-a))*additive + np.sqrt(a)*granular
        x -= np.mean(x)
        return 0.8*x/(np.max(np.abs(x))+EPS)


def sample_path(renderer: Renderer, n: int, reverse: bool, seed: int) -> dict[str, np.ndarray]:
    axis = np.linspace(0.0, 1.0, n)
    alpha = axis[::-1] if reverse else axis
    features, jacobians, coupling = [], [], []
    pd = CONTROL_NAMES.index("partial_density")
    tc = CONTROL_NAMES.index("transient_complexity")
    for i, a in enumerate(alpha):
        local_seed = seed + i
        f = extract_features(renderer.render(float(a), BASE_CONTROLS, local_seed))
        j = estimate_jacobian(renderer, float(a), BASE_CONTROLS, local_seed)
        features.append(f); jacobians.append(j); coupling.append(response_coupling(j, pd, tc))
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
    ap.add_argument("--renderer", choices=("reconstructed", "reference"), default="reconstructed")
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    renderer: Renderer = ReconstructedOmnariaRenderer() if args.renderer == "reconstructed" else ReferenceRenderer()
    tag = "RECONSTRUCTED_RESEARCH" if args.renderer == "reconstructed" else "REFERENCE_ONLY"
    forward = sample_path(renderer, args.samples, reverse=False, seed=args.seed)
    reverse = sample_path(renderer, args.samples, reverse=True, seed=args.seed + 100_000)
    np.savez(args.out_dir / f"additive_granular_forward_{tag}.npz", **forward)
    np.savez(args.out_dir / f"granular_additive_reverse_{tag}.npz", **reverse)
    print(f"Wrote {tag} forward/reverse paths.")


if __name__ == "__main__":
    main()
