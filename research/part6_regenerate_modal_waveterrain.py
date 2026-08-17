#!/usr/bin/env python3
"""Regenerate Modal→WaveTerrain Part-6 path data using the frozen handoff law."""
from pathlib import Path
import argparse
import numpy as np

from part6_regenerate_additive_granular import (
    BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES,
    extract_features, estimate_jacobian,
)
from modal_waveterrain_reconstructed_renderer import ModalWaveTerrainRenderer


def sample(renderer, n: int, seed: int):
    t = np.linspace(0.0, 1.0, n)
    features, jacobians = [], []
    for a in t:
        features.append(extract_features(renderer.render(float(a), BASE_CONTROLS, seed)))
        jacobians.append(estimate_jacobian(renderer, float(a), BASE_CONTROLS, seed))
    return {
        "t": t,
        "sound_features": np.asarray(features),
        "jacobians": np.asarray(jacobians),
        "control_names": np.asarray(CONTROL_NAMES),
        "feature_names": np.asarray(FEATURE_NAMES),
    }


def reverse_exact(forward: dict[str, np.ndarray]):
    out = dict(forward)
    out["sound_features"] = forward["sound_features"][::-1].copy()
    out["jacobians"] = forward["jacobians"][::-1].copy()
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--samples", type=int, default=41)
    ap.add_argument("--seed", type=int, default=618033)
    ap.add_argument("--out-dir", type=Path, default=Path("research/data/part6"))
    args = ap.parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    renderer = ModalWaveTerrainRenderer()
    forward = sample(renderer, args.samples, args.seed)
    reverse = reverse_exact(forward)
    np.savez(args.out_dir / "modal_waveterrain_forward_FROZEN_LAW.npz", **forward)
    np.savez(args.out_dir / "waveterrain_modal_reverse_FROZEN_LAW.npz", **reverse)
    print("Wrote frozen-law Modal↔WaveTerrain forward/reverse paths.")


if __name__ == "__main__":
    main()
