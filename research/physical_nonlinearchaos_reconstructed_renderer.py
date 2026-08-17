#!/usr/bin/env python3
"""Physical→NonlinearChaos reconstructed research renderer for Part 6.

Uses the same frozen handoff schedule established earlier:
  mix = alpha ** 1.6
  coherence = (1 - alpha) ** 0.65

Only endpoint mechanisms differ. No pair-specific transition timing is fitted.
Research model only; not recovered production code.
"""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np

SAMPLE_RATE = 48_000
N = SAMPLE_RATE
EPS = 1e-12


def _norm(x):
    x=np.asarray(x,float); x-=x.mean(); return 0.8*x/(np.max(np.abs(x))+EPS)

@dataclass
class PhysicalNonlinearChaosRenderer:
    def render(self, alpha: float, controls: np.ndarray, seed: int) -> np.ndarray:
        rng=np.random.default_rng(seed)
        c=np.clip(np.asarray(controls,float),0,1)
        bright,body,movement,density,transient,grain,scatter,harmonicity=c
        a=float(np.clip(alpha,0,1)); mix=a**1.6; coherence=(1-a)**0.65
        t=np.arange(N)/SAMPLE_RATE

        # Physical endpoint: damped resonant body excited by sparse impulses.
        freqs=np.array([110.,181.,296.,421.,673.])*(0.92+0.16*harmonicity)
        decays=np.array([1.8,1.2,0.85,0.62,0.42])*(0.55+1.25*body)
        modes=np.zeros(N)
        exc=np.zeros(N)
        pulse_count=3
        pulse_pos=np.linspace(0.05,0.82,pulse_count)
        for p in pulse_pos:
            i=int(p*N); exc[i:min(N,i+8)] += np.hanning(min(8,N-i))* (0.6+0.4*transient)
        for f,d in zip(freqs,decays):
            env=np.exp(-t/d)
            phase=2*np.pi*f*t + 0.05*movement*np.sin(2*np.pi*(0.2+movement)*t)
            modes += env*np.sin(phase)
        physical=(0.4+0.6*body)*modes*0.22 + exc

        # Chaos endpoint: deterministic logistic-state modulation drives phase,
        # amplitude and resonant frequency while retaining shared controls.
        r=3.57 + 0.42*(0.45*density+0.35*movement+0.20*scatter)
        x=np.empty(N); x[0]=0.231 + 0.05*rng.random()
        for i in range(1,N):
            x[i]=r*x[i-1]*(1-x[i-1])
        chaos=(x-0.5)*2.0
        smooth=np.convolve(chaos, np.ones(31)/31.0, mode='same')
        inst_f=110*(1 + (0.01+0.18*movement)*smooth)
        phase=np.cumsum(2*np.pi*inst_f/SAMPLE_RATE)
        nonlinear=np.sin(phase)
        nonlinear += (0.15+0.35*bright)*np.sin(2.03*phase + scatter*chaos)
        nonlinear *= 0.75 + 0.25*np.tanh((1+4*body)*chaos)
        nonlinear += transient*0.08*np.diff(np.r_[chaos[0],chaos])

        # Frozen handoff law. Coherent physical state gives way to nonlinear field.
        xout=coherence*physical + (1-coherence)*((1-mix)*physical + mix*nonlinear)
        xout *= 1 + 0.05*movement*np.sin(2*np.pi*(0.21+2.3*movement)*t)
        return _norm(np.tanh((1.05+0.7*body)*xout))
