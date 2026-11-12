# Turns Amazing Dino's tunes (our own melodies and chords) into MIDI files played by real instrument samples.
import struct, sys, random
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
    BASS, KEYS, LEAD, LEAD2, HARM, BAH, PERC2, DR = 0, 1, 2, 3, 4, 5, 6, 9
    I = sg['ins']
    m.prog(BASS, I['bass'], 112, 64, 20); m.prog(KEYS, I['keys'], 78, 44, 40); m.prog(LEAD, I['lead'], 118, 70, 45)
    m.prog(LEAD2, I['lead2'], 92, 56, 45); m.prog(HARM, I['harm'], 84, 84, 40); m.prog(BAH, I['bah'], 122, 64, 50); m.prog(PERC2, I['arp'], 74, 30, 45)
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
        c = mel[s]
        if c not in '.-':
            n = base + code(c); k = 1
            while s + k < nsteps and mel[s + k] == '-': k += 1
            dur = T(step + k) - T(step) - 30
            if k == 1: dur = min(dur, 200)
            m.note(LEAD, t, n, dur, hum(104 if st % 2 == 0 else 92))
            if lp >= 1: m.note(LEAD2, t, n + (12 if n < 72 else 0), dur, hum(88))
            if sg.get('harm'):
                h = n - 3
                while h > n - 10 and ((h - root) % 12) not in (0, 7, 3 if minor else 4): h -= 1
                m.note(HARM, t, h, dur, hum(74))
        b = pat('bass', step)
        if b != '.':
            off = {'0': 0, '7': 7, 'C': 12, '3': 3 if minor else 4, 'A': 10}[b]
            m.note(BASS, t, root + off, steplen * 2 - 60 if pat('bass', step + 1) == '.' else steplen - 30, hum(108 if st == 0 else 96))
        if pat('stab', step) == 'x':
            for q in tri[:3]: m.note(KEYS, t + R.randint(0, 8), q, 150, hum(78))
        if sg.get('arp') and (sg['arp'] == 1 or st % 2 == 0):
            m.note(PERC2, t, tri[[0, 1, 2, 3, 2, 1, 2, 3][st % 8]] + 12, 200, hum(70))
        bh = pat('bah', step) if lp == 0 else pat('bah2', step) if sg.get('bah2') else pat('bah', step)
        if bh == 'b':
            for q in tri[:3]: m.note(BAH, t, q + 12 if q < 60 else q, 190, hum(118))
        if bh == 'l': m.note(BAH, t, root + 24, 220, hum(116))
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
    for ch_, n in ((BASS, root - 12), (LEAD, root + 24), (KEYS, root + 12), (KEYS, root + 16), (KEYS, root + 19), (BAH, root + 12), (BAH, root + 16), (BAH, root + 19)): m.note(ch_, end, n, 700, 104)
    m.note(DR, end, 36, 100, 110); m.note(DR, end, 49, 600, 96)
    m.save(fn)

SONGS = {
 'jungle-hop': dict(base=60, bpm=160, swing=0.62, harm=1, fill=1,
    mel="C.CG.EC." "9.C.7-.." "5.59.C.H" "G.E.B..." "C.CG.EC." "9.C.E-.." "H.G.E.B." "C--....."
        "9.CE.C9." "7.BE.B7." "5.9C.95." "7.B.E.G." "L.J.G.E." "H.E.C.9." "7.B.E.J." "O--.C...",
    chords="0M0M5M7M0M9m7M0M9m4m5M7M9m5M7M0M",
    bass="0.7.0.7.", stab=".x.x.x.x", kick="x...x...", snare="..x...x.", hat="x.x.x.x.", perc="..s.h.sl" "..s.h.s." ,
    bah="........" "....b.b." "........" "....b.b." "........" "....b.b." "........" "....b.b.",
    bah2="........" "....b.b." "..b...b." "....b.b." "........" "....b.b." "..b...b." "..b.b.b.",
    ins=dict(bass=32, keys=24, lead=114, lead2=59, harm=12, bah=53, arp=12)),
}
if __name__ == '__main__':
    for name in sys.argv[1:] or SONGS: arrange(SONGS[name], name + '.mid')
