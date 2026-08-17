#!/usr/bin/env python3
"""Part 10 — audit shared macro coordinates across real audio mechanisms.

No calibration is fitted here. This pass only measures the acoustic response of
all eight shared controls across six existing synthesis mechanisms.
"""
from __future__ import annotations
import json
from pathlib import Path
import numpy as np

from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, ReferenceRenderer
from part7_external_baseline_renderer import _additive, _fm, _modal, _noise, _norm
from part8_persistence_renderer import waveshape, filtered_noise, norm

SR=48_000
N=SR
SEED=3141592
GRID=np.linspace(0.0,1.0,21)


def render_mechanism(name: str, controls: np.ndarray, seed: int) -> np.ndarray:
    c=np.clip(np.asarray(controls,float),0,1)
    t=np.arange(N)/SR
    if name=='additive':
        return _norm(_additive(t,c,np.random.default_rng(seed),0.0))
    if name=='fm':
        return _norm(_fm(t,c,0.0))
    if name=='modal':
        return _norm(_modal(t,c,np.random.default_rng(seed)))
    if name=='waveshape':
        return norm(waveshape(t,c,0.0))
    if name=='filtered_noise':
        return norm(filtered_noise(t,c,0.0,seed))
    if name=='granular':
        # Reuse the committed deterministic granular reference endpoint only.
        return ReferenceRenderer().render(1.0,c,seed)
    raise ValueError(name)


def corr(x,y):
    x=np.asarray(x,float); y=np.asarray(y,float)
    if np.std(y)<1e-12: return 0.0
    return float(np.corrcoef(x,y)[0,1])


def monotonic_fraction(y):
    d=np.diff(np.asarray(y,float))
    nz=d[np.abs(d)>1e-12]
    if len(nz)==0: return 0.0
    return float(max(np.mean(nz>0),np.mean(nz<0)))


def main():
    mechanisms=['additive','fm','modal','waveshape','filtered_noise','granular']
    out={'grid':GRID.tolist(),'mechanisms':{},'control_names':list(CONTROL_NAMES),'feature_names':list(FEATURE_NAMES)}
    for mech in mechanisms:
        md={}
        for ci,cname in enumerate(CONTROL_NAMES):
            feats=[]
            for v in GRID:
                c=BASE_CONTROLS.copy(); c[ci]=v
                feats.append(extract_features(render_mechanism(mech,c,SEED)))
            f=np.asarray(feats)
            correlations=[corr(GRID,f[:,j]) for j in range(f.shape[1])]
            j=int(np.argmax(np.abs(correlations)))
            ranges=np.ptp(f,axis=0)
            md[cname]={
                'best_feature':FEATURE_NAMES[j],
                'best_corr':correlations[j],
                'best_monotonic_fraction':monotonic_fraction(f[:,j]),
                'best_feature_range':float(ranges[j]),
                'feature_correlations':{FEATURE_NAMES[k]:float(correlations[k]) for k in range(len(FEATURE_NAMES))},
                'feature_ranges':{FEATURE_NAMES[k]:float(ranges[k]) for k in range(len(FEATURE_NAMES))},
                'features':f.tolist(),
            }
        out['mechanisms'][mech]=md
        print('Audited',mech)
    p=Path('research/data/part10_coordinate_audit.json'); p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(out,indent=2)+'\n')
    print('Wrote',p)

if __name__=='__main__': main()
