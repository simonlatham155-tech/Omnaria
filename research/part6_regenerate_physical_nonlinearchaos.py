#!/usr/bin/env python3
"""Regenerate Physical→NonlinearChaos Part-6 path with frozen handoff law."""
from pathlib import Path
import argparse
import numpy as np
from physical_nonlinearchaos_reconstructed_renderer import PhysicalNonlinearChaosRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian, response_coupling


def sample(renderer,n,seed,reverse=False):
    axis=np.linspace(0.0,1.0,n); alpha=axis[::-1] if reverse else axis
    feats=[]; jacs=[]; coupling=[]
    pd=CONTROL_NAMES.index('partial_density'); tc=CONTROL_NAMES.index('transient_complexity')
    for a in alpha:
        f=extract_features(renderer.render(float(a),BASE_CONTROLS,seed))
        j=estimate_jacobian(renderer,float(a),BASE_CONTROLS,seed)
        feats.append(f); jacs.append(j); coupling.append(response_coupling(j,pd,tc))
    return {'t':axis,'sound_features':np.asarray(feats),'jacobians':np.asarray(jacs),'coupling':np.asarray(coupling),'alpha':np.asarray(alpha),'control_names':np.asarray(CONTROL_NAMES),'feature_names':np.asarray(FEATURE_NAMES)}


def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--samples',type=int,default=41); ap.add_argument('--seed',type=int,default=618033); ap.add_argument('--out-dir',type=Path,default=Path('research/data/part6'))
    args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True); r=PhysicalNonlinearChaosRenderer()
    np.savez(args.out_dir/'physical_nonlinearchaos_forward_FROZEN_LAW.npz',**sample(r,args.samples,args.seed,False))
    np.savez(args.out_dir/'nonlinearchaos_physical_reverse_FROZEN_LAW.npz',**sample(r,args.samples,args.seed,True))
    print('Wrote frozen-law Physical↔NonlinearChaos paths.')
if __name__=='__main__': main()
