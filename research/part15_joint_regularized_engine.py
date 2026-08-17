#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np
from part12_richer_families import FAMILIES, BASE, norm
from part6_regenerate_additive_granular import extract_features
from part14_variable_dim_engine import MAP as MAP14

OUT=Path('research/data/part15'); OUT.mkdir(parents=True,exist_ok=True)
FAMS=['granular','physical','spectral','chaos']
TRAIN_SEEDS=[2718281,1618033]
TEST_SEED=1732051
STEP_NATIVE=.02; STEP_Z=.025
ALPHAS=np.linspace(0,1,41)
Z0=np.zeros(4)


def smoothstep(x):
    x=np.clip(x,0,1); return x*x*(3-2*x)

def feat(name,p,seed):
    return extract_features(FAMILIES[name].render(np.clip(p,.06,.94),seed))

def avg_feat(name,p):
    return np.mean([feat(name,p,s) for s in TRAIN_SEEDS],axis=0)

def native_jac(name,p,seed):
    p=np.asarray(p,float); f0=feat(name,p,seed); J=np.zeros((len(f0),8))
    for j in range(8):
        lo=p.copy(); hi=p.copy(); lo[j]=max(.06,p[j]-STEP_NATIVE); hi[j]=min(.94,p[j]+STEP_NATIVE)
        J[:,j]=(feat(name,hi,seed)-feat(name,lo,seed))/(hi[j]-lo[j])
    return J

def fit_anchors():
    # Joint acoustic-path regularization proxy: gently move each family anchor toward
    # the training-feature centroid, with hard displacement/ridge limits so family identity is retained.
    anchors={k:BASE[k].copy() for k in FAMS}
    centroid=np.mean([avg_feat(k,BASE[k]) for k in FAMS],axis=0)
    basef={k:avg_feat(k,BASE[k]) for k in FAMS}
    def loss(name,p):
        f=avg_feat(name,p)
        acoustic=np.sum((f-centroid)**2)
        identity=.35*np.sum((f-basef[name])**2)
        ridge=8.0*np.sum((p-BASE[name])**2)
        return acoustic+identity+ridge
    for name in FAMS:
        p=anchors[name].copy()
        for delta in [.035,.02,.01]:
            for _ in range(2):
                for j in range(8):
                    cands=[p.copy(),p.copy(),p.copy()]
                    cands[1][j]-=delta; cands[2][j]+=delta
                    cands=[np.clip(c,.06,.94) for c in cands]
                    # hard total displacement limit
                    cands=[BASE[name]+np.clip(c-BASE[name],-.08,.08) for c in cands]
                    vals=[loss(name,c) for c in cands]; p=cands[int(np.argmin(vals))]
        anchors[name]=p
    return anchors

def fit_maps(anchors):
    # Build a shared acoustic target basis from training Jacobians.
    Javg={}
    for name in FAMS:
        Javg[name]=np.mean([native_jac(name,anchors[name],s) for s in TRAIN_SEEDS],axis=0)
    U,_,_=np.linalg.svd(np.concatenate([Javg[k] for k in FAMS],axis=1),full_matrices=False)
    common=U[:,:4]
    maps={}
    for name in FAMS:
        Uf,_,_=np.linalg.svd(Javg[name],full_matrices=False)
        target=np.column_stack([common[:,0],common[:,1],Uf[:,0],Uf[:,1]])
        J=Javg[name]; lam=.08
        M=np.linalg.solve(J.T@J+lam*np.eye(8),J.T@target)
        # normalize native movement per global direction and keep it conservative
        for j in range(4):
            m=np.max(np.abs(M[:,j]))+1e-12
            M[:,j]*=(.075/m)
        maps[name]=M
    return maps

def params15(name,z,w,anchors,maps):
    zz=np.asarray(z,float).copy(); zz[2:]*=(.20+.80*w)
    p=anchors[name]+maps[name]@np.tanh(1.15*zz)
    return np.clip(p,.06,.94)

def params14(name,z,w):
    zz=np.asarray(z,float).copy(); zz[2:]*=(.25+.75*w)
    return np.clip(BASE[name]+MAP14[name]@np.tanh(1.3*zz),.08,.92)

def render_weighted(weights,z,seed,param_fn):
    w=np.maximum(np.asarray(weights,float),0); w=w/(np.sum(w)+1e-12)
    g=np.sqrt(w); g=g/(np.sqrt(np.sum(g*g))+1e-12)
    y=0.0
    for i,name in enumerate(FAMS):
        if g[i]<1e-9: continue
        y=y+g[i]*FAMILIES[name].render(param_fn(name,z,w[i]),seed+97*i)
    return norm(y)

def render_crossfade(weights,seed):
    w=np.maximum(np.asarray(weights,float),0); w=w/(np.sum(w)+1e-12)
    g=np.sqrt(w); g=g/(np.sqrt(np.sum(g*g))+1e-12)
    y=0.0
    for i,name in enumerate(FAMS):
        if g[i]<1e-9: continue
        y=y+g[i]*FAMILIES[name].render(BASE[name],seed+97*i)
    return norm(y)

