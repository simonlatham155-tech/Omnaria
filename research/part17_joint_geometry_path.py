#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np

from part15_joint_regularized_engine import fit_anchors, fit_maps, params15, render_weighted, render_crossfade, jac_z, continuity
from part16_transported_control_basis import local_svd, transport_basis, acoustic_control_jac, effdim, ov
from part6_regenerate_additive_granular import extract_features

OUT=Path('research/data/part17'); OUT.mkdir(parents=True,exist_ok=True)
FAMS=['granular','physical','spectral','chaos']
TEST_SEED=2449490
ALPHAS=np.linspace(0.0,1.0,31)
Z0=np.zeros(4)
COST={'acoustic':1.0,'subspace':0.55,'rank':0.12,'weight_step':0.10,'nominal':0.08}


def json_default(o):
    if isinstance(o, np.bool_): return bool(o)
    if isinstance(o, np.integer): return int(o)
    if isinstance(o, np.floating): return float(o)
    if isinstance(o, np.ndarray): return o.tolist()
    raise TypeError(f'Object of type {o.__class__.__name__} is not JSON serializable')


def smoothstep(x):
    x=np.clip(x,0.0,1.0); return x*x*(3.0-2.0*x)


def nominal_weights(route,a):
    s=smoothstep(a); w=np.zeros(4)
    if route=='granular-chaos-physical-spectral':
        p=.20*np.exp(-.5*((s-.34)/.17)**2); q=.20*np.exp(-.5*((s-.68)/.17)**2)
        extra=min(.38,p+q); rem=1-extra
        w[0]=rem*(1-s); w[3]=rem*s
        if p+q>1e-12: w[1]=extra*p/(p+q); w[2]=extra*q/(p+q)
    elif route=='physical-granular-via-spectral':
        m=.30*4*s*(1-s); rem=1-m; w[1]=rem*(1-s); w[0]=rem*s; w[2]=m
    elif route=='spectral-physical-via-chaos':
        m=.28*4*s*(1-s); rem=1-m; w[2]=rem*(1-s); w[1]=rem*s; w[3]=m
    elif route=='chaos-granular-via-physical':
        m=.30*4*s*(1-s); rem=1-m; w[3]=rem*(1-s); w[0]=rem*s; w[1]=m
    else: raise ValueError(route)
    return w/(np.sum(w)+1e-12)


def simplex(x):
    x=np.maximum(np.asarray(x,float),0.0); return x/(np.sum(x)+1e-12)

PERT=np.array([
    [0,0,0,0], [1,-1,0,0], [-1,1,0,0], [0,1,-1,0], [0,-1,1,0], [0,0,1,-1], [0,0,-1,1]
],float)


def candidates(route,a):
    n=nominal_weights(route,a)
    taper=4*a*(1-a)
    out=[]
    for p in PERT:
        out.append(simplex(n + .055*taper*p))
    uniq=[]
    for w in out:
        if not any(np.max(np.abs(w-u))<1e-9 for u in uniq): uniq.append(w)
    return uniq


def subspace_distance(Ja,Jb):
    Ua,sa,_,ra=local_svd(Ja); Ub,sb,_,rb=local_svd(Jb)
    k=min(ra,rb)
    if k<=0:return 1.0
    A=Ua[:,:k]; B=Ub[:,:k]
    sv=np.linalg.svd(A.T@B,compute_uv=False)
    return float(1.0-np.mean(np.clip(sv,0,1)))


def precompute(route,p15):
    layers=[]
    for a in ALPHAS:
        nom=nominal_weights(route,a); layer=[]
        for w in candidates(route,a):
            f=extract_features(render_weighted(w,Z0,TEST_SEED,p15))
            J=jac_z(w,Z0,TEST_SEED,p15)
            _,_,_,r=local_svd(J)
            layer.append({'w':w,'f':f,'J':J,'r':r,'d95':effdim(J),'nomdev':float(np.linalg.norm(w-nom))})
        layers.append(layer)
    return layers


def edge_cost(a,b):
    ac=np.linalg.norm(b['f']-a['f'])
    sd=subspace_distance(a['J'],b['J'])
    rd=abs(b['r']-a['r'])
    wd=np.linalg.norm(b['w']-a['w'])
    return COST['acoustic']*ac + COST['subspace']*sd + COST['rank']*rd + COST['weight_step']*wd + COST['nominal']*b['nomdev']


