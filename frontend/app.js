// app.js — visor del grid de ocupación (STD-UI). Colores siempre desde variables CSS (UI-TEC-02).
// Lee frontend/data/escenarios.json, generado por tools/ogrid_demo.py ejecutando el código C real.
// Solo muestra datos: no hay controles que actúen sobre el sistema (UI-TEC-03).

const $ = (id) => document.getElementById(id);
const canvas = $("grid");
const ctx = canvas.getContext("2d");
const MARGIN = { left: 36, top: 18 };

let data = null;
let scIdx = 0;
let stepIdx = 0;
let selected = null; // {s, d}

const cssVar = (name) => getComputedStyle(document.documentElement).getPropertyValue(name).trim();
const hexToRgb = (hex) => [0, 2, 4].map((i) => parseInt(hex.replace("#", "").slice(i, i + 2), 16));
const lerp = (a, b, t) => a.map((v, i) => Math.round(v + (b[i] - v) * t));
const fmt = (v, n = 2) => (v === null || v === undefined ? "—" : v.toFixed(n));
const scenario = () => data.scenarios[scIdx];
const step = () => scenario().steps[stepIdx];
const cellAt = (st, s, d) => st.cells[(s - data.window.s0) * data.window.cols + d];

function geometry() {
  const dpr = window.devicePixelRatio || 1;
  const w = Math.round(canvas.clientWidth * dpr);
  const h = Math.round(canvas.clientHeight * dpr);
  if (canvas.width !== w || canvas.height !== h) {
    canvas.width = w;
    canvas.height = h;
  }
  const ml = MARGIN.left * dpr;
  const mt = MARGIN.top * dpr;
  const cw = (w - ml) / data.window.rows;
  const ch = (h - mt) / data.window.cols;
  return { dpr, w, h, ml, mt, cw, ch };
}

function draw() {
  if (!data) return;
  const g = geometry();
  const { dpr, w, h, ml, mt, cw, ch } = g;
  const free = hexToRgb(cssVar("--c-grid-free"));
  const occ = hexToRgb(cssVar("--c-grid-occupied"));
  const unk = cssVar("--c-grid-unknown");
  const err = cssVar("--c-error");
  const accent = cssVar("--c-accent");
  const muted = cssVar("--c-text-muted");
  const border = cssVar("--c-border");
  const bg = cssVar("--c-bg");
  ctx.clearRect(0, 0, w, h);
  ctx.font = `${11 * dpr}px ${cssVar("--font-mono")}`;
  ctx.textBaseline = "middle";
  const st = step();
  const { s0, rows, cols } = data.window;
  for (let r = 0; r < rows; r++) {
    for (let d = 0; d < cols; d++) {
      const c = cellAt(st, s0 + r, d);
      const x = ml + r * cw;
      const y = mt + d * ch;
      if (c.p === null) {
        ctx.fillStyle = unk;
      } else {
        const rgb = lerp(free, occ, c.p);
        ctx.fillStyle = `rgb(${rgb[0]},${rgb[1]},${rgb[2]})`;
      }
      ctx.fillRect(x, y, Math.ceil(cw), Math.ceil(ch));
      ctx.strokeStyle = border;
      ctx.lineWidth = dpr;
      ctx.strokeRect(x, y, cw, ch);
      if (c.occ) {
        // Anillo del color de fondo bajo el borde rojo: el rojo solo no contrasta con la celda más oscura.
        ctx.strokeStyle = bg;
        ctx.lineWidth = 4 * dpr;
        ctx.strokeRect(x + 2 * dpr, y + 2 * dpr, cw - 4 * dpr, ch - 4 * dpr);
        ctx.strokeStyle = err;
        ctx.lineWidth = 2 * dpr;
        ctx.strokeRect(x + 2 * dpr, y + 2 * dpr, cw - 4 * dpr, ch - 4 * dpr);
      }
    }
  }
  if (selected) {
    const x = ml + (selected.s - s0) * cw;
    const y = mt + selected.d * ch;
    ctx.strokeStyle = accent;
    ctx.lineWidth = 3 * dpr;
    ctx.setLineDash([4 * dpr, 3 * dpr]);
    ctx.strokeRect(x + 1.5 * dpr, y + 1.5 * dpr, cw - 3 * dpr, ch - 3 * dpr);
    ctx.setLineDash([]);
  }
  ctx.fillStyle = muted;
  ctx.textAlign = "center";
  for (let r = 0; r < rows; r += 4) ctx.fillText(String(s0 + r), ml + (r + 0.5) * cw, mt / 2);
  ctx.textAlign = "right";
  for (let d = 0; d < cols; d++) ctx.fillText("d" + d, ml - 6 * dpr, mt + (d + 0.5) * ch);
}

