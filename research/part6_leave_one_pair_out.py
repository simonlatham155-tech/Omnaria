#!/usr/bin/env python3
"""Leave-one-pair-out validation for the Part-6 predictive handoff rule.

Fits compact geometry-only weights on N-1 paths, predicts the held-out path,
and repeats. No synthesis-family labels enter the score itself; labels are used
only to report which path was held out.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import numpy as np

from part6_predictive_handoff_rule import load_trace, grid_weights, predict


def fit(train_traces, train_targets):
    best=None
    for w in grid_weights():
        preds=[predict(tr,w)[0] for tr in train_traces]
        err=float(np.mean([(p-y)**2 for p,y in zip(preds,train_targets)]))
        complexity=sum(1 for x in w if x!=0)
        key=(err,complexity,sum(w))
        if best is None or key<best[0]:
            best=(key,w)
    return best[1]


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("traces",nargs="+",type=Path)
    ap.add_argument("--targets",nargs="+",type=float,required=True)
    ap.add_argument("--names",nargs="+")
    ap.add_argument("--out",type=Path,default=Path("part6_leave_one_pair_out.json"))
    args=ap.parse_args()
    n=len(args.traces)
    if len(args.targets)!=n: raise SystemExit("targets must match traces")
    names=args.names or [p.stem for p in args.traces]
    if len(names)!=n: raise SystemExit("names must match traces")
    traces=[load_trace(p) for p in args.traces]
    folds=[]
    for hold in range(n):
        train=[traces[i] for i in range(n) if i!=hold]
        ys=[args.targets[i] for i in range(n) if i!=hold]
        w=fit(train,ys)
        pred,score=predict(traces[hold],w)
        folds.append({
            "held_out":names[hold],
            "target":args.targets[hold],
            "prediction":pred,
            "abs_error":abs(pred-args.targets[hold]),
            "score":score,
            "weights":{"D_G":w[0],"rotation":w[1],"log10_kappa":w[2],"sound_penalty":w[3]},
        })
    result={
        "folds":folds,
        "mean_abs_error":float(np.mean([f["abs_error"] for f in folds])),
        "max_abs_error":float(np.max([f["abs_error"] for f in folds])),
        "family_labels_used_in_predictor":False,
        "criterion":"Strong evidence requires low held-out location error with compact, stable weights across folds."
    }
    args.out.write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result,indent=2))

if __name__=="__main__":
    main()
