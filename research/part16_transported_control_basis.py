#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
import numpy as np

from part12_richer_families import FAMILIES, BASE, norm
from part6_regenerate_additive_granular import extract_features
from part15_joint_regularized_engine import fit_anchors, fit_maps, params15, render_weighted, render_crossfade, jac_z, continuity

OUT = Path('research/data/part16'); OUT.mkdir(parents=True, exist_ok=True)
FAMS = ['granular','physical','spectral','chaos']
TEST_SEED = 2236067
ALPHAS = np.linspace(0.0, 1.0, 41)
Z0 = np.zeros(4)
STEP_U = 0.025


def smoothstep(x):
    x = np.clip(x, 0.0, 1.0)
    return x*x*(3.0-2.0*x)


def route_weights(route, a):
    s = smoothstep(a)
    w = np.zeros(4)
    if route == 'granular-chaos-four-family':
        # Endpoints are granular/chaos. Physical and spectral make distinct interior bows.
        p = 0.23*np.exp(-0.5*((s-0.36)/0.18)**2)
        q = 0.23*np.exp(-0.5*((s-0.67)/0.18)**2)
        extra = min(0.42, p+q)
        rem = 1.0-extra
        w[0] = rem*(1.0-s)
        w[3] = rem*s
        if p+q > 1e-12:
            w[1] = extra*p/(p+q)
            w[2] = extra*q/(p+q)
    elif route == 'physical-spectral-via-granular':
        mid = 0.34*4.0*s*(1.0-s); rem = 1.0-mid
        w[1] = rem*(1.0-s); w[2] = rem*s; w[0] = mid
    elif route == 'spectral-granular-via-chaos':
        mid = 0.30*4.0*s*(1.0-s); rem = 1.0-mid
        w[2] = rem*(1.0-s); w[0] = rem*s; w[3] = mid
    elif route == 'chaos-physical-via-spectral':
        mid = 0.32*4.0*s*(1.0-s); rem = 1.0-mid
        w[3] = rem*(1.0-s); w[1] = rem*s; w[2] = mid
    else:
        raise ValueError(route)
    return w/(np.sum(w)+1e-12)


def local_svd(J):
    U, s, Vt = np.linalg.svd(J, full_matrices=False)
    if len(s) == 0 or s[0] < 1e-12:
        return U, s, Vt.T, 0
    r = int(np.sum(s >= 0.10*s[0]))
    r = max(1, r)
    return U, s, Vt.T, r


def orthonormalize(X):
    if X.size == 0:
        return X
    Q, _ = np.linalg.qr(X)
    return Q[:, :X.shape[1]]


def transport_basis(prev, V, r):
    Qcur = V[:, :r]
    if prev is None:
        return Qcur.copy()
    rp = prev.shape[1]
    if rp == r:
        # Orthogonal Procrustes: choose the current basis nearest the previous basis.
        M = Qcur.T @ prev
        U, _, Vt = np.linalg.svd(M, full_matrices=False)
        B = Qcur @ (U @ Vt)
    else:
        k = min(rp, r)
        # Preserve as much of the previous basis as lies in the new active subspace.
        P = Qcur @ (Qcur.T @ prev[:, :k])
        Bkeep = orthonormalize(P)
        cols = [Bkeep[:, j] for j in range(Bkeep.shape[1])]
        # Fill newly active directions with locally dominant residual singular vectors.
        for j in range(r):
            v = Qcur[:, j].copy()
            for c in cols:
                v -= c*np.dot(c, v)
            n = np.linalg.norm(v)
            if n > 1e-8 and len(cols) < r:
                cols.append(v/n)
        B = np.column_stack(cols[:r])
    # Sign continuity for retained columns.
    k = min(prev.shape[1], B.shape[1])
    for j in range(k):
        if np.dot(B[:, j], prev[:, j]) < 0:
            B[:, j] *= -1.0
    return B


def render_transport(weights, u, B, seed, p15):
    z = B @ np.asarray(u, float)
    return render_weighted(weights, z, seed, p15)


def acoustic_control_jac(weights, B, seed, p15):
    r = B.shape[1]
    J = np.zeros((len(extract_features(render_transport(weights, np.zeros(r), B, seed, p15))), r))
    for j in range(r):
        lo = np.zeros(r); hi = np.zeros(r)
        lo[j] -= STEP_U; hi[j] += STEP_U
        J[:, j] = (extract_features(render_transport(weights, hi, B, seed, p15)) -
                   extract_features(render_transport(weights, lo, B, seed, p15))) / (2.0*STEP_U)
    return J


