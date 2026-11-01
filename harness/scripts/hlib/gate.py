"""Puertas de fase (STD-PRO §1): estado, checklist automático, cierre y reapertura."""
import re
from pathlib import Path

from . import common as c
from . import reqs

DESIGN_DIR = Path("docs") / "02-diseno"
IMPL_DIR = Path("docs") / "03-implementacion"
OFFER_DIR = Path("docs") / "04-oferta"

CLOSED_FINDING = ("resuelto", "resuelta", "aceptado", "aceptada", "descartado", "descartada",
                  "validado", "validada", "cerrado", "cerrada", "rechazado", "rechazada")

OFFER_SECTIONS = ["objeto", "introduccion", "datos", "objetivos", "alcance", "requisitos",
                  "descripcion", "presupuesto", "plazo"]


# --------------------------------------------------------------------------- estado
def status(root=None):
    st = c.load_state(root)
    lines = ["| Fase | Estado | Versión | Tag | Cerrada |", "|---|---|---|---|---|"]
    icon = {"open": "abierta", "closed": "cerrada", "locked": "bloqueada"}
    for ph in c.PHASES:
        p = st["phases"][ph]
        lines.append("| %s | %s | %s | %s | %s |" % (
            c.PHASE_LABEL[ph], icon[p["status"]], p.get("version") or "-",
            p.get("tag") or "-", (p.get("closed_at") or "-")[:16]))
    return "\n".join(lines)


def require(phase, root=None):
    """Falla si la fase no está abierta (porque la anterior no está cerrada)."""
    st = c.load_state(root)
    p = st["phases"][phase]["status"]
    if p == "open":
        return True
    idx = c.PHASES.index(phase)
    if p == "locked" and idx > 0:
        prev = c.PHASES[idx - 1]
        c.die("PRO-01: la fase %s está bloqueada; antes hay que cerrar %s (/%s-cerrar)" % (
            c.PHASE_LABEL[phase], c.PHASE_LABEL[prev],
            {"req": "req", "design": "diseno", "impl": "impl"}[prev]))
    c.die("PRO-01: la fase %s está cerrada; para modificarla: /fase-reabrir %s <motivo>" % (
        c.PHASE_LABEL[phase], phase))
    return False


# --------------------------------------------------------------------------- checks
class Checklist(object):
    def __init__(self, phase):
        self.phase = phase
        self.items = []

    def add(self, ok, label, detail=None):
        self.items.append((bool(ok), label, detail or []))

    @property
    def ok(self):
        return all(i[0] for i in self.items)

    def render(self):
        out = ["Checklist de cierre · %s" % c.PHASE_LABEL[self.phase]]
        for ok, label, detail in self.items:
            out.append("  [%s] %s" % ("x" if ok else " ", label))
            for d in detail[:25]:
                out.append("        - %s" % d)
            if len(detail) > 25:
                out.append("        - … (%d más)" % (len(detail) - 25))
        out.append("RESULTADO: %s" % ("PASS" if self.ok else "FAIL"))
        return "\n".join(out)


def _prev_closed(cl, phase, st):
    idx = c.PHASES.index(phase)
    if idx == 0:
        return
    prev = c.PHASES[idx - 1]
    cl.add(st["phases"][prev]["status"] == "closed",
           "fase previa (%s) cerrada" % c.PHASE_LABEL[prev])


def _doc_ok(cl, root, rel, label):
    p = root / rel
    if not p.is_file():
        cl.add(False, "%s existe (%s)" % (label, rel))
        return ""
    text = p.read_text(encoding="utf-8")
    cl.add(True, "%s existe" % label)
    marks = c.find_markers(text)
    cl.add(not marks, "%s sin marcadores TBD/TODO/[PENDIENTE]/[PROPUESTA]/??" % label,
           ["línea %d: %s" % m for m in marks])
    return text


def _open_findings(path):
    if not path.is_file():
        return []
    out = []
    for table in c.md_tables(path.read_text(encoding="utf-8")):
        header = [c.norm(h) for h in table[0]]
        if "estado" not in header:
            continue
        ie = header.index("estado")
        for row in table[1:]:
            if len(row) > ie and c.norm(row[ie]) not in CLOSED_FINDING:
                out.append("%s (%s)" % (row[0], row[ie] or "sin estado"))
    return out


def _diagrams_fresh(root, folder):
    from . import diagram
    stale = []
    d = root / folder / "diagramas"
    if d.is_dir():
        for mmd in sorted(d.glob("*.mmd")):
            if not diagram.is_fresh(mmd):
                stale.append(str(mmd.relative_to(root)))
    return stale


def check_req(root, st):
    cl = Checklist("req")
    errors, warns = reqs.lint(root, strict=True)
    cl.add(not errors, "requisitos.md cumple STD-REQ (formato, atributos, Estado=Aprobado)",
           errors)
    oq = reqs.open_questions(root)
    cl.add(not oq, "dudas.md sin dudas abiertas", oq)
    if warns:
        cl.add(True, "avisos de calidad revisados (no bloquean)", warns)
    return cl


