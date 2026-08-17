#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np

from part15_joint_regularized_engine import fit_anchors, fit_maps, params15, render_weighted, render_crossfade, jac_z, continuity
from part16_transported_control_basis import acoustic_control_jac, ov
from part6_regenerate_additive_granular import extract_features

OUT = Path('research/data/part18'); OUT.mkdir(parents=True, exist_ok=True)
FAMS = ['granular','physical','spectral','chaos']
TRAIN_SEEDS = [1800451, 1900457, 2000461]
TEST_SEED = 2662703
ALPHAS = np.linspace(0.0, 1.0, 41)
Z0 = np.zeros(4)


def smoothstep(x):
    x=np.clip(x,0.0,1.0); return x*x*(3.0-2.0*x)


def route_weights(route,a):
    s=smoothstep(a); w=np.zeros(4)
    if route=='granular-physical-via-spectral':
        m=.26*4*s*(1-s); rem=1-m; w[0]=rem*(1-s); w[1]=rem*s; w[2]=m
    elif route=='physical-chaos-via-granular':
        m=.28*4*s*(1-s); rem=1-m; w[1]=rem*(1-s); w[3]=rem*s; w[0]=m
    elif route=='spectral-granular-via-physical':
        m=.27*4*s*(1-s); rem=1-m; w[2]=rem*(1-s); w[0]=rem*s; w[1]=m
    elif route=='chaos-spectral-via-granular':
        m=.25*4*s*(1-s); rem=1-m; w[3]=rem*(1-s); w[2]=rem*s; w[0]=m
    else: raise ValueError(route)
    return w/(np.sum(w)+1e-12)


def chart_bases(p15):
    bases={}; ranks={}
    for fi,name in enumerate(FAMS):
        w=np.zeros(4); w[fi]=1.0
        G=np.zeros((4,4))
        for seed in TRAIN_SEEDS:
            J=jac_z(w,Z0,seed,p15)
            G += J.T@J
        G/=len(TRAIN_SEEDS)
        vals,V=np.linalg.eigh(G); order=np.argsort(vals)[::-1]; vals=vals[order]; V=V[:,order]
        r=max(1,int(np.sum(vals >= .01*(vals[0]+1e-15))))
        bases[name]=V
        ranks[name]=r
    return bases,ranks


def align_basis(A,B):
    # Rotate B into the coordinate convention of A using orthogonal Procrustes.
    U,_,Vt=np.linalg.svd(B.T@A,full_matrices=False)
    return B@(U@Vt)


def blended_chart_basis(w,bases):
    order=np.argsort(w)[::-1]
    i,j=int(order[0]),int(order[1])
    A=bases[FAMS[i]]; B=align_basis(A,bases[FAMS[j]])
    denom=w[i]+w[j]+1e-12; t=float(w[j]/denom)
    M=(1-t)*A+t*B
    Q,_=np.linalg.qr(M)
    return Q[:,:4], i, j, t


def stable_handoffs(dominant):
    changes=[]
    for i in range(1,len(dominant)):
        if dominant[i]!=dominant[i-1]:
            # require persistence for two samples; reject immediate reversal
            if i+1 < len(dominant) and dominant[i+1]==dominant[i]:
                changes.append(i)
    return changes


def evaluate(route,anchors,maps,bases):
    p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    feats=[]; feats_cf=[]; Jus=[]; dominant=[]; secondary=[]; blend=[]
    for a in ALPHAS:
        w=route_weights(route,a)
        B,i,j,t=blended_chart_basis(w,bases)
        feats.append(extract_features(render_weighted(w,Z0,TEST_SEED,p15)))
        feats_cf.append(extract_features(render_crossfade(w,TEST_SEED)))
        Jus.append(acoustic_control_jac(w,B,TEST_SEED,p15))
        dominant.append(i); secondary.append(j); blend.append(t)
    cp=continuity(feats); ccf=continuity(feats_cf)

    overlaps=[]
    for i in range(len(Jus)-1):
        na=np.linalg.norm(Jus[i],axis=0); nb=np.linalg.norm(Jus[i+1],axis=0)
        thr=.10*max(float(np.max(na)),float(np.max(nb)),1e-12)
        for j in range(4):
            if na[j]>=thr and nb[j]>=thr:
                overlaps.append(ov(Jus[i][:,j],Jus[i+1][:,j]))
    control_fraction=float(np.mean(np.asarray(overlaps)>=.85)) if overlaps else 1.0
    handoffs=stable_handoffs(dominant)

    audio_better=cp['p95']<=ccf['p95']+1e-12
    max_ok=cp['max_ratio']<=3.0
    control_ok=control_fraction>=.90
    endpoint_ok=(dominant[0]==int(np.argmax(route_weights(route,0.0))) and dominant[-1]==int(np.argmax(route_weights(route,1.0))))
    handoff_ok=len(handoffs)>=1
    passed=audio_better and max_ok and control_ok and endpoint_ok and handoff_ok
    return {
        'route':route,'local_chart':cp,'crossfade':ccf,
        'control_overlap_fraction_ge_085':control_fraction,
        'dominant_chart':[FAMS[i] for i in dominant],
        'handoff_indices':handoffs,'handoff_count':len(handoffs),
        'audio_better':bool(audio_better),'max_ok':bool(max_ok),'control_ok':bool(control_ok),
        'endpoint_ok':bool(endpoint_ok),'handoff_ok':bool(handoff_ok),'route_pass':bool(passed)
    }


def main():
    anchors=fit_anchors(); maps=fit_maps(anchors)
    p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    bases,ranks=chart_bases(p15)
    routes=['granular-physical-via-spectral','physical-chaos-via-granular','spectral-granular-via-physical','chaos-spectral-via-granular']
    rows=[evaluate(r,anchors,maps,bases) for r in routes]
    passes=sum(int(r['route_pass']) for r in rows)
    chaos_ok=any(r['route_pass'] and 'chaos' in r['route'] for r in rows)
    out={'heldout_seed':TEST_SEED,'train_seeds':TRAIN_SEEDS,'chart_ranks':ranks,'routes':rows,
         'route_passes':passes,'chaos_route_pass':bool(chaos_ok),'overall_pass':bool(passes>=3 and chaos_ok),
         'criteria':'Part 18 preregistration issue #19'}
    text=json.dumps(out,indent=2)+'\n'; (OUT/'part18_result.json').write_text(text); print(text)

if __name__=='__main__': main()
