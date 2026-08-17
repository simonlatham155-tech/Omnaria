#!/usr/bin/env python3
from __future__ import annotations
import json, hashlib
from pathlib import Path
import numpy as np
from part12_richer_families import FAMILIES, BASE, norm
from part6_regenerate_additive_granular import extract_features

OUT=Path('research/data/part13'); OUT.mkdir(parents=True,exist_ok=True)
PATHS=[('granular','spectral'),('physical','spectral'),('granular','physical'),('spectral','chaos')]
GRID=np.linspace(0.0,1.0,41)
STEP=0.018
SEED=1732050
HELDOUT_SEED=2236067
ENDPOINT_DELTA=np.array([.035,-.025,.03,-.02,.025,.02,-.03,.018])
EPS=1e-12

# Inherited from the earlier persistence experiments; fixed before traces.
PERSISTENCE_RATIO_REFERENCE=50.0

def smoothstep(x):
    x=np.clip(float(x),0.0,1.0); return x*x*(3.0-2.0*x)

def stable_matrix(tag):
    h=int(hashlib.sha256(tag.encode()).hexdigest()[:16],16) & 0xffffffff
    rng=np.random.default_rng(h)
    q,_=np.linalg.qr(rng.normal(size=(8,8)))
    return q

def endpoint_vectors(a,b,perturbed=False):
    pa=BASE[a].copy(); pb=BASE[b].copy()
    if perturbed:
        pa=np.clip(pa+ENDPOINT_DELTA,.1,.9)
        pb=np.clip(pb-ENDPOINT_DELTA[::-1],.1,.9)
    return pa,pb

def render_path(a,b,t,z,seed,perturbed=False):
    pa,pb=endpoint_vectors(a,b,perturbed)
    Qa=stable_matrix(a+'__'+b+'__A'); Qb=stable_matrix(a+'__'+b+'__B')
    z=np.asarray(z,float)
    # Each endpoint remains in its own native coordinate system.
    pa=np.clip(pa + 0.12*(Qa@z),.05,.95)
    pb=np.clip(pb + 0.12*(Qb@z),.05,.95)
    w=smoothstep(t)
    xa=FAMILIES[a].render(pa,seed)
    xb=FAMILIES[b].render(pb,seed+100003)
    return norm(np.sqrt(max(0.0,1.0-w))*xa + np.sqrt(w)*xb)

def jacobian(a,b,t,seed,perturbed=False):
    J=np.zeros((10,8),float)
    for k in range(8):
        lo=np.zeros(8); hi=np.zeros(8); lo[k]=-STEP; hi[k]=STEP
        flo=extract_features(render_path(a,b,t,lo,seed,perturbed))
        fhi=extract_features(render_path(a,b,t,hi,seed,perturbed))
        J[:,k]=(fhi-flo)/(2*STEP)
    return J

def effdim(s,q):
    e=s*s; c=np.cumsum(e)/(np.sum(e)+EPS); return int(np.searchsorted(c,q)+1)

def subspace_overlap(U,V,d):
    d=max(1,min(int(d),U.shape[1],V.shape[1]))
    sv=np.linalg.svd(U[:,:d].T@V[:,:d],compute_uv=False)
    return float(np.mean(np.clip(sv,0,1)))

def trace(a,b,seed,perturbed=False):
    rows=[]; metrics=[]; bases=[]
    for t in GRID:
        J=jacobian(a,b,float(t),seed,perturbed)
        U,s,_=np.linalg.svd(J,full_matrices=False)
        d90=effdim(s,.90); d95=effdim(s,.95); d99=effdim(s,.99)
        G=J@J.T
        rows.append({'t':float(t),'d90':d90,'d95':d95,'d99':d99,'singular_values':s.tolist()})
        metrics.append(G); bases.append(U)
    # Adjacent dominant-subspace rotation uses the smaller local d95.
    rotations=[]
    metric_steps=[]
    for i in range(len(GRID)-1):
        d=min(rows[i]['d95'],rows[i+1]['d95'])
        ov=subspace_overlap(bases[i],bases[i+1],d)
        rotations.append(1.0-ov)
        metric_steps.append(float(np.linalg.norm(metrics[i+1]-metrics[i],'fro')))
    med_step=float(np.median(metric_steps))+EPS
    # Three-sample pre/post persistent metric displacement, same conceptual test as Part 8/9.
    pers=np.zeros(len(GRID),float)
    for i in range(3,len(GRID)-3):
        pre=np.mean(metrics[i-3:i],axis=0); post=np.mean(metrics[i:i+3],axis=0)
        pers[i]=np.linalg.norm(post-pre,'fro')/med_step
    peak_i=int(np.argmax(pers)); peak_t=float(GRID[peak_i]); peak_ratio=float(pers[peak_i])
    # Sustained subspace separation across the same pre/post windows.
    preU=np.linalg.svd(np.mean(metrics[max(0,peak_i-3):peak_i],axis=0),full_matrices=False)[0]
    postU=np.linalg.svd(np.mean(metrics[peak_i:min(len(metrics),peak_i+3)],axis=0),full_matrices=False)[0]
    d=max(1,min(rows[max(0,peak_i-1)]['d95'],rows[min(len(rows)-1,peak_i+1)]['d95']))
    prepost_overlap=subspace_overlap(preU,postU,d)
    return {
        'rows':rows,
        'adjacent_subspace_rotation':rotations,
        'metric_steps':metric_steps,
        'persistence':pers.tolist(),
        'median_metric_step':med_step,
        'peak_t':peak_t,
        'peak_ratio':peak_ratio,
        'peak_prepost_overlap':prepost_overlap,
        'd95_unique':sorted(set(r['d95'] for r in rows)),
    }

def classify(base,held,pert):
    stable_seed=abs(base['peak_t']-held['peak_t'])<=0.05+1e-12
    stable_endpoint=abs(base['peak_t']-pert['peak_t'])<=0.05+1e-12
    localized=(0.075<=base['peak_t']<=0.925)
    strong=base['peak_ratio']>=PERSISTENCE_RATIO_REFERENCE
    sustained=base['peak_prepost_overlap']<0.90
    return {
        'candidate_handoff':bool(stable_seed and stable_endpoint and localized and strong and sustained),
        'stable_seed':bool(stable_seed),'stable_endpoint':bool(stable_endpoint),
        'localized':bool(localized),'strong_persistence':bool(strong),'sustained_subspace_separation':bool(sustained)
    }

def main():
    out={'grid':GRID.tolist(),'step':STEP,'paths':{},'persistence_reference':PERSISTENCE_RATIO_REFERENCE,
         'seed':SEED,'heldout_seed':HELDOUT_SEED,'endpoint_delta':ENDPOINT_DELTA.tolist()}
    for a,b in PATHS:
        key=f'{a}__{b}'
        base=trace(a,b,SEED,False)
        held=trace(a,b,HELDOUT_SEED,False)
        pert=trace(a,b,SEED,True)
        out['paths'][key]={'baseline':base,'heldout_seed':held,'endpoint_perturbation':pert,'classification':classify(base,held,pert)}
        print(key,'peak',base['peak_t'],'ratio',base['peak_ratio'],'d95',base['d95_unique'],'handoff',out['paths'][key]['classification']['candidate_handoff'])
    (OUT/'part13_result.json').write_text(json.dumps(out,indent=2)+'\n')
    print('Wrote',OUT/'part13_result.json')
if __name__=='__main__': main()
