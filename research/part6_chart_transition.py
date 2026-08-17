#!/usr/bin/env python3
"""OMNARIA research — Part 6: chart-transition discovery.

Consumes sampled synthesis paths containing:
  t              shape (N,)
  sound_features shape (N, F)
  jacobians      shape (N, F, P)
Optional:
  coupling       shape (N,) e.g. PartialDensity↔TransientComplexity

The script measures whether local generative geometry reorganises while the
acoustic trajectory remains continuous.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass, asdict
from pathlib import Path

import numpy as np

EPS = 1e-12


@dataclass
class TransitionPoint:
    index: int
    t: float
    segment_midpoint: float
    sound_step: float
    metric_step: float
    eigenspace_rotation_deg: float
    condition_number: float
    score: float
    coupling: float | None = None


def pullback_metric(j: np.ndarray, weights: np.ndarray | None = None) -> np.ndarray:
    if weights is None:
        return j.T @ j
    w = np.asarray(weights, dtype=float)
    if w.ndim != 1 or w.shape[0] != j.shape[0]:
        raise ValueError("weights must have shape (feature_count,)")
    return j.T @ (w[:, None] * j)


def stable_condition_number(g: np.ndarray, relative_floor: float = 1e-8) -> float:
    vals = np.clip(np.linalg.eigvalsh(g), 0.0, None)
    vmax = float(vals[-1]) if vals.size else 0.0
    if vmax <= EPS:
        return float("inf")
    positive = vals[vals > vmax * relative_floor]
    if positive.size == 0:
        return float("inf")
    return vmax / float(positive[0])


def dominant_basis(g: np.ndarray, energy: float = 0.9, max_rank: int | None = None) -> np.ndarray:
    vals, vecs = np.linalg.eigh(g)
    order = np.argsort(vals)[::-1]
    vals = np.clip(vals[order], 0.0, None)
    vecs = vecs[:, order]
    total = float(vals.sum())
    if total <= EPS:
        return vecs[:, :1]
    rank = int(np.searchsorted(np.cumsum(vals) / total, energy) + 1)
    if max_rank is not None:
        rank = min(rank, max_rank)
    return vecs[:, :max(1, rank)]


def eigenspace_rotation_deg(g0: np.ndarray, g1: np.ndarray) -> float:
    b0 = dominant_basis(g0)
    b1 = dominant_basis(g1)
    rank = min(b0.shape[1], b1.shape[1])
    s = np.linalg.svd(b0[:, :rank].T @ b1[:, :rank], compute_uv=False)
    s = np.clip(s, -1.0, 1.0)
    angles = np.degrees(np.arccos(s))
    return float(np.max(angles)) if angles.size else 0.0


def robust_z(x: np.ndarray) -> np.ndarray:
    x = np.asarray(x, dtype=float)
    finite = np.isfinite(x)
    out = np.zeros_like(x)
    if not finite.any():
        return out
    vals = x[finite]
    med = np.median(vals)
    mad_scale = 1.4826 * np.median(np.abs(vals - med))
    std_scale = np.std(vals)
    scale = max(float(mad_scale), float(std_scale) * 0.10, EPS)
    out[finite] = (vals - med) / scale
    return out


def analyse_path(t, sound_features, jacobians, coupling=None, weights=None):
    t = np.asarray(t, dtype=float)
    x = np.asarray(sound_features, dtype=float)
    js = np.asarray(jacobians, dtype=float)
    if t.ndim != 1 or x.ndim != 2 or js.ndim != 3:
        raise ValueError("expected t:(N,), sound_features:(N,F), jacobians:(N,F,P)")
    if not (len(t) == len(x) == len(js)) or len(t) < 3:
        raise ValueError("path arrays must have equal N >= 3")
    if x.shape[1] != js.shape[1]:
        raise ValueError("feature dimension mismatch")

    metrics = np.stack([pullback_metric(j, weights) for j in js])
    ds = np.linalg.norm(np.diff(x, axis=0), axis=1)
    dg = np.linalg.norm(np.diff(metrics, axis=0), axis=(1, 2))
    rot = np.array([eigenspace_rotation_deg(metrics[i], metrics[i + 1]) for i in range(len(t) - 1)])
    cond = np.array([stable_condition_number(g) for g in metrics[1:]])
    score = robust_z(dg) + robust_z(rot) + robust_z(np.log10(np.maximum(cond, 1.0))) - robust_z(ds)

    points = []
    for i in range(len(score)):
        points.append(TransitionPoint(
            index=i + 1,
            t=float(t[i + 1]),
            segment_midpoint=float(0.5 * (t[i] + t[i + 1])),
            sound_step=float(ds[i]),
            metric_step=float(dg[i]),
            eigenspace_rotation_deg=float(rot[i]),
            condition_number=float(cond[i]),
            score=float(score[i]),
            coupling=None if coupling is None else float(coupling[i + 1]),
        ))

    best = max(points, key=lambda p: p.score)
    summary = {
        "n_samples": len(t),
        "candidate_t": best.t,
        "candidate_segment_midpoint": best.segment_midpoint,
        "candidate_index": best.index,
        "candidate_score": best.score,
        "candidate_sound_step": best.sound_step,
        "candidate_sound_ratio_to_median": best.sound_step / (float(np.median(ds)) + EPS),
        "candidate_metric_step": best.metric_step,
        "candidate_rotation_deg": best.eigenspace_rotation_deg,
        "candidate_condition_number": best.condition_number,
        "candidate_coupling": best.coupling,
        "median_sound_step": float(np.median(ds)),
        "median_metric_step": float(np.median(dg)),
        "median_rotation_deg": float(np.median(rot)),
    }
    return points, summary


def load_npz(path: Path):
    z = np.load(path)
    required = {"t", "sound_features", "jacobians"}
    missing = required - set(z.files)
    if missing:
        raise ValueError(f"{path}: missing arrays {sorted(missing)}")
    return z["t"], z["sound_features"], z["jacobians"], z["coupling"] if "coupling" in z.files else None


def compare_directions(forward: dict, reverse: dict) -> dict:
    # A transition belongs to the interval between samples, not to its arrival
    # endpoint. Compare interval midpoints to avoid a one-sample directional bias.
    f_mid = float(forward["candidate_segment_midpoint"])
    r_mid_mapped = 1.0 - float(reverse["candidate_segment_midpoint"])
    delta = abs(f_mid - r_mid_mapped)
    return {
        "forward_transition_midpoint": f_mid,
        "reverse_transition_midpoint_mapped": r_mid_mapped,
        "transition_location_delta": delta,
        "bidirectional_reproduction": bool(delta <= 0.10),
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("forward", type=Path)
    ap.add_argument("--reverse", type=Path)
    ap.add_argument("--weights", type=Path)
    ap.add_argument("--out", type=Path, default=Path("part6_result.json"))
    args = ap.parse_args()

    weights = np.load(args.weights) if args.weights else None
    ft, fx, fj, fc = load_npz(args.forward)
    fpoints, fsummary = analyse_path(ft, fx, fj, fc, weights)
    result = {"forward": fsummary, "forward_trace": [asdict(p) for p in fpoints]}

    if args.reverse:
        rt, rx, rj, rc = load_npz(args.reverse)
        rpoints, rsummary = analyse_path(rt, rx, rj, rc, weights)
        result["reverse"] = rsummary
        result["reverse_trace"] = [asdict(p) for p in rpoints]
        result["bidirectional"] = compare_directions(fsummary, rsummary)

    args.out.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({k: v for k, v in result.items() if not k.endswith("_trace")}, indent=2))


if __name__ == "__main__":
    main()
