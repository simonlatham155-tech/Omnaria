#!/usr/bin/env python3
"""Independent baseline synthesis systems for Part 7 external falsification.

These are intentionally not built from the reconstructed OMNARIA handoff
architecture. Positive controls use externally imposed sigmoid transition
centres; negative controls remain within one continuously varying algorithm.
"""
from dataclasses import dataclass
import numpy as np

SR=48_000
N=SR
EPS=1e-12


def _norm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+EPS)

def _sigmoid(x):
    return 1.0/(1.0+np.exp(-np.clip(x,-60,60)))

def _additive(t,c,rng,alpha=0.0):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    x=np.zeros_like(t); f0=110.0
    phases=rng.uniform(0,2*np.pi,40)
    target=6+28*density
    for k in range(1,41):
        gate=_sigmoid((target-k)/1.2)
        tilt=0.8+2.4*(1-bright)
        detune=(1-harmonicity)*(0.001+0.006*alpha)*rng.normal()
        phase=2*np.pi*f0*k*(1+detune)*t + phases[k-1]
        x += gate*(k**-tilt)*np.sin(phase)
    x *= 0.6+0.5*body
    x *= 1+0.05*movement*np.sin(2*np.pi*(0.2+1.7*movement)*t)
    return np.tanh(x)

def _fm(t,c,alpha=0.0):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0
    ratio=1.3+4.7*harmonicity+0.3*alpha
    mod=2*np.pi*f0*ratio*t
    index=(0.4+7.0*bright)*(0.4+0.6*density)*(0.85+0.3*alpha)
    index*=1+0.16*movement*np.sin(2*np.pi*(0.23+movement)*t)
    x=np.sin(2*np.pi*f0*t + index*np.sin(mod))
    x += 0.18*body*np.sin(4*np.pi*f0*t + 0.5*np.sin(mod))
    return np.tanh((1+0.8*body)*x)

def _modal(t,c,rng):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0; x=np.zeros_like(t)
    phases=rng.uniform(0,2*np.pi,24)
    for k in range(1,25):
        ratio=k+(1-harmonicity)*0.11*(k**1.18)
        decay=(0.12+1.2*body)/(1+0.06*k)
        amp=(k**(-(0.8+2.1*(1-bright))))
        x += amp*np.exp(-t/max(0.03,decay))*np.sin(2*np.pi*f0*ratio*t+phases[k-1])
    return np.tanh(x)

def _noise(t,c,rng):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    n=rng.normal(size=len(t))
    kernel_len=5+int(80*(1-bright))
    kernel=np.ones(kernel_len)/kernel_len
    smooth=np.convolve(n,kernel,mode='same')
    env=0.65+0.35*np.sin(2*np.pi*(0.15+2.0*movement)*t)**2
    bursts=np.diff(np.r_[0.0,smooth])*transient
    return np.tanh((0.6+0.8*body)*(smooth*env+0.35*bursts))

@dataclass
class ExternalBaselineRenderer:
    mode: str

    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        a=float(np.clip(alpha,0,1)); c=np.clip(np.asarray(controls,float),0,1)
        t=np.arange(N)/SR
        rng=np.random.default_rng(seed)

        if self.mode=='independent_sigmoid_additive_fm':
            w=float(_sigmoid((a-0.68)/0.035))
            left=_additive(t,c,np.random.default_rng(seed+11))
            right=_fm(t,c)
            return _norm((1-w)*left+w*right)

        if self.mode=='independent_sigmoid_modal_noise':
            w=float(_sigmoid((a-0.43)/0.035))
            left=_modal(t,c,np.random.default_rng(seed+22))
            right=_noise(t,c,np.random.default_rng(seed+33))
            return _norm((1-w)*left+w*right)

        if self.mode=='single_engine_continuous_fm':
            # No chart/engine switch: alpha only changes parameters of one FM system.
            return _norm(_fm(t,c,a))

        if self.mode=='single_engine_continuous_additive':
            # No chart/engine switch: alpha only changes detuning continuously.
            return _norm(_additive(t,c,rng,a))

        raise ValueError(f'Unknown mode: {self.mode}')
