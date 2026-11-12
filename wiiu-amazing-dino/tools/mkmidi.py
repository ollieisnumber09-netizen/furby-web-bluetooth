# Turns Amazing Dino's tunes (our own melodies and chords) into MIDI files played by real instrument samples.
import struct, sys, random, subprocess, wave, math
import numpy as np
import bah as bahmod
PPQ = 480
def code(c): return ord(c) - 48 if c.isdigit() else ord(c) - 55
def vlq(n):
    b = [n & 127]; n >>= 7
    while n: b.append((n & 127) | 128); n >>= 7
    return bytes(reversed(b))
class Midi:
    def __init__(s, bpm): s.ev = []; s.bpm = bpm
    def prog(s, ch, p, vol=100, pan=64, rev=40):
        s.ev += [(0, 0, bytes([0xC0 | ch, p])), (0, 0, bytes([0xB0 | ch, 7, vol])), (0, 0, bytes([0xB0 | ch, 10, pan])), (0, 0, bytes([0xB0 | ch, 91, rev]))]
    def note(s, ch, t, n, dur, vel):
        vel = max(1, min(127, int(vel)))
        t = max(0, t); s.ev.append((int(t), 2, bytes([0x90 | ch, n, vel]))); s.ev.append((int(t + dur), 1, bytes([0x80 | ch, n, 0])))
    def save(s, fn):
        s.ev.sort(key=lambda e: (e[0], e[1]))
        d = b'\x00\xff\x51\x03' + struct.pack('>I', int(60000000 / s.bpm))[1:]; last = 0
        for t, _, b in s.ev: d += vlq(t - last) + b; last = t
        d += b'\x00\xff\x2f\x00'
        open(fn, 'wb').write(b'MThd' + struct.pack('>IHHH', 6, 0, 1, PPQ) + b'MTrk' + struct.pack('>I', len(d)) + d)

