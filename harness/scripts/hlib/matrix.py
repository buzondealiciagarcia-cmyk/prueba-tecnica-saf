"""Matrices ponderadas de comparación de diseños (skill matriz-ponderada).

Fuente: docs/02-diseno/comparativa.json. Salida: bloque generado dentro de
docs/02-diseno/comparativa.md entre marcadores, y gráfico Mermaid de totales.
"""
import hashlib
import json
import re
from pathlib import Path

from . import common as c

DIR = Path("docs") / "02-diseno"
JSON_REL = DIR / "comparativa.json"
MD_REL = DIR / "comparativa.md"
CHART_REL = DIR / "diagramas" / "comparativa-totales.mmd"
BEGIN = "<!-- h:matrix:begin sha=%s -->"
BEGIN_RE = re.compile(r"<!-- h:matrix:begin sha=([0-9a-f]+) -->")
END = "<!-- h:matrix:end -->"
COMPLIANCE = {"si": "✓ Cumple", "parcial": "~ Parcial", "no": "✗ No cumple", "na": "— N/A"}


def fmt(x, nd=2):
    return ("%.*f" % (nd, x)).replace(".", ",")


def validate(data):
    errs = []
    if not data:
        return ["comparativa.json vacío"]
    crit = data.get("criterios") or []
    alts = data.get("alternativas") or []
    if not crit:
        errs.append("sin criterios")
    if len(alts) < 2:
        errs.append("se necesitan al menos 2 alternativas (diseño seleccionado + 1)")
    total = sum(float(k.get("peso", 0)) for k in crit)
    if abs(total - 100.0) > 1e-6:
        errs.append("los pesos suman %s, deben sumar 100" % fmt(total))
    ids = [k.get("id") for k in crit]
    if len(set(ids)) != len(ids):
        errs.append("IDs de criterio duplicados")
    esc = data.get("escala") or {"min": 1, "max": 5}
    lo, hi = float(esc.get("min", 1)), float(esc.get("max", 5))
    sc = data.get("puntuaciones") or {}
    for a in alts:
        row = sc.get(a.get("id"), {})
        for k in crit:
            v = row.get(k["id"])
            if v is None:
                errs.append("falta puntuación %s/%s" % (a.get("id"), k["id"]))
            elif not (lo <= float(v) <= hi):
                errs.append("puntuación %s/%s=%s fuera de escala [%s, %s]" % (
                    a.get("id"), k["id"], v, lo, hi))
    sel = [a for a in alts if a.get("seleccionada")]
    if len(sel) != 1:
        errs.append("debe haber exactamente una alternativa con \"seleccionada\": true")
    comp = data.get("cumplimiento")
    if comp:
        for aid, vals in (comp.get("valores") or {}).items():
            for rid, v in vals.items():
                if c.norm(v) not in COMPLIANCE:
                    errs.append("cumplimiento %s/%s='%s' (usa si/parcial/no/na)" % (aid, rid, v))
    return errs


def totals(data, weights=None):
    crit = data["criterios"]
    w = weights or {k["id"]: float(k["peso"]) for k in crit}
    out = {}
    for a in data["alternativas"]:
        row = data["puntuaciones"][a["id"]]
        out[a["id"]] = sum(float(row[k["id"]]) * w[k["id"]] / 100.0 for k in crit)
    return out


def _winner(t):
    return max(sorted(t), key=lambda k: t[k])


def sensitivity(data, pct):
    base_w = {k["id"]: float(k["peso"]) for k in data["criterios"]}
    rows = []
    for k in data["criterios"]:
        res = []
        for sign in (-1, 1):
            w = dict(base_w)
            new = max(0.0, w[k["id"]] * (1 + sign * pct / 100.0))
            others = sum(v for kk, v in w.items() if kk != k["id"])
            rest = 100.0 - new
            for kk in w:
                if kk != k["id"] and others > 0:
                    w[kk] = w[kk] * rest / others
            w[k["id"]] = new
            res.append(_winner(totals(data, w)))
        rows.append((k, res[0], res[1]))
    return rows


