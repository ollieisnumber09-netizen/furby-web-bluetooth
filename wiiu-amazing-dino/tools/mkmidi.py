# Amazing Dino's soundtrack. Each tune is our own melody and chords (the same strings the game
# keeps in game.c); this script arranges it for a band, plays it with real instrument samples
# (FluidSynth + the FluidR3 GM sound font) and cuts out one seamless loop.
#   python3 mkmidi.py            -> out/<song>.raw (mono 32 kHz s16, one loop each) and out/medley.wav
import struct, sys, random, subprocess, wave, math, os
import numpy as np
PPQ = 480; SR = 44100; OUT_SR = 32000
SF2 = '/usr/share/sounds/sf2/FluidR3_GM.sf2'
def code(c): return ord(c) - 48 if c.isdigit() else ord(c) - 55
def vlq(n):
    b = [n & 127]; n >>= 7
    while n: b.append((n & 127) | 128); n >>= 7
    return bytes(reversed(b))
class Midi:
    def __init__(s, bpm): s.ev = []; s.bpm = bpm
    def ctl(s, ch, vol, pan, rev): s.ev += [(0, 0, bytes([0xB0 | ch, 7, vol])), (0, 0, bytes([0xB0 | ch, 10, pan])), (0, 0, bytes([0xB0 | ch, 91, rev]))]
    def prog(s, t, ch, p): s.ev.append((max(0, int(t)), 0, bytes([0xC0 | ch, p])))
    def note(s, ch, t, n, dur, vel):
        vel = max(1, min(127, int(vel))); t = max(0, t); n = max(0, min(127, n))
        s.ev.append((int(t), 2, bytes([0x90 | ch, n, vel]))); s.ev.append((int(t + max(20, dur)), 1, bytes([0x80 | ch, n, 0])))
    def save(s, fn):
        s.ev.sort(key=lambda e: (e[0], e[1]))
        d = b'\x00\xff\x51\x03' + struct.pack('>I', int(60000000 / s.bpm))[1:]; last = 0
        for t, _, b in s.ev: d += vlq(t - last) + b; last = t
        d += b'\x00\xff\x2f\x00'
        open(fn, 'wb').write(b'MThd' + struct.pack('>IHHH', 6, 0, 1, PPQ) + b'MTrk' + struct.pack('>I', len(d)) + d)

BASS, KEYS, LEAD, LEAD2, HARM, PIZZ, ARP, PIANO, LOW, DR, PAD = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
HATS = {'x': 42, 's': 83, 't': 81, 'w': 76}
PERC = {'h': 60, 'l': 61, 's': 70, 'c': 56, 'w': 76, 'q': 63, 'd': 64, 't': 81}   # bongos, maracas, cowbell, woodblock, congas, triangle

def loop_steps(sg):
    spb = sg.get('spb', 8); nbars = len(sg['mel']) // spb
    return len(sg['mel']) * max(1, math.ceil(8 * len(sg['sections']) / nbars))