def check_design(root, st):
    cl = Checklist("design")
    _prev_closed(cl, "design", st)
    text = _doc_ok(cl, root, DESIGN_DIR / "diseno.md", "diseno.md")
    of = _open_findings(root / DESIGN_DIR / "revision.md")
    cl.add((root / DESIGN_DIR / "revision.md").is_file(), "revision.md (registro de hallazgos) existe")
    cl.add(not of, "revision.md sin hallazgos abiertos", of)
    missing = []
    for r in reqs.load(root):
        prio = c.norm(r["campos"].get("prioridad", ""))
        if prio in ("must", "should") and r["id"] not in text:
            missing.append(r["id"])
    cl.add(not missing, "trazabilidad: todo requisito Must/Should citado en diseno.md", missing)
    stale = _diagrams_fresh(root, DESIGN_DIR)
    cl.add(not stale, "diagramas renderizados y actualizados (h diagram render)", stale)
    alts = sorted((root / DESIGN_DIR / "alternativas").glob("ALT-*.md")) \
        if (root / DESIGN_DIR / "alternativas").is_dir() else []
    if alts:
        bad = [a.name for a in alts
               if "no se implementa" not in c.norm(a.read_text(encoding="utf-8"))]
        cl.add(not bad, "cada alternativa marcada 'Alternativa no seleccionada — no se implementa'",
               bad)
        from . import matrix
        mpath = root / DESIGN_DIR / "comparativa.json"
        errs = matrix.validate(c.load_json(mpath, {})) if mpath.is_file() \
            else ["falta comparativa.json"]
        cl.add(not errs, "comparativa.json válida (pesos = 100, puntuaciones en escala)", errs)
        cl.add(matrix.is_rendered(root), "comparativa.md con matrices actualizadas (h matrix render)")
        for m in c.find_markers((root / DESIGN_DIR / "comparativa.md").read_text(encoding="utf-8")
                                if (root / DESIGN_DIR / "comparativa.md").is_file() else ""):
            cl.add(False, "comparativa.md sin marcadores", ["línea %d: %s" % m])
            break
    return cl


def check_impl(root, st):
    from . import impl, trace
    cl = Checklist("impl")
    _prev_closed(cl, "impl", st)
    plan = impl.load_plan(root)
    cl.add(bool(plan.get("funciones")), "plan.json con funciones")
    pend = ["%s (%s)" % (f["id"], f.get("estado")) for f in plan.get("funciones", [])
            if f.get("estado") != "integrada"]
    cl.add(not pend, "todas las funciones integradas en develop", pend)
    integ = c.load_json(root / ".harness" / "reports" / "integration-latest.json") \
        or st["phases"]["impl"].get("integration") or {}
    cl.add(integ.get("result") == "PASS", "integración completa en PASS (h test --all)",
           [] if integ else ["no hay ejecución de integración registrada"])
    if integ.get("commit"):
        from . import gitflow
        main, _ = gitflow.names(root)
        merged = c.run(["git", "merge-base", "--is-ancestor", integ["commit"], main],
                       cwd=root).returncode == 0
        cl.add(merged, "commit integrado (%s) contenido en %s" % (integ["commit"][:8], main))
    _doc_ok(cl, root, IMPL_DIR / "implementacion.md", "implementacion.md")
    gaps = trace.safety_gaps(root)
    cl.add(not gaps, "SAF-05: todo requisito con verificación 'Test' del alcance tiene test", gaps)
    stale = _diagrams_fresh(root, IMPL_DIR)
    cl.add(not stale, "diagramas de implementación actualizados", stale)
    return cl


def offer_sections(text):
    found = {}
    for m in re.finditer(r"<!--\s*seccion:\s*([a-z_]+)\s*\|\s*estado:\s*([a-z_ ]+?)\s*-->", text):
        found[m.group(1)] = m.group(2).strip()
    return found


def check_offer(root, st):
    cl = Checklist("offer")
    _prev_closed(cl, "offer", st)
    text = _doc_ok(cl, root, OFFER_DIR / "oferta.md", "oferta.md")
    secs = offer_sections(text)
    pend = ["%s (%s)" % (s, secs.get(s, "ausente")) for s in OFFER_SECTIONS
            if secs.get(s) != "aprobado"]
    cl.add(not pend, "todas las secciones de la oferta aprobadas por el usuario", pend)
    cl.add((root / OFFER_DIR / "presentacion.json").is_file(), "presentacion.json existe")
    pptx = list((root / "docs" / "export").glob("*presentacion*.pptx")) \
        if (root / "docs" / "export").is_dir() else []
    cl.add(bool(pptx), "presentación PowerPoint generada (h pptx build)")
    stale = _diagrams_fresh(root, OFFER_DIR)
    cl.add(not stale, "diagramas de la oferta actualizados", stale)
    return cl


