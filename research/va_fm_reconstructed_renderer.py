#!/usr/bin/env python3
"""VA→FM reconstructed renderer for prospective Part-6 testing.

Research model only; not recovered production code.
The previously frozen handoff schedule is retained unchanged:
  mix = alpha ** 1.6
  coherence = (1 - alpha) ** 0.65

No predictive-rule weights are touched here. This file defines only endpoint
mechanics in the shared control space.
"""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np

SAMPLE_RATE=48_000
N=SAMPLE_RATE
EPS=1e-12


def _norm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+EPS)


def _saw(phase):
    return 2.0*((phase/(2*np.pi))%1.0)-1.0


@dataclass
class VAFMRenderer:
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng=np.random.default_rng(seed)
        bright,body,movement,density,transient,grain,scatter,harmonicity=np.clip(np.asarray(controls,float),0,1)
        a=float(np.clip(alpha,0,1)); mix=a**1.6; coherence=(1-a)**0.65
        t=np.arange(N)/SAMPLE_RATE
        f0=110.0

        # VA endpoint: band-limited-ish harmonic oscillator field plus smooth
        # filter-like spectral weighting. Partial activation stays continuous.
        max_h=48
        va=np.zeros(N)
        target_h=4.0+36.0*(0.35+0.65*bright)
        phases=rng.uniform(0,2*np.pi,max_h)
        for k in range(1,max_h+1):
            gate=1/(1+np.exp((k-target_h)/1.4))
            tilt=0.7+2.2*(1-bright)
            amp=gate*(k**-tilt)
            det=(1-harmonicity)*0.0025*rng.normal()
            phi=2*np.pi*f0*k*(1+det)*t + phases[k-1]
            va += amp*np.sin(phi)
        env=1+0.06*movement*np.sin(2*np.pi*(0.18+1.8*movement)*t)
        va*=env*(0.65+0.5*body)
        va=np.tanh((1.0+1.8*body)*va)

        # FM endpoint: one carrier/modulator structure. Shared controls map to
        # ratio, index and slow modulation without introducing a hard switch.
        ratio=1.0 + 5.0*(0.25+0.75*harmonicity)
        mod_f=f0*ratio*(1+0.03*scatter*np.sin(2*np.pi*(0.11+movement)*t))
        mod_phase=np.cumsum(2*np.pi*mod_f/SAMPLE_RATE)
        index=(0.25+8.0*bright)*(0.35+0.65*density)
        index*=1+0.20*movement*np.sin(2*np.pi*(0.23+1.5*movement)*t)
        carrier_phase=2*np.pi*f0*t + index*np.sin(mod_phase)
        fm=np.sin(carrier_phase)
        fm += 0.22*body*np.sin(2*carrier_phase + 0.7*np.sin(mod_phase))
        fm += 0.06*transient*np.diff(np.r_[0.0, np.sin(mod_phase)])
        fm=np.tanh((1.0+1.1*body)*fm)

        # Frozen shared schedule; no pair-specific handoff timing.
        x=coherence*va + (1-coherence)*((1-mix)*va + mix*fm)
        x*=1+0.04*movement*np.sin(2*np.pi*(0.19+2.2*movement)*t)
        return _norm(x)
