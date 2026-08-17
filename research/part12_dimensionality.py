#!/usr/bin/env python3
from __future__ import annotations
import json,itertools
from pathlib import Path
import numpy as np
from part12_richer_families import FAMILIES,BASE
from part6_regenerate_additive_granular import extract_features
OUT=Path('research/data/part12'); OUT.mkdir(parents=True,exist_ok=True)
SEEDS=[1618033,2718281,3141592]; STEP=0.02
OFFSETS=[np.array(v,float) for v in [
[0,0,0,0,0,0,0,0],[.12,-.08,.1,-.1,.08,.05,-.06,.09],[-.1,.1,-.08,.12,-.06,.09,.08,-.05],[.08,.06,-.12,.04,-.1,-.08,.11,.07],[-.06,-.1,.09,.08,.12,.04,-.09,-.07]]]

def jac(fam,p,seed):
    f0=extract_features(fam.render(p,seed)); J=np.zeros((len(f0),len(p)))
    for i in range(len(p)):
        lo=p.copy(); hi=p.copy(); lo[i]=max(.05,p[i]-STEP); hi[i]=min(.95,p[i]+STEP)
        J[:,i]=(extract_features(fam.render(hi,seed))-extract_features(fam.render(lo,seed)))/(hi[i]-lo[i])
    return J

def effdim(s,q):
    e=s*s; c=np.cumsum(e)/(np.sum(e)+1e-15); return int(np.searchsorted(c,q)+1)

def basis(J,d):
    U,s,V=np.linalg.svd(J,full_matrices=False); return U[:,:d],s

def overlap(A,B,d):
    d=min(d,A.shape[1],B.shape[1]);
    if d<1:return 0.0
    sv=np.linalg.svd(A[:,:d].T@B[:,:d],compute_uv=False); return float(np.mean(sv))

def main():
    rec={}; meanbases={}
    for name,fam in FAMILIES.items():
        rows=[]; bases=[]
        for oi,off in enumerate(OFFSETS):
            p=np.clip(BASE[name]+off,.1,.9)
            for seed in SEEDS:
                J=jac(fam,p,seed); U,s=np.linalg.svd(J,full_matrices=False)[:2]
                rows.append({'point':oi,'seed':seed,'d90':effdim(s,.90),'d95':effdim(s,.95),'d99':effdim(s,.99),'singular_values':s.tolist(),'condition':float(s[0]/max(s[-1],1e-12))})
                bases.append(U)
        ds=[r['d95'] for r in rows]; dmed=int(round(float(np.median(ds))))
        # stable family acoustic basis from concatenated Jacobians, not pair-fitted
        Js=[]
        for oi,off in enumerate(OFFSETS):
            p=np.clip(BASE[name]+off,.1,.9); Js.append(jac(fam,p,SEEDS[0]))
        U,s,_=np.linalg.svd(np.concatenate(Js,axis=1),full_matrices=False); meanbases[name]=U
        rec[name]={'native_parameters':list(fam.names),'samples':rows,'median_d95':dmed,'d95_range':[int(min(ds)),int(max(ds))]}
    pairs={}
    for a,b in itertools.combinations(FAMILIES,2):
        da=rec[a]['median_d95']; db=rec[b]['median_d95']; m=min(da,db)
        pairs[f'{a}__{b}']={str(d):overlap(meanbases[a],meanbases[b],d) for d in range(1,m+1)}
    # held-out seed stability of family dominant bases
    held={}
    hs=1414213
    for name,fam in FAMILIES.items():
        Js=[]
        for off in OFFSETS[:3]:
            p=np.clip(BASE[name]+off,.1,.9); Js.append(jac(fam,p,hs))
        U,_,_=np.linalg.svd(np.concatenate(Js,axis=1),full_matrices=False); d=rec[name]['median_d95']; held[name]=overlap(meanbases[name],U,d)
    out={'families':rec,'pair_overlaps':pairs,'heldout_basis_stability':held,'step':STEP,'seeds':SEEDS,'heldout_seed':hs}
    (OUT/'part12_result.json').write_text(json.dumps(out,indent=2)+'\n'); print(json.dumps(out,indent=2))
if __name__=='__main__':main()
