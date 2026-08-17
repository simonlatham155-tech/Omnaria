#!/usr/bin/env python3
from pathlib import Path
import numpy as np
from part8_persistence_renderer import Part8Renderer
from part6_regenerate_additive_granular import BASE_CONTROLS, CONTROL_NAMES, FEATURE_NAMES, extract_features, estimate_jacobian
MODES=['sigmoid_waveshape_modal','sigmoid_fm_filtered_noise','continuous_waveshape','continuous_modal']
out=Path('research/data/part8_persistence'); out.mkdir(parents=True,exist_ok=True)
t=np.linspace(0,1,41); seed=1414213
for mode in MODES:
    r=Part8Renderer(mode); feats=[]; jacs=[]
    for a in t:
        feats.append(extract_features(r.render(float(a),BASE_CONTROLS,seed)))
        jacs.append(estimate_jacobian(r,float(a),BASE_CONTROLS,seed))
    np.savez(out/f'{mode}.npz',t=t,sound_features=np.asarray(feats),jacobians=np.asarray(jacs),control_names=np.asarray(CONTROL_NAMES),feature_names=np.asarray(FEATURE_NAMES))
    print('Wrote',mode)
