#!/usr/bin/env python3
"""Cross-family prospective renderer using only previously committed endpoints.

No endpoint mechanism is redefined here. Each family is obtained from an
existing research renderer evaluated exactly at that family's endpoint. The
shared transition schedule is frozen:
    mix = alpha ** 1.6
    coherence = (1 - alpha) ** 0.65
Research model only; not recovered production code.
"""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np

from omnaria_reconstructed_renderer_v2 import ReconstructedOmnariaRendererV2
from modal_waveterrain_reconstructed_renderer import ModalWaveTerrainRenderer
from va_fm_reconstructed_renderer import VAFMRenderer
from physical_nonlinearchaos_reconstructed_renderer import PhysicalNonlinearChaosRenderer

EPS=1e-12


def _norm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+EPS)


@dataclass
class CrossFamilyRenderer:
    pair: str

    def __post_init__(self):
        self.ag=ReconstructedOmnariaRendererV2()
        self.mw=ModalWaveTerrainRenderer()
        self.vf=VAFMRenderer()
        self.pc=PhysicalNonlinearChaosRenderer()
        allowed={"additive_modal","granular_fm","waveterrain_nonlinearchaos"}
        if self.pair not in allowed:
            raise ValueError(f"Unknown prospective pair {self.pair}")

    def _endpoints(self, controls: np.ndarray, seed: int):
        if self.pair=="additive_modal":
            src=self.ag.render(0.0, controls, seed)
            tgt=self.mw.render(0.0, controls, seed)
        elif self.pair=="granular_fm":
            src=self.ag.render(1.0, controls, seed)
            tgt=self.vf.render(1.0, controls, seed)
        else:
            src=self.mw.render(1.0, controls, seed)
            tgt=self.pc.render(1.0, controls, seed)
        return src,tgt

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        a=float(np.clip(alpha,0.0,1.0))
        mix=a**1.6
        coherence=(1.0-a)**0.65
        src,tgt=self._endpoints(controls,seed)
        x=coherence*src + (1.0-coherence)*((1.0-mix)*src + mix*tgt)
        return _norm(x)