def optimize(layers):
    dp=[np.full(len(layer),np.inf) for layer in layers]
    back=[np.full(len(layer),-1,int) for layer in layers]
    dp[0][:]=[COST['nominal']*x['nomdev'] for x in layers[0]]
    for i in range(1,len(layers)):
        for j,b in enumerate(layers[i]):
            vals=[dp[i-1][k]+edge_cost(a,b) for k,a in enumerate(layers[i-1])]
            k=int(np.argmin(vals)); dp[i][j]=vals[k]; back[i][j]=k
    j=int(np.argmin(dp[-1])); idx=[j]
    for i in range(len(layers)-1,0,-1):
        j=int(back[i][j]); idx.append(j)
    idx=idx[::-1]
    return [layers[i][idx[i]] for i in range(len(layers))], float(np.min(dp[-1]))


def rank_flicker_free(ranks):
    if len(set(ranks))<=1:return False
    for i in range(1,len(ranks)-1):
        if ranks[i]!=ranks[i-1] and ranks[i+1]==ranks[i-1]:
            return False
    return True


def evaluate(route,anchors,maps):
    p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    layers=precompute(route,p15); path,cost=optimize(layers)
    feats=[x['f'] for x in path]; cp=continuity(feats)
    f15=[]; fcf=[]
    for a in ALPHAS:
        w=nominal_weights(route,a)
        f15.append(extract_features(render_weighted(w,Z0,TEST_SEED,p15)))
        fcf.append(extract_features(render_crossfade(w,TEST_SEED)))
    c15=continuity(f15); ccf=continuity(fcf)
    prev=None; Jus=[]; ranks=[]; d95=[]
    for x in path:
        _,_,V,r=local_svd(x['J']); B=transport_basis(prev,V,r); prev=B
        Jus.append(acoustic_control_jac(x['w'],B,TEST_SEED,p15)); ranks.append(r); d95.append(x['d95'])
    ovs=[]
    for i in range(len(Jus)-1):
        k=min(Jus[i].shape[1],Jus[i+1].shape[1])
        ovs.extend([ov(Jus[i][:,j],Jus[i+1][:,j]) for j in range(k)])
    control_fraction=float(np.mean(np.asarray(ovs)>=.85)) if ovs else 1.0
    audio_better=cp['p95']<=min(c15['p95'],ccf['p95'])+1e-12
    max_ok=cp['max_ratio']<=3.0
    control_ok=control_fraction>=.90
    adapt_ok=rank_flicker_free(ranks) or rank_flicker_free(d95)
    endpoint_ok=np.max(np.abs(path[0]['w']-nominal_weights(route,0)))<1e-9 and np.max(np.abs(path[-1]['w']-nominal_weights(route,1)))<1e-9
    passed=audio_better and max_ok and control_ok and adapt_ok and endpoint_ok
    return {
      'route':route,'planned':cp,'part15_nominal':c15,'crossfade':ccf,'planner_cost':cost,
      'control_overlap_fraction_ge_085':control_fraction,'active_rank':ranks,'d95':d95,
      'audio_better':audio_better,'max_ok':max_ok,'control_ok':control_ok,
      'dim_adapt_stable':adapt_ok,'endpoint_ok':endpoint_ok,'route_pass':passed,
      'mean_nominal_deviation':float(np.mean([x['nomdev'] for x in path]))
    }


def main():
    anchors=fit_anchors(); maps=fit_maps(anchors)
    routes=['granular-chaos-physical-spectral','physical-granular-via-spectral','spectral-physical-via-chaos','chaos-granular-via-physical']
    rows=[evaluate(r,anchors,maps) for r in routes]
    passes=sum(int(r['route_pass']) for r in rows)
    chaos_ok=any(r['route_pass'] and 'chaos' in r['route'] for r in rows)
    out={'heldout_seed':TEST_SEED,'cost_weights':COST,'routes':rows,'route_passes':passes,
         'chaos_route_pass':chaos_ok,'overall_pass':passes>=3 and chaos_ok,'criteria':'Part 17 issue #18'}
    text=json.dumps(out,indent=2,default=json_default)+'\n'
    (OUT/'part17_result.json').write_text(text)
    print(text,end='')

if __name__=='__main__': main()
