#!/usr/bin/env python3
"""Regenerate prospective Wavetable→Physical Part-6 path data."""
from pathlib import Path
import argparse
import numpy as np
from wavetable_physical_reconstructed_renderer import WavetablePhysicalRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian


def sample(renderer,n,seed,reverse=False):
    axis=np.linspace(0.0,1.0,n)
    alpha=axis[::-1] if reverse else axis
    feats=[]; jacs=[]
    for a in alpha:
        feats.append(extract_features(renderer.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(renderer,float(a),BASE_CONTROLS,seed))
    return {
        't':axis,
        'alpha':np.asarray(alpha),
        'sound_features':np.asarray(feats),
        'jacobians':np.asarray(jacs),
        'control_names':np.asarray(CONTROL_NAMES),
        'feature_names':np.asarray(FEATURE_NAMES),
    }


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--samples',type=int,default=41)
    ap.add_argument('--seed',type=int,default=618033)
    ap.add_argument('--out-dir',type=Path,default=Path('research/data/part6'))
    args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True)
    r=WavetablePhysicalRenderer()
    np.savez(args.out_dir/'wavetable_physical_forward_PROSPECTIVE.npz',**sample(r,args.samples,args.seed,False))
    np.savez(args.out_dir/'physical_wavetable_reverse_PROSPECTIVE.npz',**sample(r,args.samples,args.seed,True))
    print('Wrote prospective Wavetable↔Physical paths.')

if __name__=='__main__': main()