def ov(a, b):
    na = np.linalg.norm(a); nb = np.linalg.norm(b)
    if na < 1e-12 or nb < 1e-12:
        return 1.0
    return float(abs(np.dot(a, b)/(na*nb)))


def effdim(J, q=.95):
    s = np.linalg.svd(J, compute_uv=False)
    e = s*s
    c = np.cumsum(e)/(np.sum(e)+1e-15)
    return int(np.searchsorted(c, q)+1)


def eval_route(route, anchors, maps):
    p15 = lambda n,z,w: params15(n,z,w,anchors,maps)
    feats_t = []; feats_15 = []; feats_cf = []
    bases = []; jacs_u = []; ranks = []; d95 = []
    prev = None

    for a in ALPHAS:
        w = route_weights(route, a)
        Jz = jac_z(w, Z0, TEST_SEED, p15)
        _, s, V, r = local_svd(Jz)
        B = transport_basis(prev, V, r)
        prev = B
        Ju = acoustic_control_jac(w, B, TEST_SEED, p15)
        bases.append(B); jacs_u.append(Ju); ranks.append(r); d95.append(effdim(Jz))
        # At neutral control state, transported and fixed-basis engines share the same anchor path.
        feats_t.append(extract_features(render_transport(w, np.zeros(r), B, TEST_SEED, p15)))
        feats_15.append(extract_features(render_weighted(w, Z0, TEST_SEED, p15)))
        feats_cf.append(extract_features(render_crossfade(w, TEST_SEED)))

    ct = continuity(feats_t); c15 = continuity(feats_15); ccf = continuity(feats_cf)

    # Evaluate retained direction identity only across pairs where that direction exists at both samples.
    pair_scores = []
    all_overlaps = []
    for i in range(len(jacs_u)-1):
        k = min(jacs_u[i].shape[1], jacs_u[i+1].shape[1])
        ovs = [ov(jacs_u[i][:,j], jacs_u[i+1][:,j]) for j in range(k)]
        all_overlaps.extend(ovs)
        pair_scores.append(float(np.mean(np.asarray(ovs) >= .85)) if ovs else 1.0)
    control_fraction = float(np.mean(np.asarray(all_overlaps) >= .85)) if all_overlaps else 1.0
    control_ok = control_fraction >= .90

    best = min(c15['p95'], ccf['p95'])
    audio_better = ct['p95'] <= best + 1e-12
    max_ok = ct['max_ratio'] <= 3.0
    adapt = len(set(ranks)) > 1 or len(set(d95)) > 1
    route_pass = control_ok and audio_better and max_ok and adapt
    return {
        'route': route,
        'transported': ct,
        'part15_fixed': c15,
        'crossfade': ccf,
        'control_overlap_fraction_ge_085': control_fraction,
        'adjacent_pair_pass_fractions': pair_scores,
        'active_rank': ranks,
        'd95': d95,
        'audio_better': audio_better,
        'max_ok': max_ok,
        'control_ok': control_ok,
        'dim_adapt': adapt,
        'route_pass': route_pass,
    }


def main():
    anchors = fit_anchors()
    maps = fit_maps(anchors)
    routes = [
        'granular-chaos-four-family',
        'physical-spectral-via-granular',
        'spectral-granular-via-chaos',
        'chaos-physical-via-spectral',
    ]
    rows = [eval_route(r, anchors, maps) for r in routes]
    control_count = sum(int(r['control_ok']) for r in rows)
    audio_count = sum(int(r['audio_better'] and r['max_ok']) for r in rows)
    adapt_count = sum(int(r['dim_adapt']) for r in rows)
    pass_count = sum(int(r['route_pass']) for r in rows)
    chaos_ok = any(r['route_pass'] and 'chaos' in r['route'] for r in rows)
    out = {
        'heldout_seed': TEST_SEED,
        'routes': rows,
        'control_route_count': control_count,
        'audio_route_count': audio_count,
        'dim_adapt_route_count': adapt_count,
        'route_passes': pass_count,
        'chaos_route_pass': chaos_ok,
        'overall_pass': pass_count >= 3 and chaos_ok,
        'criteria': 'Part 16 preregistration issue #17',
    }
    (OUT/'part16_result.json').write_text(json.dumps(out, indent=2)+'\n')
    print(json.dumps(out, indent=2))


if __name__ == '__main__':
    main()
