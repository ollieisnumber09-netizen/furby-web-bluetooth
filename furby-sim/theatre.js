// Furby Theatre inside the simulator: play a video with a countdown, and the
// simulated Furby reacts at the times you set. Reactions press the same
// buttons you can press yourself, so they look and sound the same.
(() => {
  const $ = s => document.querySelector(s);
  const REACTIONS = [
    ['SING', '🎵 Sing'], ['TALK', '💬 Talk'], ['HUG', '🤗 Hug'], ['SHAKE', '🌀 Shake'],
    ['FEED', '🍪 Feed'], ['TAIL', '🐾 Pull tail'], ['UPSIDE', '🙃 Upside down'],
  ];
  const labelOf = a => (REACTIONS.find(r => r[0] === a) || [a, a])[1];

  const css = document.createElement('style');
  css.textContent = `
  #theatre .screen { position: relative; width: 100%; aspect-ratio: 16/9; background: #000; border-radius: 10px; overflow: hidden; margin-bottom: 8px; }
  #theatre video, #theatre canvas { position: absolute; inset: 0; width: 100%; height: 100%; }
  #theatre video { object-fit: contain; }
  #theatre canvas { pointer-events: none; }
  #theatre .row + .row { margin-top: 8px; }
  #theatre .row button { min-height: 40px; padding: 8px 12px; flex: 0 0 auto; }
  #theatre .time { color: var(--muted); font-variant-numeric: tabular-nums; font-size: 13px; }
  #thCues { list-style: none; margin: 8px 0 0; padding: 0; display: grid; gap: 4px; }
  #thCues li { display: flex; align-items: center; gap: 8px; font-size: 14px; }
  #thCues li.done { color: var(--muted); }
  #thCues li span:first-child { font-variant-numeric: tabular-nums; min-width: 52px; }
  #thCues li button { min-height: 30px; padding: 2px 10px; margin-left: auto; }
  #thHint { color: var(--muted); font-size: 13px; margin-top: 6px; }`;
  document.head.appendChild(css);

  const panel = document.createElement('div');
  panel.className = 'panel'; panel.id = 'theatre';
  panel.innerHTML = `
    <h2>Theatre</h2>
    <div class="screen"><video id="thVideo" playsinline preload="auto"></video><canvas id="thFx"></canvas></div>
    <div class="row">
      <select id="thShow" aria-label="Show"></select>
      <button id="thStart">▶ Start show</button>
      <button id="thPause">Pause</button>
      <span class="time" id="thTime">0:00.0</span>
    </div>
    <div class="row">
      <select id="thReact" aria-label="Reaction">${REACTIONS.map(r => `<option value="${r[0]}">${r[1]}</option>`).join('')}</select>
      <button id="thAdd">Add at current time</button>
    </div>
    <ul id="thCues"></ul>
    <label class="time"><input type="checkbox" id="thSound" checked> Video sound</label>
    <div id="thHint">Wake Furby up, pick a show and press Start show. He sings along with the video.</div>`;
  const dlcPanel = $('#dlcPick') && $('#dlcPick').closest('.panel');
  if (dlcPanel) dlcPanel.before(panel); else document.body.appendChild(panel);

  const video = $('#thVideo'), fx = $('#thFx'), ctx = fx.getContext('2d');
  let videoName = null, cues = [], fired = new Set(), shows = [], show = null, dlcLoaded = null;

  // ---- reactions: press the simulator's own button so visuals and sounds match
  function react(a) {
    if (window.furby && window.furby.asleep) return;
    const b = document.querySelector(`.controls [data-act="${a}"]`);
    if (b) b.click();
  }

  // ---- saving cues per video in this browser
  const key = () => 'furby-sim-theatre:' + (videoName || '');
  function save() { try { localStorage.setItem(key(), JSON.stringify(cues)); } catch (e) {} }
  function load() { try { return JSON.parse(localStorage.getItem(key()) || '[]'); } catch (e) { return []; } }

  const fmt = t => { t = isFinite(t) ? t : 0; const m = Math.floor(t / 60); return m + ':' + (t - m * 60).toFixed(1).padStart(4, '0'); };
  function render() {
    cues.sort((a, b) => a.t - b.t);
    const ul = $('#thCues'); ul.innerHTML = '';
    cues.forEach((c, i) => {
      const li = document.createElement('li');
      if (fired.has(c)) li.className = 'done';
      li.innerHTML = `<span>${fmt(c.t)}</span><span>${labelOf(c.a)}</span><button data-i="${i}" aria-label="Delete">✕</button>`;
      ul.appendChild(li);
    });
  }
  $('#thCues').addEventListener('click', e => {
    const b = e.target.closest('button'); if (!b) return;
    cues.splice(+b.dataset.i, 1); save(); render();
  });
  $('#thAdd').onclick = () => {
    if (!video.src) { $('#thHint').textContent = 'Pick a show first.'; return; }
    const c = { t: Math.round(video.currentTime * 10) / 10, a: $('#thReact').value };
    cues.push(c); fired.add(c); save(); render(); react(c.a);
  };
  function pickShow(id) {
    show = shows.find(s => s.id === id) || shows[0]; if (!show) return;
    video.src = '../' + show.video; videoName = show.id;
    cues = load(); fired.clear(); render();
  }
  fetch('../shows/shows.json', { cache: 'no-store' }).then(r => r.json()).then(list => {
    shows = list;
    for (const s of shows) $('#thShow').add(new Option(s.title, s.id));
    pickShow(shows[0] && shows[0].id);
  }).catch(() => { $('#thHint').textContent = 'Could not load the show list. Check your connection and reload.'; });
  $('#thShow').addEventListener('change', e => pickShow(e.target.value));
  $('#thSound').addEventListener('change', e => video.muted = !e.target.checked);

  // Load the show's song DLC into the simulated Furby (once per show).
  async function loadSong() {
    const f = window.furby;
    if (!show || !show.dlc || dlcLoaded === show.id) return true;
    if (!f || !f.base) { $('#thHint').textContent = 'Wake Furby up first (the button at the top), then press Start show.'; return false; }
    $('#thHint').textContent = 'Loading the song…';
    const r = await fetch('../' + show.dlc, { cache: 'no-store' });
    if (!r.ok) throw new Error('could not download the song');
    await f.loadDLC(new Uint8Array(await r.arrayBuffer()), show.dlc.split('/').pop());
    dlcLoaded = show.id; return true;
  }
  video.addEventListener('seeked', () => { fired = new Set(cues.filter(c => c.t < video.currentTime)); render(); });
  $('#thPause').onclick = () => { if (!video.src) return; video.paused ? video.play() : video.pause(); };
  video.addEventListener('play', () => $('#thPause').textContent = 'Pause');
  video.addEventListener('pause', () => $('#thPause').textContent = 'Play');

  // ---- countdown: our own design, pastel rings rippling out with a big number
  const COLS = [['#d6b3ff', '#b98cf2', '#a070e0'], ['#b6e2ff', '#86c4f0', '#4fa6e0'], ['#ffc4e3', '#f597c9', '#e8609f']];
  const cd = { on: false, start: 0, beat: 1000, last: -1 };
  function size() { const r = fx.getBoundingClientRect(), d = Math.min(devicePixelRatio || 1, 2); fx.width = r.width * d; fx.height = r.height * d; }
  new ResizeObserver(size).observe(fx);
  function rings(now, c, alpha) {
    const w = fx.width, h = fx.height, cx = w / 2, cy = h / 2, band = Math.max(w, h) / 14, maxR = Math.hypot(cx, cy) + band;
    ctx.globalAlpha = alpha; ctx.fillStyle = c[0]; ctx.fillRect(0, 0, w, h);
    ctx.strokeStyle = c[1]; ctx.lineWidth = band * 0.62;
    for (let r = (now / 18) % (band * 2); r < maxR; r += band * 2) {
      ctx.beginPath();
      for (let k = 0; k <= 48; k++) {
        const g = k / 48 * Math.PI * 2, rr = r + Math.sin(g * 3 + now / 900 + r / 90) * band * 0.12;
        const x = cx + Math.cos(g) * rr, y = cy + Math.sin(g) * rr * 0.92;
        k ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
      }
      ctx.closePath(); ctx.stroke();
    }
    ctx.globalAlpha = 1;
  }
  function number(n, p, c) {
    const w = fx.width, h = fx.height, pop = p < 0.18 ? 0.6 + p / 0.18 * 0.5 : 1.1 - Math.min(1, (p - 0.18) / 0.3) * 0.1, s = h * 0.45 * pop;
    ctx.save(); ctx.translate(w / 2, h / 2);
    ctx.font = `800 ${s}px system-ui, sans-serif`; ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
    ctx.lineJoin = 'round'; ctx.lineWidth = s * 0.09; ctx.strokeStyle = c[2]; ctx.strokeText(n, 0, 0);
    ctx.fillStyle = '#fff'; ctx.fillText(n, 0, 0); ctx.restore();
  }
  let ac = null;
  function chime(f, len) {
    try {
      ac = ac || new (window.AudioContext || window.webkitAudioContext)();
      const t = ac.currentTime, o = ac.createOscillator(), g = ac.createGain();
      o.frequency.value = f; g.gain.setValueAtTime(0.0001, t);
      g.gain.exponentialRampToValueAtTime(0.25, t + 0.02); g.gain.exponentialRampToValueAtTime(0.0001, t + len);
      o.connect(g); g.connect(ac.destination); o.start(t); o.stop(t + len);
    } catch (e) {}
  }

  $('#thStart').onclick = async () => {
    if (!show) return;
    try { if (!(await loadSong())) return; }
    catch (err) { console.error(err); $('#thHint').textContent = 'Could not load the song: ' + err.message; return; }
    $('#thHint').textContent = 'Enjoy the show!';
    video.pause(); video.currentTime = 0; fired.clear(); render();
    video.muted = !$('#thSound').checked;
    cd.on = true; cd.start = performance.now(); cd.last = -1;
    setTimeout(() => {
      chime(783.99, 0.9);
      // Furby starts singing at the same moment the video starts.
      if (show.song && window.furby && !window.furby.asleep) window.furby.playDLC(show.song);
      video.play().catch(() => {});
    }, 3 * cd.beat);
  };

  function tick(now) {
    ctx.clearRect(0, 0, fx.width, fx.height);
    if (cd.on) {
      const el = now - cd.start, i = Math.floor(el / cd.beat);
      if (i >= 3) {
        const f = 1 - (el - 3 * cd.beat) / 500;
        if (f > 0) rings(now, COLS[2], f); else cd.on = false;
      } else {
        if (cd.last !== i) { cd.last = i; chime([523.25, 587.33, 659.25][i], 0.5); }
        rings(now, COLS[i], 1); number(String(3 - i), (el % cd.beat) / cd.beat, COLS[i]);
      }
    } else if (!video.src) {
      rings(now, COLS[0], 0.35);
    }
    if (!video.paused && !cd.on) {
      let changed = false;
      for (const c of cues) if (!fired.has(c) && c.t <= video.currentTime) { fired.add(c); react(c.a); changed = true; }
      if (changed) render();
    }
    $('#thTime').textContent = fmt(video.currentTime) + ' / ' + fmt(video.duration);
    requestAnimationFrame(tick);
  }
  requestAnimationFrame(tick);
  render();
})();
