#!/usr/bin/env python3
"""Score one Part-6 trace with the prospectively frozen rule.
Frozen rule from the three-pair in-sample candidate:
  S = 0.5 z(D_G) + 0.5 z(rotation) - 1.0 z(sound_step)
No fitting occurs in this script.
"""
from __future__ import annotations
import argparse, json
from pathlib import Path
import numpy as np
from part6_predictive_handoff_rule import load_trace, score_trace

FROZEN_WEIGHTS=(0.5,0.5,0.0,1.0)

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('trace',type=Path); ap.add_argument('--out',type=Path,default=Path('part6_frozen_rule_result.json'))
    args=ap.parse_args(); tr=load_trace(args.trace); s=score_trace(tr,FROZEN_WEIGHTS); i=int(np.argmax(s))
    result={'weights':{'D_G':0.5,'rotation':0.5,'log10_kappa':0.0,'sound_penalty':1.0},'predicted_handoff_midpoint':float(tr['t'][i]),'score':float(s[i]),'fitted_on_this_trace':False}
    args.out.write_text(json.dumps(result,indent=2)+'\n'); print(json.dumps(result,indent=2))
if __name__=='__main__': main()