function renderStep() {
  const sc = scenario();
  const st = step();
  $("step-n").textContent = `paso ${stepIdx + 1} de ${sc.steps.length}`;
  $("step-label").textContent = st.label;
  $("k-occ").textContent = st.total_occupied;
  $("k-obs").textContent = st.total_observed;
  $("prev").disabled = stepIdx === 0;
  $("first").disabled = stepIdx === 0;
  $("next").disabled = stepIdx === sc.steps.length - 1;
  renderCell();
  draw();
}

function renderCell() {
  const sc = scenario();
  $("cell-title").textContent = `Celda s = ${selected.s}, d = ${selected.d}`;
  const body = $("cell-rows");
  body.textContent = "";
  sc.steps.forEach((st, i) => {
    const c = cellAt(st, selected.s, selected.d);
    const tr = document.createElement("tr");
    if (i === stepIdx) tr.className = "is-current";
    const cells = [
      [i === 0 ? "inicio" : st.label.split(":")[0], ""],
      [fmt(c.lo), "num"],
      [fmt(c.p, 3), "num"],
      [data.state_names[c.state], ""],
    ];
    for (const [text, cls] of cells) {
      const td = document.createElement("td");
      td.textContent = text;
      if (cls) td.className = cls;
      tr.appendChild(td);
    }
    body.appendChild(tr);
  });
}

function renderScenario() {
  const sc = scenario();
  $("sc-text").textContent = sc.text;
  selected = { s: sc.focus.s, d: sc.focus.d };
  stepIdx = 0;
  renderStep();
}

function renderConstants() {
  const k = data.constants;
  const rows = [
    ["Voto por eco", `+${k.vote_echo}`],
    ["Voto por paso sin eco", `${k.vote_pass}`],
    ["Celda oculta", "sin cambio"],
    ["Límites del log-odds", `${k.lo_min} a ${k.lo_max}`],
    ["Ocupado si", `p > ${k.occupied_prob}`],
    ["Celdas del grid", `${k.length_cells} × ${k.width_cells}`],
  ];
  const body = $("consts");
  for (const [name, value] of rows) {
    const tr = document.createElement("tr");
    const a = document.createElement("td");
    const b = document.createElement("td");
    a.textContent = name;
    b.textContent = value;
    b.className = "num";
    tr.append(a, b);
    body.appendChild(tr);
  }
}

function go(delta) {
  const n = scenario().steps.length;
  stepIdx = Math.max(0, Math.min(n - 1, stepIdx + delta));
  renderStep();
}

function onCanvasClick(ev) {
  const rect = canvas.getBoundingClientRect();
  const { s0, rows, cols } = data.window;
  const x = ev.clientX - rect.left - MARGIN.left;
  const y = ev.clientY - rect.top - MARGIN.top;
  const cw = (rect.width - MARGIN.left) / rows;
  const ch = (rect.height - MARGIN.top) / cols;
  const r = Math.floor(x / cw);
  const d = Math.floor(y / ch);
  if (r < 0 || r >= rows || d < 0 || d >= cols) return;
  selected = { s: s0 + r, d };
  renderStep();
}

function initTheme() {
  let theme = "light";
  try { theme = localStorage.getItem("ui-theme") || "light"; } catch (e) { /* sin storage */ }
  document.documentElement.dataset.theme = theme;
  $("theme").addEventListener("click", () => {
    theme = theme === "light" ? "dark" : "light";
    document.documentElement.dataset.theme = theme;
    try { localStorage.setItem("ui-theme", theme); } catch (e) { /* sin storage */ }
    draw();
  });
}

function setConn(state, text) {
  const el = $("conn");
  el.className = "ui-status ui-status--" + state;
  el.textContent = text;
}

async function init() {
  initTheme();
  try {
    const res = await fetch("data/escenarios.json");
    data = await res.json();
  } catch (e) {
    setConn("err", "sin datos: ejecuta tools/ogrid_demo.py --serve");
    return;
  }
  setConn("ok", "datos del código C real");
  const sel = $("scenario");
  data.scenarios.forEach((sc, i) => {
    const opt = document.createElement("option");
    opt.value = String(i);
    opt.textContent = sc.title;
    sel.appendChild(opt);
  });
  sel.addEventListener("change", () => { scIdx = parseInt(sel.value, 10); renderScenario(); });
  $("next").addEventListener("click", () => go(1));
  $("prev").addEventListener("click", () => go(-1));
  $("first").addEventListener("click", () => { stepIdx = 0; renderStep(); });
  canvas.addEventListener("click", onCanvasClick);
  document.addEventListener("keydown", (ev) => {
    if (ev.target.tagName === "SELECT") return;
    if (ev.key === "ArrowRight") go(1);
    if (ev.key === "ArrowLeft") go(-1);
  });
  window.addEventListener("resize", draw);
  renderConstants();
  renderScenario();
}

init();