CHECKS = {"req": check_req, "design": check_design, "impl": check_impl, "offer": check_offer}


def check(phase, root=None):
    root = root or c.project_root()
    return CHECKS[phase](root, c.load_state(root))


# --------------------------------------------------------------------------- cierre
def _next_version(p):
    closes = [h for h in p.get("history", []) if h.get("event") == "close"]
    return "1.%d" % len(closes) if closes else "1.0"


def close(phase, root=None, use_git=True):
    root = root or c.project_root()
    st = c.load_state(root)
    if st["phases"][phase]["status"] != "open":
        c.die("la fase %s no está abierta (estado: %s)" % (
            c.PHASE_LABEL[phase], st["phases"][phase]["status"]))
    cl = check(phase, root)
    print(cl.render())
    if not cl.ok:
        return c.EXIT_FAIL
    if use_git:
        from . import gitflow
        if gitflow.current_branch(root) != c.PHASE_BRANCH[phase]:
            gitflow.start(c.PHASE_BRANCH[phase], root)
    p = st["phases"][phase]
    if phase == "impl":
        p["integration"] = c.load_json(root / ".harness" / "reports" / "integration-latest.json")
    version = _next_version(p)
    tag = "%s-v%s" % (c.PHASE_TAG[phase], version)
    p.update({"status": "closed", "version": version, "closed_at": c.now_iso(), "tag": tag})
    p.setdefault("history", []).append({"event": "close", "at": c.now_iso(), "version": version})
    idx = c.PHASES.index(phase)
    if idx + 1 < len(c.PHASES):
        nxt = st["phases"][c.PHASES[idx + 1]]
        if nxt["status"] == "locked":
            nxt["status"] = "open"
    c.save_state(st, root)
    if use_git:
        from . import gitflow
        gitflow.close_phase(phase, version, root)
    c.info("Fase %s cerrada: v%s (tag %s)" % (c.PHASE_LABEL[phase], version, tag))
    return c.EXIT_OK


def reopen(phase, reason, root=None, use_git=True):
    root = root or c.project_root()
    if not reason or len(reason.strip()) < 5:
        c.die("PRO-03: indica el motivo de la reapertura", c.EXIT_USAGE)
    st = c.load_state(root)
    p = st["phases"][phase]
    if p["status"] != "closed":
        c.die("la fase %s no está cerrada" % c.PHASE_LABEL[phase])
    if use_git:
        from . import gitflow
        gitflow.start(c.PHASE_BRANCH[phase], root)
    p["status"] = "open"
    p.setdefault("history", []).append({"event": "reopen", "at": c.now_iso(), "reason": reason})
    for later in c.PHASES[c.PHASES.index(phase) + 1:]:
        lp = st["phases"][later]
        if lp["status"] != "locked":
            lp.setdefault("history", []).append(
                {"event": "locked-by-reopen", "at": c.now_iso(), "of": phase})
            lp["status"] = "locked"
    c.save_state(st, root)
    if use_git:
        from . import gitflow
        gitflow.git("add", str(c.STATE_REL), root=root)
        gitflow.git("commit", "-m", "docs(%s): reopen %s phase" % (
            {"req": "req", "design": "design", "impl": "impl", "offer": "offer"}[phase], phase),
            "-m", "Reason: %s" % reason, root=root)
    c.info("Fase %s reabierta. Fases posteriores bloqueadas hasta recerrar." % c.PHASE_LABEL[phase])
    return c.EXIT_OK


# --------------------------------------------------------------------------- guard de escritura
PHASE_PATHS = [
    ("docs/01-requisitos", "req"),
    ("docs/02-diseno", "design"),
    ("docs/03-implementacion", "impl"),
    ("src", "impl"), ("include", "impl"), ("tests", "impl"), ("frontend", "impl"),
    ("deploy", "impl"), ("config", "impl"), ("tools", "impl"), ("CMakeLists.txt", "impl"),
    ("CMakePresets.json", "impl"), (".harness/sandbox", "impl"),
    ("docs/04-oferta", "offer"), ("docs/dosier", "offer"),
]


def phase_for_path(rel):
    rel = rel.replace("\\", "/").lstrip("./")
    for prefix, ph in PHASE_PATHS:
        if rel == prefix or rel.startswith(prefix + "/"):
            return ph
    return None


def write_allowed(rel, root=None):
    ph = phase_for_path(rel)
    if ph is None:
        return True, ""
    stt = c.load_state(root)["phases"][ph]["status"]
    if stt == "open":
        return True, ""
    if stt == "locked":
        return False, "PRO-01: %s pertenece a la fase %s, que está bloqueada" % (
            rel, c.PHASE_LABEL[ph])
    return False, "PRO-01: %s pertenece a la fase %s, que está cerrada (/fase-reabrir)" % (
        rel, c.PHASE_LABEL[ph])
