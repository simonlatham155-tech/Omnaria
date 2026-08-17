#!/usr/bin/env python3
"""Preregistered Part-6 robustness stress test.

Uses existing CrossFamilyRenderer endpoint mechanisms unchanged. Perturbations are
applied externally so renderer timing and frozen predictor remain untouched.
"""
from pathlib import Path
import argparse, json
import numpy as np

from part6_cross_family_renderer import CrossFamilyRenderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian

PAIRS=["additive_modal","granular_fm","waveterrain_nonlinearchaos"]
ALT=np.array([0.31,0.72,0.63,0.41,0.76,0.27,0.58,0.84],float)
VARIANTS={
 "seed_271828":dict(seed=271828,pitch=1.0,duration=1.0,controls="base"),
 "pitch_up":dict(seed=618033,pitch=1.189207115,duration=1.0,controls="base"),
 "short_window":dict(seed=618033,pitch=1.0,duration=0.65,controls="base"),
 "alternate_controls":dict(seed=618033,pitch=1.0,duration=1.0,controls="alternate"),
}

def transform_audio(x,pitch,duration):
    x=np.asarray(x,float)
    n=len(x)
    if abs(pitch-1.0)>1e-12:
        src=np.arange(n,dtype=float)*pitch
        y=np.interp(src,np.arange(n,dtype=float),x,left=0.0,right=0.0)
    else:
        y=x.copy()
    if duration < 0.999999:
        m=max(8,int(round(n*duration)))
        y=y[:m]
        # Preserve feature extractor assumptions by padding with endpoint silence.
        y=np.pad(y,(0,n-m))
    return y

class PerturbedRenderer:
    def __init__(self,pair,pitch,duration):
        self.base=CrossFamilyRenderer(pair); self.pitch=pitch; self.duration=duration
    def render(self,alpha,controls,seed):
        return transform_audio(self.base.render(alpha,controls,seed),self.pitch,self.duration)

def sample(renderer,n,seed,controls):
    t=np.linspace(0.0,1.0,n); feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(renderer.render(float(a),controls,seed)))
        jacs.append(estimate_jacobian(renderer,float(a),controls,seed))
    return {"t":t,"sound_features":np.asarray(feats),"jacobians":np.asarray(jacs),"control_names":np.asarray(CONTROL_NAMES),"feature_names":np.asarray(FEATURE_NAMES)}

def reverse_exact(f):
    return {"t":f["t"].copy(),"sound_features":f["sound_features"][::-1].copy(),"jacobians":f["jacobians"][::-1].copy(),"control_names":f["control_names"].copy(),"feature_names":f["feature_names"].copy()}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument("--samples",type=int,default=41); ap.add_argument("--out-dir",type=Path,default=Path("research/data/part6_robustness")); args=ap.parse_args(); args.out_dir.mkdir(parents=True,exist_ok=True)
    manifest={}
    for vname,v in VARIANTS.items():
        controls=BASE_CONTROLS if v["controls"]=="base" else ALT
        for pair in PAIRS:
            key=f"{pair}__{vname}"; r=PerturbedRenderer(pair,v["pitch"],v["duration"])
            f=sample(r,args.samples,v["seed"],controls); rev=reverse_exact(f)
            np.savez(args.out_dir/f"{key}_forward.npz",**f); np.savez(args.out_dir/f"{key}_reverse.npz",**rev)
            manifest[key]={"pair":pair,"variant":vname,**v}; print("Wrote",key)
    (args.out_dir/"manifest.json").write_text(json.dumps(manifest,indent=2))
if __name__=="__main__": main()