def route_weights(route,a):
    s=smoothstep(a); w=np.zeros(4)
    if route=='granular-chaos':
        w[0]=1-s; w[3]=s
    elif route=='physical-chaos':
        w[1]=1-s; w[3]=s
    elif route=='granular-spectral-via-physical':
        mid=4*s*(1-s)*.35; rem=1-mid; w[0]=rem*(1-s); w[2]=rem*s; w[1]=mid
    elif route=='physical-granular-via-spectral':
        mid=4*s*(1-s)*.35; rem=1-mid; w[1]=rem*(1-s); w[0]=rem*s; w[2]=mid
    else: raise ValueError(route)
    return w/(np.sum(w)+1e-12)

def jac_z(weights,z,seed,param_fn):
    f0=extract_features(render_weighted(weights,z,seed,param_fn)); J=np.zeros((len(f0),4))
    for j in range(4):
        lo=z.copy(); hi=z.copy(); lo[j]-=STEP_Z; hi[j]+=STEP_Z
        J[:,j]=(extract_features(render_weighted(weights,hi,seed,param_fn))-extract_features(render_weighted(weights,lo,seed,param_fn)))/(2*STEP_Z)
    return J

def effdim(J,q=.95):
    s=np.linalg.svd(J,compute_uv=False); e=s*s; c=np.cumsum(e)/(np.sum(e)+1e-15); return int(np.searchsorted(c,q)+1)

def overlap(a,b):
    na=np.linalg.norm(a); nb=np.linalg.norm(b)
    if na<1e-12 or nb<1e-12:return 1.0
    return float(abs(np.dot(a,b)/(na*nb)))

def continuity(F):
    F=np.asarray(F); st=np.linalg.norm(np.diff(F,axis=0),axis=1); med=float(np.median(st)+1e-12)
    return {'p95':float(np.percentile(st,95)),'max_ratio':float(np.max(st)/med),'median':med}

def eval_route(route,anchors,maps):
    p15=lambda n,z,w:params15(n,z,w,anchors,maps)
    p14=lambda n,z,w:params14(n,z,w)
    f15=[]; f14=[]; fcf=[]; Js=[]; ds=[]
    for a in ALPHAS:
        w=route_weights(route,a)
        f15.append(extract_features(render_weighted(w,Z0,TEST_SEED,p15)))
        f14.append(extract_features(render_weighted(w,Z0,TEST_SEED,p14)))
        fcf.append(extract_features(render_crossfade(w,TEST_SEED)))
        J=jac_z(w,Z0,TEST_SEED,p15); Js.append(J); ds.append(effdim(J))
    c15,c14,ccf=continuity(f15),continuity(f14),continuity(fcf)
    ctrl=[]
    for j in range(4):
        mags=np.array([np.linalg.norm(J[:,j]) for J in Js]); mx=float(np.max(mags)+1e-12); ovs=[]
        for i in range(len(Js)-1):
            if min(mags[i],mags[i+1])>=.1*mx: ovs.append(overlap(Js[i][:,j],Js[i+1][:,j]))
        ctrl.append(float(np.mean(np.array(ovs)>=.7)) if ovs else 1.0)
    best=min(c14['p95'],ccf['p95'])
    audio_better=c15['p95']<=best+1e-12
    max_ok=c15['max_ratio']<=3.0
    control_ok=all(x>=.90 for x in ctrl)
    dim_ok=len(set(ds))>1
    route_pass=audio_better and max_ok and control_ok and dim_ok
    return {'route':route,'part15':c15,'part14':c14,'crossfade':ccf,'control_overlap_fractions':ctrl,'d95':ds,
            'audio_better':audio_better,'max_ok':max_ok,'control_ok':control_ok,'dim_adapt':dim_ok,'route_pass':route_pass}

def main():
    anchors=fit_anchors(); maps=fit_maps(anchors)
    routes=['granular-chaos','physical-chaos','granular-spectral-via-physical','physical-granular-via-spectral']
    rows=[eval_route(r,anchors,maps) for r in routes]
    pass_count=sum(int(r['route_pass']) for r in rows)
    chaos_ok=any(r['route_pass'] and 'chaos' in r['route'] for r in rows)
    audio_count=sum(int(r['audio_better'] and r['max_ok']) for r in rows)
    control_count=sum(int(r['control_ok']) for r in rows)
    dim_count=sum(int(r['dim_adapt']) for r in rows)
    out={'training_seeds':TRAIN_SEEDS,'heldout_seed':TEST_SEED,'anchors':{k:v.tolist() for k,v in anchors.items()},
         'maps':{k:v.tolist() for k,v in maps.items()},'routes':rows,'audio_route_count':audio_count,
         'control_route_count':control_count,'dim_adapt_route_count':dim_count,'route_passes':pass_count,
         'chaos_route_pass':chaos_ok,'overall_pass':pass_count>=3 and chaos_ok,'criteria':'Part 15 preregistration issue #16'}
    (OUT/'part15_result.json').write_text(json.dumps(out,indent=2)+'\n')
    print(json.dumps(out,indent=2))
if __name__=='__main__': main()