def arrange(sg, fn, loops=2, seed=1):
    R = random.Random(seed)
    m = Midi(sg['bpm']); spb = sg.get('spb', 8); sw = sg['swing']; mel = sg['mel']; ch = sg['chords']; base = sg['base']
    nsteps = len(mel); nbars = nsteps // spb
    BASS, KEYS, LEAD, LEAD2, HARM, PIZZ, PERC2, PIANO, TROM, DR = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    bahs = []; secs = sg['sections']
    I = sg['ins']
    m.prog(BASS, I['bass'], 112, 64, 20); m.prog(KEYS, I['keys'], 78, 44, 40); m.prog(LEAD, I['lead'], 118, 70, 45)
    m.prog(LEAD2, 73, 88, 56, 45); m.prog(HARM, 12, 84, 84, 40); m.prog(PIZZ, 45, 96, 96, 45); m.prog(PERC2, I['arp'], 74, 30, 45); m.prog(PIANO, 0, 74, 80, 40); m.prog(TROM, 57, 70, 40, 45)
    m.ev.append((0, 0, bytes([0xB0 | DR, 7, 104])))
    def T(step):   # tick of a step, with swing
        bar, st = divmod(step, spb); return bar * spb * 240 + (st // 2) * 480 + (int(480 * sw) if st % 2 else 0)
    def pat(name, step):
        p = sg.get(name, '');  return p[step % len(p)] if p else '.'
    hum = lambda v: v + R.randint(-6, 6)
    total = nsteps * loops
    for step in range(total):
        lp = step // nsteps; s = step % nsteps; bar = s // spb; st = s % spb; t = T(step) + R.randint(-4, 4)
        root = base - 24 + code(ch[(bar * 2) % len(ch)]); minor = ch[(bar * 2 + 1) % len(ch)] == 'm'
        if root > base - 24 + 7: root -= 12
        tri = [root + 24, root + 24 + (3 if minor else 4), root + 31, root + 36]
        steplen = T(step + 1) - T(step)
        sec = secs[(step // (spb * 8)) % len(secs)]
        if step % (spb * 8) == 0:   # a new section: the tune moves to other instruments
            for chn, key in ((LEAD, 'lead'), (LEAD2, 'lead2'), (HARM, 'harm')):
                if sec.get(key) is not None: m.ev.append((max(0, T(step) - 20), 0, bytes([0xC0 | chn, sec[key]])))
        c = mel[s]
        if c not in '.-':
            n = base + code(c); k = 1
            while s + k < nsteps and mel[s + k] == '-': k += 1
            dur = T(step + k) - T(step) - 30
            if k == 1: dur = min(dur, 200)
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
        if sec.get('trom') and st == 0: m.note(TROM, t, tri[1] - 12, spb * 240 - 40, hum(72))
        if sg.get('arp') and (sg['arp'] == 1 or st % 2 == 0):
            m.note(PERC2, t, tri[[0, 1, 2, 3, 2, 1, 2, 3][st % 8]] + 12, 200, hum(70))
        bp = sg['bah']; bh = bp[step % len(bp)]
        if bh == 'b':
            for q in tri[:3]: bahs.append((t, q + 12 if q < 58 else q))
        if pat('kick', step) == 'x': m.note(DR, t, 36, 100, hum(112))
        sn = pat('snare', step)
        if sn == 'x': m.note(DR, t, 38, 100, hum(100))
        if sn in 'xc': m.note(DR, t, 39, 100, hum(88))
        hh = pat('hat', step)
        if hh == 'x': m.note(DR, t, 42, 80, hum(70 if st % 2 == 0 else 52))
        if hh == 's': m.note(DR, t, 83, 80, hum(80 if st % 2 == 0 else 56))   # sleigh bells
        pc = pat('perc', step)
        if pc == 'h': m.note(DR, t, 60, 80, hum(86))    # high bongo
        if pc == 'l': m.note(DR, t, 61, 80, hum(82))    # low bongo
        if pc == 's': m.note(DR, t, 70, 60, hum(60))    # maracas
        if pc == 'c': m.note(DR, t, 56, 80, hum(72))    # cowbell
        if st == 0 and bar == 0: m.note(DR, t, 49, 400, 84)   # a crash to start each time round
        if bar % 8 == 7 and st >= spb - 3 and sg.get('fill'): m.note(DR, t, [47, 45, 43][st - (spb - 3)], 100, 96 + st)
    end = T(total)   # a last chord to finish
    root = base - 12
    for ch_, n in ((BASS, root - 12), (LEAD, root + 24), (KEYS, root + 12), (KEYS, root + 16), (KEYS, root + 19), ): m.note(ch_, end, n, 700, 104)
    for q in (12, 16, 19): bahs.append((end, root + q))
    m.note(DR, end, 36, 100, 110); m.note(DR, end, 49, 600, 96)
    m.save(fn)
    return [(tk / PPQ * 60.0 / sg['bpm'], n) for tk, n in bahs]


def render(name, out_wav, gain=0.25):
    """MIDI -> real instrument samples, then the sung bahs are mixed in on top."""
    sg = SONGS[name]; bahs = arrange(sg, name + '.mid')
    subprocess.run(['fluidsynth', '-ni', '-q', '-F', name + '-bed.wav', '-r', '44100', '-g', str(gain), '/usr/share/sounds/sf2/FluidR3_GM.sf2', name + '.mid'], check=True)
    w = wave.open(name + '-bed.wav'); n = w.getnframes(); x = np.frombuffer(w.readframes(n), dtype=np.int16).astype(np.float64).reshape(-1, 2) / 32768; w.close()
    v = np.zeros(len(x) + 44100)
    for k, (t, note) in enumerate(bahs):
        smp, pre = bahmod.bah(note, 0.25, 3, seed=k % 4)
        i0 = max(0, int((t - pre) * 44100)); seg = smp[:len(v) - i0]; v[i0:i0 + len(seg)] += seg * 0.16
    # a little room echo on the singers so they sit with the band
    rs = np.random.RandomState(7); ir = rs.randn(int(0.5 * 44100)) * np.exp(-np.arange(int(0.5 * 44100)) / (0.12 * 44100)) * 0.05; ir[0] = 1
    N = 1 << int(math.ceil(math.log2(len(v) + len(ir)))); v = np.fft.irfft(np.fft.rfft(v, N) * np.fft.rfft(ir, N), N)[:len(x)]
    y = x + v[:, None]; y = np.tanh(y * 1.6) / 1.05
    w = wave.open(out_wav, 'wb'); w.setnchannels(2); w.setsampwidth(2); w.setframerate(44100); w.writeframes((y * 30000).astype(np.int16).tobytes()); w.close()
    print(name, 'bahs', len(bahs), 'peak', float(np.abs(y).max()), 'secs', len(y) / 44100)

SONGS = {
 'jungle-hop': dict(base=60, bpm=160, swing=0.62, harm=1, fill=1,
    mel="C.CG.EC." "9.C.7-.." "5.59.C.H" "G.E.B..." "C.CG.EC." "9.C.E-.." "H.G.E.B." "C--....."
        "9.CE.C9." "7.BE.B7." "5.9C.95." "7.B.E.G." "L.J.G.E." "H.E.C.9." "7.B.E.J." "O--.C...",
    chords="0M0M5M7M0M9m7M0M9m4m5M7M9m5M7M0M",
    bass="0.7.0.7.", stab=".x.x.x.x", kick="x...x...", snare="..x...x.", hat="x.x.x.x.", perc="..s.h.sl" "..s.h.s.",
    # the singers only answer at the end of each line of the tune
    bah="........" "........" "........" "....b.b." "........" "........" "........" "....b.b.",
    sections=[dict(lead=114, harm=12),                                  # steel drums, marimba underneath
              dict(lead=71, lead2=13, harm=73, pizz=1),                 # clarinet with xylophone, flute, plucked strings
              dict(lead=21, lead2=73, harm=12, piano=1, trom=1),        # accordion with flute, piano, trombone
              dict(lead=56, lead2=114, harm=65, pizz=1, piano=1, trom=1)],   # trumpet with steel drums, saxophone
    ins=dict(bass=32, keys=24, lead=114, arp=12)),
}
if __name__ == '__main__':
    for name in sys.argv[1:] or SONGS: render(name, name + '.wav')
