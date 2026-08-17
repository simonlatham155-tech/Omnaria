#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np

from part15_joint_regularized_engine import fit_anchors, fit_maps, params15, render_weighted, render_crossfade, jac_z, continuity
from part18_local_chart_handoff import chart_bases, stable_handoffs, smoothstep
from part6_regenerate_additive_granular import extract_features

OUT=Path('research/data/part19'); OUT.mkdir(parents=True,exist_ok=True)
FAMS=['granular','physical','spectral','chaos']
TRAIN_SEEDS=[2111171,2111183,2111207]
TEST_SEED=2889137
ALPHAS=np.linspace(0.0,1.0,41)
Z0=np.zeros(4); STEP=.025

# Feature order: rms, centroid, spread, flatness, high-band, flux, attack, zcr, partial-density, transient-complexity.
TARGETS={
 'brightness':np.array([0,.55,.30,.05,.55,.05,0,.15,0,0.]),
 'transientness':np.array([0,0,.05,0,.05,.45,.55,.05,0,.55]),
 'density':np.array([0,.05,.15,.30,.05,.10,0,.10,.70,.20]),
 'body':np.array([.55,-.30,-.10,-.35,-.20,0,.05,-.05,-.05,0.]),
}
TARGETS={k:v/(np.linalg.norm(v)+1e-15) for k,v in TARGETS.items()}
SEM=list(TARGETS)


def route_weights(route,a):
    s=smoothstep(a); w=np.zeros(4)
    if route=='granular-spectral-via-chaos':
        m=.29*4*s*(1-s); rem=1-m; w[0]=rem*(1-s); w[2]=rem*s; w[3]=m
    elif route=='physical-granular-via-chaos':
        m=.27*4*s*(1-s); rem=1-m; w[1]=rem*(1-s); w[0]=rem*s; w[3]=m
    elif route=='chaos-physical-via-spectral':
        m=.26*4*s*(1-s); rem=1-m; w[3]=rem*(1-s); w[1]=rem*s; w[2]=m
    elif route=='spectral-chaos-via-granular':
        m=.28*4*s*(1-s); rem=1-m; w[2]=rem*(1-s); w[3]=rem*s; w[0]=m
    else: raise ValueError(route)
    return w/(np.sum(w)+1e-12)


def semantic_dirs(p15,bases,ranks):
    out={}
    for fi,f in enumerate(FAMS):
        w=np.zeros(4); w[fi]=1
        J=sum(jac_z(w,Z0,s,p15) for s in TRAIN_SEEDS)/len(TRAIN_SEEDS)
        B=bases[f][:,:ranks[f]]; A=J@B
        cols=[]
        for name in SEM:
            t=TARGETS[name]
            G=A.T@A + 1e-3*np.eye(A.shape[1])
            c=np.linalg.solve(G,A.T@t)
            d=B@c; n=np.linalg.norm(d)
            if n<1e-10:
                d=B[:,0].copy(); n=np.linalg.norm(d)
            d=d/n
            if np.dot(J@d,t)<0:d=-d
            cols.append(d)
        out[f]=np.column_stack(cols)
    return out


def semantic_frame(w,dirs):
    order=np.argsort(w)[::-1]; i,j=int(order[0]),int(order[1])
    den=w[i]+w[j]+1e-12; t=float(w[j]/den)
    D=(1-t)*dirs[FAMS[i]]+t*dirs[FAMS[j]]
    for k in range(D.shape[1]):
        n=np.linalg.norm(D[:,k]);
        if n>1e-12:D[:,k]/=n
    return D,i,j,t


def effects(w,D,seed,p15):
    E=np.zeros((10,len(SEM)))
    for j in range(len(SEM)):
        z=STEP*D[:,j]
        hi=extract_features(render_weighted(w,z,seed,p15)); lo=extract_features(render_weighted(w,-z,seed,p15))
        E[:,j]=(hi-lo)/(2*STEP)
    return E


def cos(a,b):
    na=np.linalg.norm(a); nb=np.linalg.norm(b)
    if na<1e-12 or nb<1e-12:return 1.0
    return float(np.dot(a,b)/(na*nb))


