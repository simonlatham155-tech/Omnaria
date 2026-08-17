#!/usr/bin/env python3
from dataclasses import dataclass
import numpy as np

SR=48_000
N=SR
EPS=1e-12

def norm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+EPS)

def sigmoid(x):
    return 1.0/(1.0+np.exp(-np.clip(x,-60,60)))

def waveshape(t,c,a=0.0):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0
    drive=1.0+7.0*bright+2.0*a
    phase=2*np.pi*f0*t + 0.08*movement*np.sin(2*np.pi*(0.3+movement)*t)
    base=np.sin(phase)+0.22*body*np.sin(2*phase)
    return np.tanh(drive*base)/(np.tanh(drive)+EPS)

def modal(t,c,a=0.0):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0; x=np.zeros_like(t)
    for k in range(1,22):
        ratio=k+(1-harmonicity)*(0.04+0.09*a)*(k**1.12)
        decay=(0.16+1.0*body)/(1+0.07*k)
        amp=k**(-(0.9+2.0*(1-bright)))
        x += amp*np.exp(-t/max(decay,0.03))*np.sin(2*np.pi*f0*ratio*t+0.17*k)
    return np.tanh(x)

def fm(t,c,a=0.0):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    f0=110.0; ratio=1.4+4.2*harmonicity+0.4*a
    idx=(0.3+6.5*bright)*(0.5+0.5*density)*(0.8+0.4*a)
    return np.sin(2*np.pi*f0*t + idx*np.sin(2*np.pi*f0*ratio*t))

def filtered_noise(t,c,a,seed):
    bright,body,movement,density,transient,grain,scatter,harmonicity=c
    rng=np.random.default_rng(seed)
    n=rng.normal(size=len(t))
    width=max(3,int(10+90*(1-bright)*(1-0.5*a)))
    k=np.ones(width)/width
    y=np.convolve(n,k,mode='same')
    env=0.65+0.35*np.sin(2*np.pi*(0.2+1.5*movement)*t)**2
    return y*env

@dataclass
class Part8Renderer:
    mode: str
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        a=float(np.clip(alpha,0,1)); c=np.clip(np.asarray(controls,float),0,1)
        t=np.arange(N)/SR
        if self.mode=='sigmoid_waveshape_modal':
            w=float(sigmoid((a-0.31)/0.035))
            return norm((1-w)*waveshape(t,c,0.0)+w*modal(t,c,0.0))
        if self.mode=='sigmoid_fm_filtered_noise':
            w=float(sigmoid((a-0.74)/0.035))
            return norm((1-w)*fm(t,c,0.0)+w*filtered_noise(t,c,0.0,seed+77))
        if self.mode=='continuous_waveshape':
            return norm(waveshape(t,c,a))
        if self.mode=='continuous_modal':
            return norm(modal(t,c,a))
        raise ValueError(self.mode)
