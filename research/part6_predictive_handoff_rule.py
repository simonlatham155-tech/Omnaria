#!/usr/bin/env python3
"""Fit/test a family-agnostic predictive handoff rule for Part 6.

Inputs are one or more NPZ/JSON-derived traces containing per-step:
  metric_step, eigenspace_rotation_deg, condition_number, sound_step, midpoint_t

The rule intentionally ignores synthesis-family labels. It searches compact
weighted combinations of robust-normalised geometry terms and selects the
maximum score per path. Sound-step is optional and used only as a smoothness
penalty; the core predictor is D_G, rotation and log10(kappa).

This is a research utility, not evidence by itself.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import numpy as np

EPS = 1e-12


def robust_z(x):
    x = np.asarray(x, dtype=float)
    med = np.median(x)
    mad = 1.4826 * np.median(np.abs(x - med))
    std = np.std(x)
    scale = max(float(mad), 0.10 * float(std), EPS)
    return (x - med) / scale


def load_trace(path: Path):
    obj = json.loads(path.read_text())
    trace = obj.get("forward_trace", obj.get("trace", obj))
    rows = trace if isinstance(trace, list) else trace["rows"]
    t = [] ; dg=[] ; rot=[] ; kap=[] ; ds=[]
    prev_t = 0.0
    for r in rows:
        cur_t = float(r.get("t", r.get("candidate_t", 0.0)))
        mid = float(r.get("midpoint_t", (prev_t + cur_t) * 0.5))
        t.append(mid)
        dg.append(float(r["metric_step"]))
        rot.append(float(r["eigenspace_rotation_deg"]))
        kap.append(float(r["condition_number"]))
        ds.append(float(r.get("sound_step", 0.0)))
        prev_t = cur_t
    return {
        "t": np.asarray(t),
        "dg": np.asarray(dg),
        "rot": np.asarray(rot),
        "kap": np.asarray(kap),
        "ds": np.asarray(ds),
    }


def score_trace(tr, w):
    z_dg = robust_z(tr["dg"])
    z_rot = robust_z(tr["rot"])
    z_kap = robust_z(np.log10(np.maximum(tr["kap"], 1.0)))
    z_ds = robust_z(tr["ds"])
    return w[0]*z_dg + w[1]*z_rot + w[2]*z_kap - w[3]*z_ds


def predict(tr, w):
    s = score_trace(tr, w)
    i = int(np.argmax(s))
    return float(tr["t"][i]), float(s[i])


def grid_weights():
    vals = [0.0, 0.5, 1.0, 1.5, 2.0]
    for a in vals:
        for b in vals:
            for c in vals:
                if a+b+c == 0: continue
                for d in [0.0, 0.5, 1.0]:
                    yield (a,b,c,d)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("traces", nargs="+", type=Path)
    ap.add_argument("--targets", nargs="+", type=float, required=True,
                    help="observed handoff midpoints, one per trace")
    ap.add_argument("--out", type=Path, default=Path("part6_predictive_rule_result.json"))
    args = ap.parse_args()
    if len(args.traces) != len(args.targets):
        raise SystemExit("traces and targets must have same length")

    traces = [load_trace(p) for p in args.traces]
    best = None
    for w in grid_weights():
        preds = [predict(tr, w)[0] for tr in traces]
        err = float(np.mean([(p-y)**2 for p,y in zip(preds,args.targets)]))
        complexity = sum(1 for x in w if x != 0)
        key = (err, complexity, sum(w))
        if best is None or key < best[0]:
            best = (key, w, preds)

    _, w, preds = best
    result = {
        "weights": {"D_G":w[0], "rotation":w[1], "log10_kappa":w[2], "sound_penalty":w[3]},
        "targets": args.targets,
        "predictions": preds,
        "mean_squared_location_error": float(np.mean([(p-y)**2 for p,y in zip(preds,args.targets)])),
        "family_labels_used": False,
        "note": "In-sample compact-rule search only; hold-out validation is required before interpreting as a law."
    }
    args.out.write_text(json.dumps(result, indent=2)+"\n")
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()