def evaluate(route,anchors,maps,dirs):
    p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    feats=[]; cf=[]; Es=[]; dom=[]
    for a in ALPHAS:
        w=route_weights(route,a); D,i,_,_=semantic_frame(w,dirs)
        feats.append(extract_features(render_weighted(w,Z0,TEST_SEED,p15)))
        cf.append(extract_features(render_crossfade(w,TEST_SEED)))
        Es.append(effects(w,D,TEST_SEED,p15)); dom.append(i)
    cp=continuity(feats); ccf=continuity(cf)

    target_scores=[]; target_flags=[]
    for E in Es:
        norms=np.linalg.norm(E,axis=0); thr=.10*max(float(np.max(norms)),1e-12)
        for j,name in enumerate(SEM):
            if norms[j]>=thr:
                c=cos(E[:,j],TARGETS[name]); target_scores.append(c); target_flags.append(c>=.80)
    target_fraction=float(np.mean(target_flags)) if target_flags else 1.0

    adjacent=[]
    for a,b in zip(Es[:-1],Es[1:]):
        na=np.linalg.norm(a,axis=0); nb=np.linalg.norm(b,axis=0); thr=.10*max(float(np.max(na)),float(np.max(nb)),1e-12)
        for j in range(len(SEM)):
            if na[j]>=thr and nb[j]>=thr: adjacent.append(cos(a[:,j],b[:,j]))
    adjacent_fraction=float(np.mean(np.asarray(adjacent)>=.70)) if adjacent else 1.0
    handoffs=stable_handoffs(dom)

    # Neutral semantic controls share the Part-18 local-chart acoustic path; require it not to lose to crossfade.
    audio_ok=cp['p95']<=ccf['p95']+1e-12
    max_ok=cp['max_ratio']<=3.0
    semantic_ok=target_fraction>=.90
    adjacent_ok=adjacent_fraction>=.85
    endpoint_ok=(dom[0]==int(np.argmax(route_weights(route,0))) and dom[-1]==int(np.argmax(route_weights(route,1))))
    handoff_ok=len(handoffs)>=1
    passed=audio_ok and max_ok and semantic_ok and adjacent_ok and endpoint_ok and handoff_ok
    return {'route':route,'semantic_chart':cp,'crossfade':ccf,
      'semantic_target_fraction_ge_080':target_fraction,'adjacent_semantic_fraction_ge_070':adjacent_fraction,
      'mean_semantic_target_cosine':float(np.mean(target_scores)) if target_scores else 1.0,
      'handoff_indices':handoffs,'handoff_count':len(handoffs),
      'audio_ok':bool(audio_ok),'max_ok':bool(max_ok),'semantic_ok':bool(semantic_ok),'adjacent_ok':bool(adjacent_ok),
      'endpoint_ok':bool(endpoint_ok),'handoff_ok':bool(handoff_ok),'route_pass':bool(passed)}


def main():
    anchors=fit_anchors(); maps=fit_maps(anchors); p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    bases,ranks=chart_bases(p15); dirs=semantic_dirs(p15,bases,ranks)
    routes=['granular-spectral-via-chaos','physical-granular-via-chaos','chaos-physical-via-spectral','spectral-chaos-via-granular']
    rows=[evaluate(r,anchors,maps,dirs) for r in routes]
    passes=sum(int(r['route_pass']) for r in rows); chaos_ok=any(r['route_pass'] and 'chaos' in r['route'] for r in rows)
    out={'heldout_seed':TEST_SEED,'train_seeds':TRAIN_SEEDS,'semantic_controls':SEM,'chart_ranks':ranks,'routes':rows,
      'route_passes':passes,'chaos_route_pass':bool(chaos_ok),'overall_pass':bool(passes>=3 and chaos_ok),'criteria':'Part 19 preregistration issue #20'}
    text=json.dumps(out,indent=2)+'\n'; (OUT/'part19_result.json').write_text(text); print(text)

if __name__=='__main__':main()
