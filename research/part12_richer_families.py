#!/usr/bin/env python3
from __future__ import annotations
from dataclasses import dataclass
import numpy as np
SR=48000; N=SR; EPS=1e-12

def norm(x):
    x=np.asarray(x,float); x=x-np.mean(x); return 0.8*x/(np.max(np.abs(x))+EPS)

def tvec(): return np.arange(N)/SR

@dataclass
class RichGranular:
    names=tuple(['grain_ms','density','spray','pitch_spread','scan_rate','window_shape','feedback','grain_filter'])
    def render(self,p,seed):
        rng=np.random.default_rng(seed); t=tvec(); p=np.clip(np.asarray(p,float),0,1)
        grain_ms,density,spray,pitch_spread,scan_rate,window_shape,feedback,grain_filter=p
        src=np.sin(2*np.pi*110*t)+0.35*np.sin(2*np.pi*221*t)+0.18*np.sin(2*np.pi*333*t)
        src += 0.08*np.sin(2*np.pi*(40+120*scan_rate)*t)*src
        out=np.zeros_like(t); gl=max(64,int((0.006+0.12*grain_ms)*SR)); events=int(20+220*density)
        fb=np.zeros_like(t)
        for _ in range(events):
            o=int(rng.integers(0,max(1,N-gl))); s=int(rng.integers(0,max(1,N-gl)))
            ratio=2**((rng.normal()*12*pitch_spread)/12); idx=np.clip((s+np.arange(gl)*ratio).astype(int),0,N-1)
            w=np.hanning(gl)**(0.6+2.4*window_shape)
            g=src[idx]*w
            if grain_filter>0: g=np.convolve(g,np.ones(3+int(30*(1-grain_filter)))/(3+int(30*(1-grain_filter))),mode='same')
            shift=int((rng.normal()*spray*0.08)*SR); oo=np.clip(o+shift,0,max(0,N-gl))
            out[oo:oo+gl]+=g
        if feedback>0:
            d=max(1,int((0.008+0.05*feedback)*SR)); fb[d:]=out[:-d]*feedback*0.55; out+=fb
        return norm(out)

@dataclass
class RichPhysical:
    names=tuple(['stiffness','damping','pickup','excitation','inharmonicity','body_coupling','dispersion','nonlinearity'])
    def render(self,p,seed):
        rng=np.random.default_rng(seed); t=tvec(); p=np.clip(np.asarray(p,float),0,1)
        stiffness,damping,pickup,excitation,inharm,body,dispersion,nonlin=p
        x=np.zeros_like(t); f0=90+40*pickup
        for k in range(1,28):
            ratio=k*np.sqrt(1+0.004*stiffness*k*k)+(inharm*0.015)*(k**1.35)
            decay=(0.08+1.8*(1-damping))/(1+0.035*k*k)
            amp=(k**-(0.7+1.7*(1-excitation)))*(0.7+0.3*np.cos(np.pi*k*pickup)**2)
            phase=0.07*k+0.15*dispersion*k*k
            x += amp*np.exp(-t/max(decay,0.02))*np.sin(2*np.pi*f0*ratio*t+phase)
        x += body*0.35*np.exp(-t/(0.4+body))*np.sin(2*np.pi*(180+240*body)*t)
        x += 0.02*excitation*rng.normal(size=N)*np.exp(-t/0.03)
        return norm(np.tanh((1+5*nonlin)*x))

@dataclass
class RichSpectral:
    names=tuple(['centroid','tilt','formant','smear','phase_rand','noise_mix','transient','partial_warp'])
    def render(self,p,seed):
        rng=np.random.default_rng(seed); t=tvec(); p=np.clip(np.asarray(p,float),0,1)
        centroid,tilt,formant,smear,phase_rand,noise_mix,transient,warp=p
        x=np.zeros_like(t); f0=100
        for k in range(1,48):
            fk=f0*k*(1+warp*0.002*k)
            amp=k**(-(0.5+2.8*tilt))
            target=300+2600*formant; bw=120+900*smear
            amp*=0.45+1.7*np.exp(-0.5*((fk-target)/bw)**2)
            amp*=1/(1+np.exp((fk-(600+7000*centroid))/500))
            ph=(1-phase_rand)*0.13*k+phase_rand*rng.uniform(0,2*np.pi)
            x += amp*np.sin(2*np.pi*fk*t+ph)
        if noise_mix>0: x += noise_mix*0.25*rng.normal(size=N)
        env=(1-transient)+transient*np.exp(-t/(0.01+0.18*(1-transient)))
        return norm(x*env)

@dataclass
class RichChaos:
    names=tuple(['drive','feedback','fold','fm_depth','fm_ratio','memory','bias','noise'])
    def render(self,p,seed):
        rng=np.random.default_rng(seed); p=np.clip(np.asarray(p,float),0,1); t=tvec()
        drive,feedback,fold,fm_depth,fm_ratio,memory,bias,noise=p
        f0=70+80*fm_ratio; phase=2*np.pi*f0*t + (0.2+8*fm_depth)*np.sin(2*np.pi*f0*(0.5+5*fm_ratio)*t)
        s=np.sin(phase); y=np.zeros(N); state=0.0
        a=0.02+0.96*memory
        for i in range(N):
            inp=(1+10*drive)*s[i] + feedback*2.6*state + bias*0.8 + noise*0.08*rng.normal()
            z=np.tanh(inp)
            z=np.sin((1+8*fold)*z)
            state=a*state+(1-a)*z
            y[i]=state
        return norm(y)

FAMILIES={'granular':RichGranular(),'physical':RichPhysical(),'spectral':RichSpectral(),'chaos':RichChaos()}
BASE={k:np.array([0.45,0.55,0.35,0.4,0.5,0.3,0.25,0.45],float) for k in FAMILIES}
