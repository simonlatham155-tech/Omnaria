#!/usr/bin/env python3
"""Regenerate preregistered three-pair Part-6 prospective batch."""
from pathlib import Path
import argparse
import numpy as np

from part6_cross_family_renderer import CrossFamilyRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian

PAIRS=["additive_modal","granular_fm","waveterrain_nonlinearchaos"]


def sample(renderer,n,seed):
    t=np.linspace(0.0,1.0,n)
    feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(renderer.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(renderer,float(a),BASE_CONTROLS,seed))
    return {
        "t":t,
        "sound_features":np.asarray(feats),
        "jacobians":np.asarray(jacs),
        "control_names":np.asarray(CONTROL_NAMES),
        "feature_names":np.asarray(FEATURE_NAMES),
    }


def reverse_exact(forward):
    return {
        "t":forward["t"].copy(),
        "sound_features":forward["sound_features"][::-1].copy(),
        "jacobians":forward["jacobians"][::-1].copy(),
        "control_names":forward["control_names"].copy(),
        "feature_names":forward["feature_names"].copy(),
    }


def main():
    ap=argparse.ArgumentParser(); ap.add_argument("--samples",type=int,default=41); ap.add_argument("--seed",type=int,default=618033); ap.add_argument("--out-dir",type=Path,default=Path("research/data/part6_batch2"))
    args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True)
    for pair in PAIRS:
        r=CrossFamilyRenderer(pair)
        f=sample(r,args.samples,args.seed); rev=reverse_exact(f)
        np.savez(args.out_dir/f"{pair}_forward_PROSPECTIVE.npz",**f)
        np.savez(args.out_dir/f"{pair}_reverse_PROSPECTIVE.npz",**rev)
        print(f"Wrote {pair}")

if __name__=="__main__": main()
