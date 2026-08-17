#!/usr/bin/env python3
from pathlib import Path
import argparse
import numpy as np
from part7_external_baseline_renderer import ExternalBaselineRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian

MODES=['independent_sigmoid_additive_fm','independent_sigmoid_modal_noise','single_engine_continuous_fm','single_engine_continuous_additive']

def sample(renderer,n,seed):
    t=np.linspace(0.0,1.0,n); feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(renderer.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(renderer,float(a),BASE_CONTROLS,seed))
    return {'t':t,'sound_features':np.asarray(feats),'jacobians':np.asarray(jacs),'control_names':np.asarray(CONTROL_NAMES),'feature_names':np.asarray(FEATURE_NAMES)}

def reverse_exact(f):
    return {'t':f['t'].copy(),'sound_features':f['sound_features'][::-1].copy(),'jacobians':f['jacobians'][::-1].copy(),'control_names':f['control_names'].copy(),'feature_names':f['feature_names'].copy()}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--samples',type=int,default=41); ap.add_argument('--seed',type=int,default=1618033); ap.add_argument('--out-dir',type=Path,default=Path('research/data/part7_external'))
    args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True)
    for mode in MODES:
        f=sample(ExternalBaselineRenderer(mode),args.samples,args.seed); r=reverse_exact(f)
        np.savez(args.out_dir/f'{mode}_forward.npz',**f); np.savez(args.out_dir/f'{mode}_reverse.npz',**r)
        print('Wrote',mode)

if __name__=='__main__': main()
