// app.js — visor del grid (STD-UI §6). Colores siempre desde variables CSS (UI-TEC-02).
// Formato de mensaje PROVISIONAL hasta cerrar el diseño (STD-MQTT MQ-PAY-03):
//   {"type":"grid","seq":1,"ts_ms":0,"w":W,"h":H,"res_m":0.2,"p":[...W*H, -1 = desconocido],
//    "n_echoes":N,"latency_ms":L}

const $ = (id) => document.getElementById(id);
const canvas = $("grid");
const ctx = canvas.getContext("2d");
let last = null;
let threshold = 0.7;

function cssVar(name) {
  return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
}

function hexToRgb(hex) {
  const h = hex.replace("#", "");
  return [0, 2, 4].map((i) => parseInt(h.slice(i, i + 2), 16));
}

function lerp(a, b, t) {
  return a.map((v, i) => Math.round(v + (b[i] - v) * t));
}

function draw() {
  requestAnimationFrame(draw);
  if (!last) return;
  const dpr = window.devicePixelRatio || 1;
  const w = canvas.clientWidth * dpr;
  const h = canvas.clientHeight * dpr;
  if (canvas.width !== w || canvas.height !== h) {
    canvas.width = w;
    canvas.height = h;
  }
  const free = hexToRgb(cssVar("--c-grid-free"));
  const occ = hexToRgb(cssVar("--c-grid-occupied"));
  const unk = cssVar("--c-grid-unknown");
  const err = cssVar("--c-error");
  const cw = w / last.w;
  const ch = h / last.h;
  let occupied = 0;
  for (let y = 0; y < last.h; y++) {
    for (let x = 0; x < last.w; x++) {
      const p = last.p[y * last.w + x];
      if (p < 0) {
        ctx.fillStyle = unk;
      } else {
        const c = lerp(free, occ, p);
        ctx.fillStyle = `rgb(${c[0]},${c[1]},${c[2]})`;
      }
      // eje x del grid = avance de la vía (de abajo arriba en pantalla)
      ctx.fillRect(x * cw, h - (y + 1) * ch, Math.ceil(cw), Math.ceil(ch));
      if (p >= threshold) {
        occupied++;
        ctx.strokeStyle = err;
        ctx.lineWidth = 2 * dpr;
        ctx.strokeRect(x * cw + dpr, h - (y + 1) * ch + dpr, cw - 2 * dpr, ch - 2 * dpr);
      }
    }
  }
  $("k-occ").textContent = occupied;
}

function onGrid(msg) {
  last = msg;
  $("k-seq").textContent = msg.seq ?? "—";
  $("k-lat").textContent = msg.latency_ms != null ? msg.latency_ms.toFixed(1) : "—";
  $("k-echo").textContent = msg.n_echoes ?? "—";
}

function setConn(state, text) {
  const el = $("conn");
  el.className = "ui-status ui-status--" + state;
  el.textContent = text;
}

function demo() {
  // Datos sintéticos solo para revisar el estilo (?demo=1); nunca para validar el algoritmo.
  let seq = 0;
  const W = 40, H = 60;
  setInterval(() => {
    const p = new Array(W * H).fill(-1);
    for (let y = 0; y < H; y++) {
      for (let x = 14; x < 26; x++) p[y * W + x] = 0.1;
    }
    for (let y = 38; y < 41; y++) for (let x = 18; x < 22; x++) p[y * W + x] = 0.9;
    onGrid({ type: "grid", seq: seq++, w: W, h: H, p, n_echoes: 128, latency_ms: 12.5 });
  }, 50);
  setConn("warn", "modo demo");
}

function connect() {
  const es = new EventSource("/events");
  es.onopen = () => setConn("ok", "conectado");
  es.onerror = () => setConn("err", "sin datos");
  es.onmessage = (ev) => {
    const msg = JSON.parse(ev.data);
    if (msg.type === "grid") onGrid(msg);
  };
}

function initTheme() {
  let theme = "light";
  try { theme = localStorage.getItem("ui-theme") || "light"; } catch (e) { /* sin storage */ }
  document.documentElement.dataset.theme = theme;
  $("theme").addEventListener("click", () => {
    theme = theme === "light" ? "dark" : "light";
    document.documentElement.dataset.theme = theme;
    try { localStorage.setItem("ui-theme", theme); } catch (e) { /* sin storage */ }
  });
}

$("thr").addEventListener("input", (e) => {
  threshold = parseFloat(e.target.value);
  $("thr-o").textContent = threshold.toFixed(2);
});
initTheme();
if (new URLSearchParams(location.search).has("demo")) demo(); else connect();
requestAnimationFrame(draw);