def render_block(data):
    crit = data["criterios"]
    alts = data["alternativas"]
    names = {a["id"]: a["nombre"] for a in alts}
    esc = data.get("escala") or {"min": 1, "max": 5}
    hi = float(esc.get("max", 5))
    t = totals(data)
    L = []
    L.append("### Criterios y pesos\n")
    L.append("| ID | Criterio | Peso (%) | Descripción |")
    L.append("|---|---|---:|---|")
    for k in crit:
        L.append("| %s | %s | %s | %s |" % (k["id"], k["nombre"], fmt(float(k["peso"]), 0),
                                           k.get("descripcion", "")))
    L.append("\nEscala de puntuación: %s–%s. %s\n" % (
        fmt(float(esc.get("min", 1)), 0), fmt(hi, 0), esc.get("descripcion", "")))
    head = "| Criterio | Peso | " + " | ".join(a["id"] for a in alts) + " |"
    sep = "|---|---:|" + "---:|" * len(alts)
    L.append("### Matriz de puntuaciones\n")
    L += [head, sep]
    for k in crit:
        L.append("| %s | %s | %s |" % (k["nombre"], fmt(float(k["peso"]), 0), " | ".join(
            fmt(float(data["puntuaciones"][a["id"]][k["id"]]), 0) for a in alts)))
    L.append("\n### Matriz ponderada\n")
    L += [head, sep]
    for k in crit:
        L.append("| %s | %s | %s |" % (k["nombre"], fmt(float(k["peso"]), 0), " | ".join(
            fmt(float(data["puntuaciones"][a["id"]][k["id"]]) * float(k["peso"]) / 100.0)
            for a in alts)))
    L.append("| **Total** | **100** | %s |" % " | ".join("**%s**" % fmt(t[a["id"]]) for a in alts))
    L.append("| **Total normalizado (%%)** | | %s |" % " | ".join(
        fmt(100.0 * t[a["id"]] / hi, 1) for a in alts))
    L.append("\n### Clasificación\n")
    L.append("| Posición | Alternativa | Total | Implementación |")
    L.append("|---:|---|---:|---|")
    for i, aid in enumerate(sorted(t, key=lambda k: (-t[k], k)), 1):
        sel = next(a for a in alts if a["id"] == aid).get("seleccionada")
        L.append("| %d | %s · %s | %s | %s |" % (i, aid, names[aid], fmt(t[aid]),
                                                 "Seleccionada" if sel else "No se implementa"))
    L.append("\n![Figura. Puntuación ponderada total por alternativa](diagramas/%s)\n"
             % CHART_REL.with_suffix(".svg").name)
    pct = float((data.get("sensibilidad") or {}).get("variacion_pct", 20))
    L.append("### Análisis de sensibilidad (±%s %% en cada peso)\n" % fmt(pct, 0))
    L.append("| Criterio | Ganadora con −%s %% | Ganadora con +%s %% | ¿Cambia la ganadora? |"
             % (fmt(pct, 0), fmt(pct, 0)))
    L.append("|---|---|---|---|")
    base = _winner(t)
    for k, lo_w, hi_w in sensitivity(data, pct):
        L.append("| %s | %s | %s | %s |" % (k["nombre"], lo_w, hi_w,
                                            "Sí" if (lo_w != base or hi_w != base) else "No"))
    comp = data.get("cumplimiento")
    if comp and comp.get("requisitos"):
        L.append("\n### Cumplimiento de requisitos\n")
        L.append("| Requisito | " + " | ".join(a["id"] for a in alts) + " |")
        L.append("|---|" + "---|" * len(alts))
        vals = comp.get("valores") or {}
        for rid in comp["requisitos"]:
            L.append("| %s | %s |" % (rid, " | ".join(
                COMPLIANCE[c.norm(vals.get(a["id"], {}).get(rid, "si"))] for a in alts)))
    just = data.get("justificaciones") or {}
    if just:
        L.append("\n### Justificación de las puntuaciones\n")
        L.append("| Alternativa | Criterio | Puntuación | Justificación |")
        L.append("|---|---|---:|---|")
        for a in alts:
            for k in crit:
                j = (just.get(a["id"]) or {}).get(k["id"])
                if j:
                    L.append("| %s | %s | %s | %s |" % (
                        a["id"], k["nombre"], fmt(float(data["puntuaciones"][a["id"]][k["id"]]), 0),
                        j))
    return "\n".join(L) + "\n"


def chart(data):
    t = totals(data)
    alts = [a["id"] for a in data["alternativas"]]
    hi = float((data.get("escala") or {}).get("max", 5))
    return ("xychart-beta\n    title \"Puntuación ponderada total\"\n"
            "    x-axis [%s]\n    y-axis \"Puntuación\" 0 --> %s\n    bar [%s]\n" % (
                ", ".join('"%s"' % a for a in alts), ("%g" % hi),
                ", ".join("%.2f" % t[a] for a in alts)))


def _sha(data):
    return hashlib.sha256(json.dumps(data, sort_keys=True).encode("utf-8")).hexdigest()[:16]


def render(root=None):
    root = root or c.project_root()
    data = c.load_json(root / JSON_REL)
    if data is None:
        c.die("no existe %s" % JSON_REL)
    errs = validate(data)
    if errs:
        for e in errs:
            c.log("ERROR", e)
        return c.EXIT_FAIL
    block = "%s\n\n%s\n%s" % (BEGIN % _sha(data), render_block(data), END)
    md = root / MD_REL
    if md.is_file():
        text = md.read_text(encoding="utf-8")
        if BEGIN_RE.search(text) and END in text:
            text = re.sub(r"<!-- h:matrix:begin sha=[0-9a-f]+ -->.*?<!-- h:matrix:end -->",
                          lambda _m: block, text, flags=re.S)
        else:
            text = text.rstrip() + "\n\n## Matrices de decisión\n\n" + block + "\n"
    else:
        from . import init
        tpl = c.HARNESS_HOME / "templates" / "docs" / "comparativa.md"
        text = init._render(tpl.read_text(encoding="utf-8"), init._values(root))
        text = text.replace("<!-- h:matrix -->", block)
    md.write_text(text, encoding="utf-8")
    chart_path = root / CHART_REL
    chart_path.parent.mkdir(parents=True, exist_ok=True)
    chart_path.write_text(chart(data), encoding="utf-8")
    from . import diagram
    ok, err = diagram.render_file(chart_path)
    if not ok:
        c.log("WARN", "gráfico no renderizado", detail=err.splitlines()[-1:] or "")
    c.info("matrices actualizadas en %s" % MD_REL)
    return c.EXIT_OK


def is_rendered(root=None):
    root = root or c.project_root()
    data = c.load_json(root / JSON_REL)
    md = root / MD_REL
    if data is None or not md.is_file():
        return False
    m = BEGIN_RE.search(md.read_text(encoding="utf-8"))
    return bool(m) and m.group(1) == _sha(data)
