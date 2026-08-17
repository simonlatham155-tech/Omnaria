#!/usr/bin/env python3
from dataclasses import dataclass
import numpy as np
from part8_persistence_renderer import SR, N, EPS, norm, sigmoid, waveshape, modal, fm


def matched_fm(t,c):
    # Lower-index FM chosen to remain tonally close to a waveshaped oscillator.
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0
    ratio=2.0+0.35*(harmonicity-0.5)
    idx=0.55+1.7*bright+0.35*density
    phase=2*np.pi*f0*t + idx*np.sin(2*np.pi*f0*ratio*t)
    return np.tanh((0.8+0.35*body)*np.sin(phase))


def matched_additive(t,c,a=0.0):
    # Harmonic envelope intentionally overlaps the modal spectrum.
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0; x=np.zeros_like(t)
    tilt=0.95+1.9*(1-bright)+0.15*a
    inh=(1-harmonicity)*(0.012+0.02*a)
    for k in range(1,22):
        ratio=k*(1+inh*(k-1)/21.0)
        amp=k**(-tilt)
        x += amp*np.sin(2*np.pi*f0*ratio*t+0.17*k)
    env=np.exp(-t/(0.18+0.9*body))
    return np.tanh(x*env)


def smooth_accumulating_fm(t,c,a):
    # One FM chart only. Alpha follows smooth nonlinear parameter trajectories.
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0
    p=a*a*(3-2*a)  # smoothstep; no discontinuity or engine switch
    q=p*p*(3-2*p)
    ratio=1.15+5.6*harmonicity+3.2*q
    idx=(0.25+7.5*bright)*(0.35+0.65*density)*(0.55+1.5*p)
    lfo=(0.08+0.9*movement)*(0.4+1.3*q)
    idx_t=idx*(1+0.22*movement*np.sin(2*np.pi*lfo*t))
    y=np.sin(2*np.pi*f0*t + idx_t*np.sin(2*np.pi*f0*ratio*t))
    y += 0.12*body*np.sin(4*np.pi*f0*t + 0.3*idx_t*np.sin(2*np.pi*f0*(ratio+0.5)*t))
    return np.tanh(y)


def smooth_accumulating_additive(t,c,a):
    # One additive chart only, with smooth nonlinear spectral drift.
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0; x=np.zeros_like(t)
    p=a*a*(3-2*a)
    q=p*p*(3-2*p)
    target=5.0+30.0*(0.25+0.75*density)*(0.35+0.65*q)
    tilt=0.65+2.8*(1-bright)+1.15*p
    inh=(1-harmonicity)*(0.001+0.024*q)
    for k in range(1,41):
        gate=sigmoid((target-k)/1.1)
        ratio=k*(1+inh*(k-1)/40.0)
        amp=gate*k**(-tilt)
        x += amp*np.sin(2*np.pi*f0*ratio*t+0.11*k)
    x *= 1+0.08*movement*np.sin(2*np.pi*(0.2+1.5*p)*t)
    return np.tanh((0.75+0.5*body)*x)


@dataclass
class Part9Renderer:
    mode: str
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        a=float(np.clip(alpha,0,1)); c=np.clip(np.asarray(controls,float),0,1)
        t=np.arange(N)/SR
        if self.mode=='subtle_waveshape_to_fm':
            w=float(sigmoid((a-0.27)/0.035))
            left=waveshape(t,c,0.0)
            right=matched_fm(t,c)
            return norm((1-w)*left+w*right)
        if self.mode=='subtle_modal_to_additive':
            w=float(sigmoid((a-0.81)/0.035))
            left=modal(t,c,0.0)
            right=matched_additive(t,c,0.0)
            return norm((1-w)*left+w*right)
        if self.mode=='smooth_accumulating_fm':
            return norm(smooth_accumulating_fm(t,c,a))
        if self.mode=='smooth_accumulating_additive':
            return norm(smooth_accumulating_additive(t,c,a))
        raise ValueError(self.mode)
