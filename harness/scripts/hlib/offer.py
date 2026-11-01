"""Oferta: estado de secciones, presupuesto y plazo generados desde presupuesto.json."""
import datetime as dt
import re
from pathlib import Path

from . import common as c
from .gate import OFFER_SECTIONS, offer_sections

DIR = Path("docs") / "04-oferta"
MD_REL = DIR / "oferta.md"
BUDGET_REL = DIR / "presupuesto.json"
GANTT_REL = DIR / "diagramas" / "plazo.mmd"
SECTION_TITLES = {
    "objeto": "Objeto de la oferta", "introduccion": "Introducción", "datos": "Datos utilizados",
    "objetivos": "Objetivos", "alcance": "Alcance del proyecto y de los resultados",
    "requisitos": "Requisitos", "descripcion": "Descripción del proyecto",
    "presupuesto": "Presupuesto", "plazo": "Plazo del proyecto",
}


def _money(x):
    s = "{:,.2f}".format(x)
    return s.replace(",", "X").replace(".", ",").replace("X", ".")


def status(root=None):
    root = root or c.project_root()
    p = root / MD_REL
    secs = offer_sections(p.read_text(encoding="utf-8")) if p.is_file() else {}
    print("| Sección | Estado |\n|---|---|")
    for s in OFFER_SECTIONS:
        print("| %s (`%s`) | %s |" % (SECTION_TITLES[s], s, secs.get(s, "ausente")))
    return c.EXIT_OK


def set_section_state(section, state, root=None):
    root = root or c.project_root()
    if section not in OFFER_SECTIONS:
        c.die("sección desconocida: %s (%s)" % (section, ", ".join(OFFER_SECTIONS)), c.EXIT_USAGE)
    if state not in ("borrador", "en revision", "aprobado"):
        c.die("estado: borrador | en revision | aprobado", c.EXIT_USAGE)
    p = root / MD_REL
    text = p.read_text(encoding="utf-8")
    new, n = re.subn(r"<!--\s*seccion:\s*%s\s*\|\s*estado:\s*[a-z_ ]+?\s*-->" % section,
                     "<!-- seccion: %s | estado: %s -->" % (section, state), text)
    if n != 1:
        c.die("no se encontró el marcador de la sección %s en oferta.md" % section)
    p.write_text(new, encoding="utf-8")
    c.info("sección %s → %s" % (section, state))
    return c.EXIT_OK


def validate_budget(b):
    errs = []
    if not b.get("partidas"):
        errs.append("sin partidas")
    for p in b.get("partidas", []):
        for k in ("concepto", "horas", "tarifa"):
            if p.get(k) in (None, ""):
                errs.append("partida %s: falta %s" % (p.get("id", "?"), k))
    for t in (b.get("plazo") or {}).get("tareas", []):
        if not t.get("semanas"):
            errs.append("tarea %s sin semanas" % t.get("id"))
    if (b.get("plazo") or {}).get("inicio"):
        try:
            dt.date.fromisoformat(b["plazo"]["inicio"])
        except ValueError:
            errs.append("plazo.inicio debe ser AAAA-MM-DD")
    return errs


def render_budget(root=None):
    root = root or c.project_root()
    b = c.load_json(root / BUDGET_REL)
    if not b:
        c.die("no existe %s (lo rellena /oferta-seccion presupuesto con datos del usuario)"
              % BUDGET_REL)
    errs = validate_budget(b)
    if errs:
        for e in errs:
            c.log("ERROR", e)
        return c.EXIT_FAIL
    cur = b.get("moneda", "EUR")
    L = ["Tabla: Desglose del presupuesto por partidas", "",
         "| ID | Partida | Fase | Horas | Tarifa (%s/h) | Importe (%s) |" % (cur, cur),
         "|----|------------------------------|--------------|-------:|---------:|-----------:|"]
    sub = 0.0
    hours = 0.0
    for p in b["partidas"]:
        imp = float(p["horas"]) * float(p["tarifa"])
        sub += imp
        hours += float(p["horas"])
        L.append("| %s | %s | %s | %s | %s | %s |" % (p.get("id", ""), p["concepto"],
                                                      p.get("fase", ""), _money(float(p["horas"])),
                                                      _money(float(p["tarifa"])), _money(imp)))
    for o in b.get("otros", []):
        sub += float(o["importe"])
        L.append("| %s | %s | %s | — | — | %s |" % (o.get("id", ""), o["concepto"],
                                                     o.get("fase", ""), _money(float(o["importe"]))))
    L.append("| | **Subtotal** | | **%s** | | **%s** |" % (_money(hours), _money(sub)))
    total = sub
    cont = float(b.get("contingencia_pct") or 0)
    if cont:
        ci = sub * cont / 100.0
        total += ci
        L.append("| | Contingencia (%s %%) | | | | %s |" % (_money(cont), _money(ci)))
    tax = b.get("impuestos")
    if tax:
        ti = total * float(tax["pct"]) / 100.0
        L.append("| | Base imponible | | | | %s |" % _money(total))
        L.append("| | %s (%s %%) | | | | %s |" % (tax.get("nombre", "Impuestos"),
                                                   _money(float(tax["pct"])), _money(ti)))
        total += ti
    L.append("| | **Total** | | | | **%s %s** |" % (_money(total), cur))
    _replace_block(root, "budget", "\n".join(L))
    if (b.get("plazo") or {}).get("tareas"):
        _gantt(root, b["plazo"])
    c.info("presupuesto y plazo actualizados en %s" % MD_REL)
    return c.EXIT_OK


def _gantt(root, plazo):
    start = plazo.get("inicio") or c.today()
    L = ["gantt", "    dateFormat YYYY-MM-DD", "    axisFormat %d/%m",
         "    title Planificación del proyecto"]
    section = None
    for t in plazo["tareas"]:
        if t.get("fase") != section:
            section = t.get("fase") or "Proyecto"
            L.append("    section %s" % section)
        after = ("after %s" % " ".join(t["depende"])) if t.get("depende") else start
        L.append("    %s :%s, %s, %sw" % (t["nombre"], t["id"].lower(), after, t["semanas"]))
    for h in plazo.get("hitos", []):
        L.append("    %s :milestone, %s, after %s, 0d" % (h["nombre"], h["id"].lower(),
                                                         " ".join(h["tras"])))
    p = root / GANTT_REL
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text("\n".join(L) + "\n", encoding="utf-8")
    from . import diagram
    diagram.render_file(p)
    total = sum(float(t["semanas"]) for t in plazo["tareas"])
    _replace_block(root, "plazo", "![Figura. Planificación del proyecto](diagramas/plazo.svg)\n\n"
                   "Duración por tareas (suma, sin solapes): %s semanas." % _money(total).rstrip("0").rstrip(","))


def _replace_block(root, name, content):
    p = root / MD_REL
    text = p.read_text(encoding="utf-8")
    pat = r"<!-- h:%s:begin -->.*?<!-- h:%s:end -->" % (name, name)
    block = "<!-- h:%s:begin -->\n%s\n<!-- h:%s:end -->" % (name, content, name)
    if re.search(pat, text, flags=re.S):
        text = re.sub(pat, lambda _m: block, text, flags=re.S)
    else:
        c.log("WARN", "marcador h:%s no encontrado en oferta.md; se añade al final" % name)
        text = text.rstrip() + "\n\n" + block + "\n"
    p.write_text(text, encoding="utf-8")
