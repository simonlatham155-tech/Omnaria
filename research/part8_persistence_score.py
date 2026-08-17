#!/usr/bin/env python3
import argparse, json
from pathlib import Path
import numpy as np

EPS=1e-12

def metrics(js):
    return np.asarray([j.T@j for j in js])

def persistence(t,js,half=3):
    g=metrics(js)
    step=np.linalg.norm(np.diff(g,axis=0),axis=(1,2))
    base=float(np.median(step))+EPS
    vals=[]; centers=[]
    for i in range(half,len(g)-half):
        before=g[i-half:i].mean(axis=0)
        after=g[i:i+half].mean(axis=0)
        vals.append(float(np.linalg.norm(after-before)/base))
        centers.append(float(t[i]))
    vals=np.asarray(vals); centers=np.asarray(centers)
    k=int(np.argmax(vals))
    return {'peak_t':float(centers[k]),'peak_ratio':float(vals[k]),'median_step':base}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('npz',type=Path); ap.add_argument('--out',type=Path,required=True)
    args=ap.parse_args(); z=np.load(args.npz); out=persistence(z['t'],z['jacobians'])
    args.out.write_text(json.dumps(out,indent=2)+'\n'); print(json.dumps(out,indent=2))

if __name__=='__main__': main()