def arrange(sg, fn, loops=3, seed=1):
    R = random.Random(seed)
    m = Midi(sg['bpm']); spb = sg.get('spb', 8); sw = sg['swing']; mel = sg['mel']; ch = sg['chords']; base = sg['base']
    nsteps = len(mel); secs = sg['sections']; I = sg['ins']; ls = loop_steps(sg)
    V = dict(bass=112, keys=76, lead=118, lead2=86, harm=80, pizz=92, arp=72, piano=72, low=70, pad=62, drums=100); V.update(sg.get('vol', {}))
    m.ctl(BASS, V['bass'], 64, 20); m.ctl(KEYS, V['keys'], 44, 40); m.ctl(LEAD, V['lead'], 70, 45); m.ctl(LEAD2, V['lead2'], 56, 45); m.ctl(HARM, V['harm'], 84, 40)
    m.ctl(PIZZ, V['pizz'], 96, 45); m.ctl(ARP, V['arp'], 30, 45); m.ctl(PIANO, V['piano'], 80, 40); m.ctl(LOW, V['low'], 40, 45); m.ctl(PAD, V['pad'], 64, 60); m.ctl(DR, V['drums'], 64, 30)
    m.prog(0, BASS, I.get('bass', 32)); m.prog(0, KEYS, I.get('keys', 24)); m.prog(0, ARP, I.get('arp', 12)); m.prog(0, PIZZ, 45); m.prog(0, PIANO, 0)
    def T(step):   # tick of a step, with swing
        bar, st = divmod(step, spb); return bar * spb * 240 + (st // 2) * 480 + (int(480 * sw) if st % 2 else 0)
    def pat(name, step):
        p = sg.get(name, ''); return p[step % len(p)] if p else '.'
    hum = lambda v: v + R.randint(-6, 6)
    total = ls * loops
    for step in range(total):
        s = step % nsteps; bar = s // spb; st = s % spb; t = T(step) + R.randint(-4, 4)
        root = base - 24 + code(ch[(bar * 2) % len(ch)]); minor = ch[(bar * 2 + 1) % len(ch)] == 'm'
        if root > base - 24 + 7: root -= 12
        tri = [root + 24, root + 24 + (3 if minor else 4), root + 31, root + 36]
        steplen = T(step + 1) - T(step)
        sec = secs[(step // (spb * 8)) % len(secs)]
        if step % (spb * 8) == 0:   # a new section: the tune moves to other instruments
            for chn, key in ((LEAD, 'lead'), (LEAD2, 'lead2'), (HARM, 'harm'), (LOW, 'low'), (PAD, 'pad')):
                if sec.get(key) is not None: m.prog(T(step) - 30, chn, sec[key])
        c = mel[s]
        if c not in '.-':
            n = base + code(c) + sec.get('oct', 0); k = 1
            while s + k < nsteps and mel[s + k] == '-': k += 1
            dur = T(step + k) - T(step) - 30
            if k == 1 and not sg.get('legato'): dur = min(dur, 200)
            m.note(LEAD, t, n, dur, hum(104 if st % 2 == 0 else 92))
            if sec.get('lead2') is not None: m.note(LEAD2, t, n + (12 if n < 76 else 0), dur, hum(84))
            if sec.get('harm') is not None:
                h = n - 3
                while h > n - 10 and ((h - root) % 12) not in (0, 7, 3 if minor else 4): h -= 1
                m.note(HARM, t, h, dur, hum(74))
        b = pat('bass', step)
        if b != '.':
            off = {'0': 0, '7': 7, 'C': 12, '3': 3 if minor else 4, 'A': 10}[b]
            m.note(BASS, t, root + off, steplen * 2 - 60 if pat('bass', step + 1) == '.' else steplen - 30, hum(108 if st == 0 else 96))
        if pat('stab', step) == 'x':
            for q in tri[:3]: m.note(KEYS, t + R.randint(0, 8), q, 150, hum(78))
            if sec.get('piano'):
                for q in tri[1:4]: m.note(PIANO, t + R.randint(0, 8), q + 12, 130, hum(70))
        if sec.get('pizz'): m.note(PIZZ, t, tri[[0, 2, 1, 2, 3, 2, 1, 2][st % 8]] + 12, 160, hum(80 if st % 2 == 0 else 66))
        if st == 0:
            if sec.get('low') is not None: m.note(LOW, t, tri[1] - 12, spb * 240 - 40, hum(72))
            if sec.get('pad') is not None:
                for q in tri[:3]: m.note(PAD, t, q, spb * 240 - 20, hum(64))
        if sg.get('arp') and (sg['arp'] == 1 or st % 2 == 0):
            m.note(ARP, t, tri[[0, 1, 2, 3, 2, 1, 2, 3][st % 8]] + 12, 220, hum(70))
            if sg['arp'] == 1: m.note(ARP, t + steplen // 2, tri[[1, 2, 3, 2, 1, 2, 3, 0][st % 8]] + 12, 200, hum(58))
        if pat('kick', step) == 'x': m.note(DR, t, 36, 100, hum(112))
        sn = pat('snare', step)
        if sn == 'x': m.note(DR, t, 38, 100, hum(100))
        if sn in 'xc': m.note(DR, t, sg.get('clap', 39), 100, hum(84))
        hh = pat('hat', step)
        if hh in HATS: m.note(DR, t, HATS[hh], 80, hum(70 if st % 2 == 0 else 52))
        pc = pat('perc', step)
        if pc in PERC: m.note(DR, t, PERC[pc], 80, hum(84))
        if step % ls == 0 and sg.get('crash', 1): m.note(DR, t, 49, 400, 74)   # a crash each time round
        if bar % 8 == 7 and st >= spb - 3 and sg.get('fill'): m.note(DR, t, [47, 45, 43][st - (spb - 3)], 100, 96 + st)
    m.save(fn)
    return ls / 2 * 60.0 / sg['bpm']   # seconds in one loop

def render(name):
    """MIDI -> real instruments -> one loop, cut from the middle so its echoes wrap round."""
    sg = SONGS[name]; L = arrange(sg, 'out/' + name + '.mid')
    subprocess.run(['fluidsynth', '-ni', '-q', '-F', 'out/tmp.wav', '-r', str(SR), '-g', '0.25', SF2, 'out/' + name + '.mid'], check=True)
    w = wave.open('out/tmp.wav'); n = w.getnframes(); x = np.frombuffer(w.readframes(n), dtype=np.int16).astype(np.float64).reshape(-1, 2) / 32768; w.close()
    a = int(round(L * SR)); y = x[a:2 * a]
    rms = math.sqrt(float((y ** 2).mean())); y = y * (0.16 / rms); y = np.tanh(y * 1.25) / 1.25
    # the game's copy: mono, 32 kHz
    mono = y.mean(axis=1); nn = int(round(len(mono) * OUT_SR / SR))
    pos = np.arange(nn) * (len(mono) / nn); i0 = pos.astype(int); fr = pos - i0; i1 = (i0 + 1) % len(mono)
    # a gentle low-pass before dropping the rate, so high notes do not turn into fizz
    k = np.array([0.1, 0.24, 0.32, 0.24, 0.1]); ext = np.concatenate([mono[-2:], mono, mono[:2]]); mono = np.convolve(ext, k, 'valid')
    res = mono[i0] * (1 - fr) + mono[i1] * fr
    (res * 30000).astype('<i2').tofile('out/' + name + '.raw')
    print('%-10s loop %.1f s  peak %.2f' % (name, L, float(np.abs(y).max())))
    return y

S = lambda **k: k
SONGS = {
 'title': dict(base=62, bpm=168, swing=0.60, fill=1,   # Hello Dino: a little parade
    mel="7.77C.G." "E.C.9.7-" "9.99C.9." "7--.4.2." "4.57C.G." "H.G.E.C-" "E.EGE.B." "C--....."
        "H.HGH.L." "J.G.E.G-" "H.HGH.L." "N--.L.J." "O.L.J.G." "H.E.C.9." "B.EGJ.N." "O--.....",
    chords="0M5M5M7M0M9m7M0M5M0M5M7M0M9m7M0M",
    bass="0.0.7.7.", stab="..x...x.", kick="x...x...", snare="..x...xx", hat="x.x.x.x.",
    ins=dict(bass=58, keys=0), vol=dict(bass=120),
    sections=[S(lead=56, lead2=9, harm=60), S(lead=71, lead2=73, harm=60, pizz=1)]),
 'map': dict(base=67, bpm=112, swing=0.66, legato=1, clap=37, crash=0,   # Which Way?: a lazy stroll
    mel="4--57--4" "0--24---" "5--79--7" "7--54-2-" "4--57--C" "E--C9---" "5-7-9-B-" "C-----..",
    chords="0M9m5M7M0M9m5M0M",
    bass="0...7...", stab="..x...x.", kick="x.......", snare="....c...", hat="x.x.x.x.",
    ins=dict(bass=32, keys=24),
    sections=[S(lead=11, harm=25), S(lead=78, lead2=11, harm=73)]),
 'grass': dict(base=60, bpm=160, swing=0.62, fill=1,   # Jungle Hop
    mel="C.CG.EC." "9.C.7-.." "5.59.C.H" "G.E.B..." "C.CG.EC." "9.C.E-.." "H.G.E.B." "C--....."
        "9.CE.C9." "7.BE.B7." "5.9C.95." "7.B.E.G." "L.J.G.E." "H.E.C.9." "7.B.E.J." "O--.C...",
    chords="0M0M5M7M0M9m7M0M9m4m5M7M9m5M7M0M",
    bass="0.7.0.7.", stab=".x.x.x.x", kick="x...x...", snare="..x...x.", hat="x.x.x.x.", perc="..s.h.sl" "..s.h.s.",
    ins=dict(bass=32, keys=24),
    sections=[S(lead=114, harm=12), S(lead=71, lead2=13, harm=73, pizz=1), S(lead=21, lead2=73, harm=12, piano=1, low=57), S(lead=56, lead2=114, harm=65, pizz=1, piano=1, low=57)]),
 'wild': dict(base=64, bpm=150, swing=0.5, fill=1,   # Bongo Party: steel drums and a three-three-two beat
    mel="7..9..C." "E..C..9." "9..C..H." "G.E.B.7." "7..9..C." "H..L..H." "J.G.E.B." "C--....."
        "H.HH.L.H" "G.GG.J.G" "E.EE.H.E" "E.B.7.B." "H.HH.L.O" "N.L.J.G." "H.E.B.E." "C--.C.C.",
    chords="0M0M5M7M0M5M7M0M5M0M2m7M5M0M7M0M",
    bass="0..0..7.", stab="...x..x.", kick="x..x..x.", snare="....c...", hat="xxxxxxxx", perc="h..l..h." ".l.h..ll",
    ins=dict(bass=32, keys=24),
    sections=[S(lead=114, harm=12), S(lead=75, lead2=114, harm=12, piano=1)]),
 'cave': dict(base=57, bpm=104, swing=0.5, crash=0, arp=2,   # Sneaky Steps: tiptoeing
    mel="0.3.7.3." "8-7-...." "5.8.C.8." "B-7-...." "C.F.J.F." "K-J-H-F-" "E.B.7.B." "C--.....",
    chords="0m0m5m7M0m8M7M0m",
    bass="0...7...", hat="..x...x.", perc="....w..." "....w.w.",
    ins=dict(bass=43, arp=46), vol=dict(arp=60, bass=118),
    sections=[S(lead=45, oct=12), S(lead=68, oct=12, lead2=8, pad=48)]),
 'falls': dict(base=65, bpm=150, swing=0.5, spb=6, legato=1, crash=0, arp=2,   # Up in the Clouds: a waltz
    mel="7-C-G-" "E---C-" "9-C-H-" "E---B-" "C-E-G-" "H---L-" "J-H-E-" "C-----"
        "L-J-H-" "G---C-" "H-E-9-" "B---7-" "9-C-H-" "G-J-O-" "N-J-E-" "C-----",
    chords="0M0M5M7M9m5M7M0M5M0M2m7M5M0M7M0M",
    bass="0.....", stab="..x.x.", hat="..t.t.",
    ins=dict(bass=32, keys=24, arp=46),
    sections=[S(lead=73, harm=71, pad=48), S(lead=68, lead2=9, harm=73, pad=48)]),
 'snow': dict(base=62, bpm=132, swing=0.67, clap=37, crash=0, arp=2,   # Frosty Skip: sleigh bells
    mel="G.EC.E.G" "L.HC.H.L" "J.GE.G.J" "G--.C..." "L.GC.G.L" "O.LH.L.O" "N.JE.J.N" "O--....."
        "H.L.O.L." "G.J.O.J." "E.H.L.H." "J--.E.J." "L.O.L.H." "J.G.E.C." "E.G.J.N." "O--.C...",
    chords="0M5M7M0M9m5M7M0M5M0M2m7M5M0M7M0M",
    bass="0.7.0.7.", kick="x...x...", snare="..c...c.", hat="ssssssss",
    ins=dict(bass=32, arp=45), vol=dict(arp=84),
    sections=[S(lead=9, lead2=8, harm=71), S(lead=71, lead2=9, harm=73, pad=48)]),
 'ice': dict(base=64, bpm=96, swing=0.5, legato=1, crash=0, arp=1,   # Crystal Echo: slow and sparkly
    mel="J---F-C-" "K---F---" "M---J-F-" "N---J---" "O-M-K-J-" "K-H-F-H-" "E-J-N-J-" "C-------",
    chords="0m8M3M7M0m5m7M0m",
    bass="0.......", hat="....t...",
    ins=dict(bass=32, arp=46), vol=dict(arp=64),
    sections=[S(lead=10, pad=49), S(lead=8, lead2=11, harm=73, pad=49)]),
 'night': dict(base=58, bpm=120, swing=0.58, legato=1, clap=37, crash=0,   # Northern Lights: a gentle song
    mel="7---C-G-" "J---G-E-" "H---L-O-" "N-L-J---" "L---O-L-" "H---C-9-" "B-E-J-N-" "O-----..",
    chords="0M4m5M7M9m5M7M0M",
    bass="0...7.0.", stab="..x...x.", kick="x...x...", snare="....c...", hat="x.x.x.x.",
    ins=dict(bass=32, keys=24),
    sections=[S(lead=79, harm=73, pad=48), S(lead=73, lead2=11, harm=71, pad=48)]),
 'desert': dict(base=62, bpm=126, swing=0.5, crash=0,   # Dune Dance: hand drums and a snake-charmer tune
    mel="0.14.5.4" "1-0-...." "4.57.8.7" "5-8-...." "7.8A.C.A" "8.7.5.4." "5.8.5.1." "0--.....",
    chords="0M1M0M5m0M5m1M0M",
    bass="0..0..7.", stab="..x...x.", kick="x..x..x.", hat="x.xxx.xx", perc="q..d..d." "q.dd..d.",
    ins=dict(bass=32, keys=24),
    sections=[S(lead=68, oct=12), S(lead=71, oct=12, lead2=104, harm=68)]),
 'castle': dict(base=57, bpm=138, swing=0.5, crash=0, arp=2,   # Castle in the Clouds: harpsichord and organ
    mel="C.E.F.J." "N.J.E.B." "C.F.J.O." "M.H.E.A." "K.O.K.F." "H.K.H.C." "E.J.N.J." "O--.C...",
    chords="0m7M0mAM8M5m7M0m",
    bass="0.7.3.7.", hat="..t...t.",
    ins=dict(bass=43, arp=6), vol=dict(arp=70, bass=118),
    sections=[S(lead=6, pad=48), S(lead=19, lead2=6, pad=48, low=57)]),
 'boss': dict(base=62, bpm=184, swing=0.5, fill=1,   # Big Trouble
    mel="0.03.5.7" "8.7.5.3." "8.8C.F.C" "E.B.7-.." "C.CF.H.J" "K.J.H.F." "E.EJ.N.J" "O--.C.C.",
    chords="0m0m8M7M0m5m7M0m",
    bass="0.0C0.0C", stab="x..x..x.", kick="x.x.x.x.", snare="..x...x.", hat="xxxxxxxx",
    ins=dict(bass=33, keys=48), vol=dict(keys=90),
    sections=[S(lead=61, oct=12, harm=57), S(lead=56, oct=12, lead2=61, harm=57, piano=1)]),
}
ORDER = ['title', 'map', 'grass', 'wild', 'cave', 'falls', 'snow', 'ice', 'night', 'desert', 'castle', 'boss']
if __name__ == '__main__':
    os.makedirs('out', exist_ok=True)
    names = sys.argv[1:] or ORDER; parts = []
    for name in names:
        y = render(name); n = min(len(y), 22 * SR); seg = y[:n].copy()
        f = int(1.5 * SR); seg[-f:] *= np.linspace(1, 0, f)[:, None]
        parts += [seg, np.zeros((SR // 2, 2))]
    y = np.concatenate(parts)
    w = wave.open('out/medley.wav', 'wb'); w.setnchannels(2); w.setsampwidth(2); w.setframerate(SR); w.writeframes((y * 30000).astype(np.int16).tobytes()); w.close()
