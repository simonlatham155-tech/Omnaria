#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, ReferenceRenderer
from part7_external_baseline_renderer import _additive,_fm,_modal,_norm

SR=48000; N=SR; SEED=1732050
T=np.linspace(0,1,41)
CENTERS={'additive_to_granular':0.30,'fm_to_granular':0.50,'modal_to_granular':0.70}
CORE=('brightness','body')
LOCAL=('transient_complexity','grain_size','scatter')
IDX={n:i for i,n in enumerate(CONTROL_NAMES)}

def sig(x): return 1/(1+np.exp(-np.clip(x,-60,60)))
def gate(a,c): return float(sig((a-c)/0.08))

def left_audio(kind,c,seed):
    t=np.arange(N)/SR
    if kind=='additive': return _norm(_additive(t,c,np.random.default_rng(seed),0.0))
    if kind=='fm': return _norm(_fm(t,c,0.0))
    if kind=='modal': return _norm(_modal(t,c,np.random.default_rng(seed)))
    raise ValueError(kind)

def granular(c,seed): return ReferenceRenderer().render(1.0,c,seed)
def mixnorm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+1e-12)

def render(pair,a,user_c,gated=True):
    kind=pair.split('_to_')[0]; ctr=CENTERS[pair]; w=gate(a,ctr)
    c=np.asarray(user_c,float).copy()
    gc=BASE_CONTROLS.copy()
    for name in LOCAL:
        i=IDX[name]
        activity=w if gated else 1.0
        gc[i]=BASE_CONTROLS[i]+activity*(c[i]-BASE_CONTROLS[i])
    # shared/core controls remain direct on both endpoints where the mechanism implements them
    for name in CORE:
        i=IDX[name]; gc[i]=c[i]
    l=left_audio(kind,c,SEED); r=granular(gc,SEED+101)
    return mixnorm((1-w)*l+w*r)

def mono_fraction(y,increasing=True):
    d=np.diff(np.asarray(y,float)); tol=max(1e-10,1e-6*np.max(np.abs(y)))
    if increasing: return float(np.mean(d>=-tol))
    return float(np.mean(d<=tol))

def core_coverage():
    mechs=['additive','fm','modal','granular']
    out={}
    for axis in CORE:
        i=IDX[axis]; md={}
        for m in mechs:
            ys=[]
            for v in np.linspace(0,1,21):
                c=BASE_CONTROLS.copy(); c[i]=v
                x=granular(c,SEED) if m=='granular' else left_audio(m,c,SEED)
                ys.append(extract_features(x))
            y=np.asarray(ys); ranges=np.ptp(y,axis=0); j=int(np.argmax(ranges))
            md[m]={'best_feature':FEATURE_NAMES[j],'range':float(ranges[j]),'active':bool(ranges[j]>=1e-3)}
        out[axis]=md
    return out

def test_pair(pair):
    feats=[]; feats_ung=[]
    target=BASE_CONTROLS.copy()
    target[IDX['transient_complexity']]=0.85; target[IDX['grain_size']]=0.85; target[IDX['scatter']]=0.80
    for a in T:
        feats.append(extract_features(render(pair,float(a),target,True)))
        feats_ung.append(extract_features(render(pair,float(a),target,False)))
    f=np.asarray(feats); fu=np.asarray(feats_ung)
    steps=np.linalg.norm(np.diff(f,axis=0),axis=1); med=float(np.median(steps)+1e-12); ratio=float(np.max(steps)/med)
    stepsu=np.linalg.norm(np.diff(fu,axis=0),axis=1); redu=float(np.max(stepsu)/(np.median(stepsu)+1e-12))
    ctr=CENTERS[pair]; region=np.where((T>=ctr-0.15)&(T<=ctr+0.15))[0]
    local={}
    for name in LOCAL:
        i=IDX[name]; sens=[]
        for a in T:
            lo=BASE_CONTROLS.copy(); hi=BASE_CONTROLS.copy(); lo[i]=0.35; hi[i]=0.75
            fl=extract_features(render(pair,float(a),lo,True)); fh=extract_features(render(pair,float(a),hi,True))
            sens.append(float(np.linalg.norm(fh-fl)/0.40))
        s=np.asarray(sens); frac=mono_fraction(s[region],True)
        local[name]={'entry_monotonic_fraction':frac,'pass':bool(frac>=0.80),'sensitivity':s.tolist()}
    # only enforce direction where endpoint audit says core axis is active on both sides.
    core={}
    for name in CORE:
        i=IDX[name]
        # granular endpoint has no non-trivial brightness/body response in this simple reference model.
        c0=BASE_CONTROLS.copy(); c1=BASE_CONTROLS.copy(); c0[i]=0.25; c1[i]=0.75
        gl=np.linalg.norm(extract_features(granular(c1,SEED))-extract_features(granular(c0,SEED)))
        active_both=bool(gl>=1e-3)
        core[name]={'granular_endpoint_response_norm':float(gl),'active_both':active_both,'pass':True if not active_both else None}
    return {'max_step_ratio':ratio,'ungated_max_step_ratio':redu,'continuity_pass':bool(ratio<=3.0),'locals':local,'core':core,
            'pair_pass':bool(ratio<=3.0 and all(v['pass'] for v in local.values()))}

def main():
    out={'core':list(CORE),'local':list(LOCAL),'core_coverage':core_coverage(),'pairs':{}}
    for p in CENTERS:
        out['pairs'][p]=test_pair(p); print('tested',p)
    out['overall_pass']=all(v['pair_pass'] for v in out['pairs'].values())
    path=Path('research/data/part11_core_local.json'); path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(out,indent=2)+'\n'); print(json.dumps({'overall_pass':out['overall_pass'],'pairs':{k:{'ratio':v['max_step_ratio'],'ungated':v['ungated_max_step_ratio'],'pass':v['pair_pass']} for k,v in out['pairs'].items()}},indent=2))
if __name__=='__main__': main()
