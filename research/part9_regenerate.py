#!/usr/bin/env python3
from pathlib import Path
import numpy as np
from part9_adversarial_renderer import Part9Renderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian

MODES=['subtle_waveshape_to_fm','subtle_modal_to_additive','smooth_accumulating_fm','smooth_accumulating_additive']
out=Path('research/data/part9_adversarial'); out.mkdir(parents=True,exist_ok=True)
t=np.linspace(0,1,41); seed=2718281
for mode in MODES:
    r=Part9Renderer(mode); feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(r.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(r,float(a),BASE_CONTROLS,seed))
    np.savez(out/f'{mode}.npz',t=t,sound_features=np.asarray(feats),jacobians=np.asarray(jacs),control_names=np.asarray(CONTROL_NAMES),feature_names=np.asarray(FEATURE_NAMES))
    print('Wrote',mode)
