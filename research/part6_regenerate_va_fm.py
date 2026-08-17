#!/usr/bin/env python3
"""Regenerate VA→FM Part-6 path for prospective frozen-rule testing."""
from pathlib import Path
import argparse
import numpy as np
from va_fm_reconstructed_renderer import VAFMRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian


def sample(renderer,n,seed):
    t=np.linspace(0.0,1.0,n)
    feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(renderer.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(renderer,float(a),BASE_CONTROLS,seed))
    return {'t':t,'sound_features':np.asarray(feats),'jacobians':np.asarray(jacs),'alpha':t.copy(),'control_names':np.asarray(CONTROL_NAMES),'feature_names':np.asarray(FEATURE_NAMES)}


def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--samples',type=int,default=41); ap.add_argument('--seed',type=int,default=618033); ap.add_argument('--out-dir',type=Path,default=Path('research/data/part6'))
    args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True)
    f=sample(VAFMRenderer(),args.samples,args.seed)
    r={k:(v[::-1].copy() if k in {'sound_features','jacobians','alpha'} else v.copy() if hasattr(v,'copy') else v) for k,v in f.items()}
    r['t']=np.linspace(0.0,1.0,args.samples)
    np.savez(args.out_dir/'va_fm_forward_PROSPECTIVE.npz',**f)
    np.savez(args.out_dir/'fm_va_reverse_PROSPECTIVE.npz',**r)
    print('Wrote prospective VA↔FM paths.')
if __name__=='__main__': main()
