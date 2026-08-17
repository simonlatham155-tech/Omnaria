#!/usr/bin/env python3
"""Corrected Part-6 Additive↔Granular regeneration entrypoint.

This version keeps the stochastic realization fixed across the entire path and
uses the same seed in both directions. That prevents path-point RNG changes
from masquerading as geometric transitions.
"""
from pathlib import Path
import argparse
import numpy as np

from part6_regenerate_additive_granular import (
    BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES,
    ReconstructedOmnariaRenderer, ReferenceRenderer,
    extract_features, estimate_jacobian, response_coupling,
)


def sample_path(renderer, n: int, reverse: bool, seed: int):
    axis = np.linspace(0.0, 1.0, n)
    alpha = axis[::-1] if reverse else axis
    features, jacobians, coupling = [], [], []
    pd = CONTROL_NAMES.index("partial_density")
    tc = CONTROL_NAMES.index("transient_complexity")

    # Critical Part-6 rule: keep one stochastic realization fixed across t.
    # Finite-difference perturbations and forward/reverse traversal therefore
    # compare geometry, not unrelated random grain clouds.
    for a in alpha:
        f = extract_features(renderer.render(float(a), BASE_CONTROLS, seed))
        j = estimate_jacobian(renderer, float(a), BASE_CONTROLS, seed)
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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--samples", type=int, default=41)
    ap.add_argument("--seed", type=int, default=618033)
    ap.add_argument("--out-dir", type=Path, default=Path("research/data/part6"))
    ap.add_argument("--renderer", choices=("reconstructed", "reference"), default="reconstructed")
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    renderer = ReconstructedOmnariaRenderer() if args.renderer == "reconstructed" else ReferenceRenderer()
    tag = "RECONSTRUCTED_RESEARCH_V2" if args.renderer == "reconstructed" else "REFERENCE_ONLY_V2"
    forward = sample_path(renderer, args.samples, False, args.seed)
    reverse = sample_path(renderer, args.samples, True, args.seed)
    np.savez(args.out_dir / f"additive_granular_forward_{tag}.npz", **forward)
    np.savez(args.out_dir / f"granular_additive_reverse_{tag}.npz", **reverse)
    print(f"Wrote {tag} forward/reverse paths with a fixed path seed.")


if __name__ == "__main__":
    main()
