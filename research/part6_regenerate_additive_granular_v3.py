#!/usr/bin/env python3
"""Part-6 Additive↔Granular regeneration using reconstructed renderer v2.

The stochastic realization is fixed for the full path. The reverse traversal is
constructed by exact array reversal of the forward traversal, which is the
correct bidirectional control for a deterministic fixed-seed path and avoids
re-rendering another random realization.
"""
from pathlib import Path
import argparse
import numpy as np

from omnaria_reconstructed_renderer_v2 import ReconstructedOmnariaRendererV2
from part6_regenerate_additive_granular import (
    BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES,
    extract_features, estimate_jacobian, response_coupling,
)


def sample_forward(renderer, n: int, seed: int):
    axis = np.linspace(0.0, 1.0, n)
    features, jacobians, coupling = [], [], []
    pd = CONTROL_NAMES.index("partial_density")
    tc = CONTROL_NAMES.index("transient_complexity")
    for a in axis:
        f = extract_features(renderer.render(float(a), BASE_CONTROLS, seed))
        j = estimate_jacobian(renderer, float(a), BASE_CONTROLS, seed)
        features.append(f)
        jacobians.append(j)
        coupling.append(response_coupling(j, pd, tc))
    return {
        "t": axis,
        "alpha": axis.copy(),
        "sound_features": np.asarray(features),
        "jacobians": np.asarray(jacobians),
        "coupling": np.asarray(coupling),
        "control_names": np.asarray(CONTROL_NAMES),
        "feature_names": np.asarray(FEATURE_NAMES),
    }


def exact_reverse(forward):
    n = len(forward["t"])
    return {
        "t": np.linspace(0.0, 1.0, n),
        "alpha": forward["alpha"][::-1].copy(),
        "sound_features": forward["sound_features"][::-1].copy(),
        "jacobians": forward["jacobians"][::-1].copy(),
        "coupling": forward["coupling"][::-1].copy(),
        "control_names": forward["control_names"].copy(),
        "feature_names": forward["feature_names"].copy(),
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--samples", type=int, default=41)
    ap.add_argument("--seed", type=int, default=618033)
    ap.add_argument("--out-dir", type=Path, default=Path("research/data/part6"))
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    renderer = ReconstructedOmnariaRendererV2()
    forward = sample_forward(renderer, args.samples, args.seed)
    reverse = exact_reverse(forward)

    np.savez(args.out_dir / "additive_granular_forward_RECONSTRUCTED_RESEARCH_V3.npz", **forward)
    np.savez(args.out_dir / "granular_additive_reverse_RECONSTRUCTED_RESEARCH_V3.npz", **reverse)
    print("Wrote V3 fixed-seed forward path and exact reverse path.")
    print(f"Granular endpoint coupling: {forward['coupling'][-1]:.6f}")


if __name__ == "__main__":
    main()
