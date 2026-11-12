# Sung "bah" voices made from scratch: a buzzing voice source shaped by mouth resonances that
# open from a closed-lips "b" into "ah". Several singers per note, each slightly different.
import numpy as np, math, random
FS = 44100
def _singer(midi, dur, R):
    n = int((dur + 0.10) * FS); t = np.arange(n) / FS
    pre = 0.035                                   # lips closed: a quiet hum before the "b" pops open
    f0 = 440 * 2 ** ((midi - 69 + R.uniform(-0.10, 0.10)) / 12)
    scoop = -1.2 * np.exp(-np.maximum(t - pre, 0) / 0.035) * (t >= pre) - 1.2 * (t < pre)   # the note slides up into pitch
    vib = 0.18 * np.sin(2 * math.pi * 5.6 * t + R.uniform(0, 6)) * np.clip((t - 0.12) / 0.1, 0, 1)
    f = f0 * 2 ** ((scoop + vib) / 12)
    ph = 2 * math.pi * np.cumsum(f) / FS
    src = np.zeros(n); k = 1
    while k * f0 * 1.1 < FS * 0.45:               # a buzz with all its overtones, softer the higher they go
        src += np.sin(k * ph) / k ** 1.25; k += 1
    src += np.random.RandomState(R.randint(0, 99999)).randn(n) * 0.012
    open_ = np.clip((t - pre) / 0.045, 0, 1); open_ = open_ * open_ * (3 - 2 * open_)
    sc = R.uniform(0.97, 1.05)
    F = [(260 + 640 * open_) * sc, (850 + 450 * open_) * sc, (2300 + 600 * open_) * sc, np.full(n, 3900.0 * sc)]
    BW = [90, 110, 170, 260]; G = [1.0, 0.7, 0.35, 0.2]
    out = np.zeros(n)
    for fi, bw, g in zip(F, BW, G):               # each mouth resonance rings on its own, then they are added up
        C = -math.exp(-2 * math.pi * bw / FS); r = math.exp(-math.pi * bw / FS)
        B = 2 * r * np.cos(2 * math.pi * fi / FS); A = 1 - B - C
        y = np.zeros(n); y1 = y2 = 0.0
        sl = src.tolist(); Bl = B.tolist(); Al = A.tolist()
        for i in range(n):
            v = Al[i] * sl[i] + Bl[i] * y1 + C * y2; y[i] = v; y2 = y1; y1 = v
        out += y * g
    env = np.where(t < pre, 0.10 * np.clip(t / 0.01, 0, 1), 0)
    tt = t - pre
    body = np.clip(tt / 0.012, 0, 1) * (0.72 + 0.28 * np.exp(-np.maximum(tt, 0) / 0.08)) * np.clip((dur - t) / 0.07, 0, 1)
    env = np.where(t >= pre, np.maximum(body, 0), env)
    return out * env, pre
_cache = {}
def bah(midi, dur=0.24, singers=3, seed=0):
    key = (midi, round(dur, 2), singers, seed)
    if key in _cache: return _cache[key]
    R = random.Random(seed * 1000 + midi); n = int((dur + 0.16) * FS); out = np.zeros(n); pre = 0
    for s in range(singers):
        w, pre = _singer(midi, dur, R); off = int(R.uniform(0, 0.012) * FS)
        out[off:off + len(w)] += w[:n - off]
    out /= max(1e-9, np.abs(out).max())
    _cache[key] = (out, pre); return _cache[key]
if __name__ == '__main__':
    import wave
    x = np.concatenate([np.concatenate([bah(m)[0], np.zeros(6000)]) for m in (60, 64, 67, 72, 67, 64)])
    w = wave.open('bah-test.wav', 'wb'); w.setnchannels(1); w.setsampwidth(2); w.setframerate(FS); w.writeframes((x * 20000).astype(np.int16).tobytes()); w.close()
