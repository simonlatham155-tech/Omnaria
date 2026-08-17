#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np
from part12_richer_families import FAMILIES, BASE, norm
from part6_regenerate_additive_granular import extract_features

OUT=Path('research/data/part14'); OUT.mkdir(parents=True,exist_ok=True)
FAMS=['granular','physical','spectral','chaos']
STEP=0.025
SEEDS=[2718281,1414213]
ALPHAS=np.linspace(0,1,41)
Z0=np.array([0.0,0.0,0.0,0.0])

# Frozen smooth global->native mappings. Rows are native parameters, cols are global z directions.
# Shared first two directions drive all families; local directions are attenuated by family weight.
MAP={
'granular':np.array([[ .10,.00,.04,.00],[ .00,.12,.00,.06],[ .03,.00,.10,.00],[ .00,.04,.08,.00],[ .06,.02,.00,.05],[ .00,.03,.05,.00],[ .04,.00,.00,.10],[ .08,.02,.00,.00]]),
'physical':np.array([[ .10,.00,.03,.00],[ .00,.11,.00,.04],[ .05,.02,.00,.03],[ .00,.08,.03,.00],[ .04,.00,.07,.00],[ .00,.05,.00,.08],[ .05,.01,.04,.00],[ .03,.00,.00,.10]]),
'spectral':np.array([[ .11,.00,.04,.00],[ .00,.10,.00,.05],[ .06,.02,.00,.04],[ .00,.05,.08,.00],[ .02,.00,.06,.00],[ .07,.03,.00,.05],[ .00,.08,.00,.06],[ .05,.00,.07,.00]]),
'chaos':np.array([[ .06,.02,.00,.08],[ .00,.07,.02,.10],[ .03,.00,.08,.05],[ .08,.02,.00,.04],[ .02,.09,.03,.00],[ .00,.04,.07,.06],[ .05,.00,.05,.04],[ .00,.03,.04,.09]])}
BIAS={k:BASE[k].copy() for k in FAMS}

def smoothstep(x):
    x=np.clip(x,0,1); return x*x*(3-2*x)

def family_params(name,z,w):
    # family-local global directions 2/3 emerge continuously with family activity.
    zz=np.asarray(z,float).copy(); zz[2:]*=(0.25+0.75*w)
    p=BIAS[name]+MAP[name]@np.tanh(1.3*zz)
    return np.clip(p,.08,.92)

def render_engine(weights,z,seed):
    weights=np.asarray(weights,float); weights=np.maximum(weights,0); weights=weights/(np.sum(weights)+1e-12)
    gains=np.sqrt(weights); gains=gains/(np.sqrt(np.sum(gains*gains))+1e-12)
    y=0.0
    for i,name in enumerate(FAMS):
        if gains[i]<1e-8: continue
        p=family_params(name,z,weights[i])
        y=y+gains[i]*FAMILIES[name].render(p,seed+97*i)
    return norm(y)

def render_crossfade(a,b,alpha,z,seed):
    s=smoothstep(alpha); wa=np.sqrt(1-s); wb=np.sqrt(s)
    ya=FAMILIES[a].render(BASE[a],seed); yb=FAMILIES[b].render(BASE[b],seed+97)
    return norm(wa*ya+wb*yb)

def render_naive(a,b,alpha,z,seed):
    # Naive paired native interpolation: both engines share the same interpolated 8-vector.
    s=smoothstep(alpha); p=(1-s)*BASE[a]+s*BASE[b]
    ya=FAMILIES[a].render(p,seed); yb=FAMILIES[b].render(p,seed+97)
    return norm(np.sqrt(1-s)*ya+np.sqrt(s)*yb)

def weights_for(a,b,alpha):
    s=smoothstep(alpha); w=np.zeros(4); w[FAMS.index(a)]=1-s; w[FAMS.index(b)]=s; return w

def jac_z(a,b,alpha,z,seed):
    w=weights_for(a,b,alpha); f0=extract_features(render_engine(w,z,seed)); J=np.zeros((len(f0),len(z)))
    for j in range(len(z)):
        lo=z.copy(); hi=z.copy(); lo[j]-=STEP; hi[j]+=STEP
        J[:,j]=(extract_features(render_engine(w,hi,seed))-extract_features(render_engine(w,lo,seed)))/(2*STEP)
    return J

def effdim(J,q=.95):
    s=np.linalg.svd(J,compute_uv=False); e=s*s; c=np.cumsum(e)/(np.sum(e)+1e-15); return int(np.searchsorted(c,q)+1)

def dir_overlap(a,b):
    na=np.linalg.norm(a); nb=np.linalg.norm(b)
    if na<1e-12 or nb<1e-12:return 1.0
    return float(abs(np.dot(a,b)/(na*nb)))

def path_eval(a,b,seed):
    featsE=[]; featsC=[]; featsN=[]; Js=[]; d95=[]
    for alpha in ALPHAS:
        w=weights_for(a,b,alpha)
        featsE.append(extract_features(render_engine(w,Z0,seed)))
        featsC.append(extract_features(render_crossfade(a,b,alpha,Z0,seed)))
        featsN.append(extract_features(render_naive(a,b,alpha,Z0,seed)))
        J=jac_z(a,b,alpha,Z0,seed); Js.append(J); d95.append(effdim(J,.95))
    def continuity(F):
        F=np.asarray(F); st=np.linalg.norm(np.diff(F,axis=0),axis=1); med=float(np.median(st)+1e-12)
        return {'p95':float(np.percentile(st,95)),'max_ratio':float(np.max(st)/med),'median':med}
    ce,cc,cn=continuity(featsE),continuity(featsC),continuity(featsN)
    # control continuity per retained global direction
    good=[]
    for j in range(4):
        mags=np.array([np.linalg.norm(J[:,j]) for J in Js]); mx=float(np.max(mags)+1e-12)
        ovs=[]; elig=[]
        for i in range(len(Js)-1):
            active=min(mags[i],mags[i+1])>=.1*mx
            if active: ovs.append(dir_overlap(Js[i][:,j],Js[i+1][:,j])); elig.append(1)
        good.append(float(np.mean(np.array(ovs)>=.7)) if ovs else 1.0)
    better=min(cc['p95'],cn['p95'])
    audio_pass=ce['p95']<=better+1e-12 and ce['max_ratio']<=3.0
    control_pass=all(x>=.90 for x in good)
    dim_adapt=len(set(d95))>1
    return {'route':f'{a}->{b}','engine':ce,'crossfade':cc,'naive':cn,'control_overlap_fractions':good,'d95':d95,'audio_pass':audio_pass,'control_pass':control_pass,'dim_adapt':dim_adapt,'route_pass':audio_pass and control_pass and dim_adapt}

def main():
    routes=[('granular','spectral'),('physical','spectral'),('granular','physical'),('spectral','chaos')]
    allruns=[]
    for seed in SEEDS:
        for a,b in routes: allruns.append({'seed':seed,**path_eval(a,b,seed)})
    # frozen decision uses held-out seed only
    held=[r for r in allruns if r['seed']==SEEDS[1]]
    passes=sum(int(r['route_pass']) for r in held)
    chaos_ok=any(r['route_pass'] and 'chaos' in r['route'] for r in held)
    overall=passes>=3 and chaos_ok
    out={'routes':allruns,'heldout_seed':SEEDS[1],'heldout_route_passes':passes,'chaos_route_pass':chaos_ok,'overall_pass':overall,'criteria':'Part 14 preregistration issue #15'}
    (OUT/'part14_result.json').write_text(json.dumps(out,indent=2)+'\n')
    print(json.dumps(out,indent=2))
if __name__=='__main__':main()
