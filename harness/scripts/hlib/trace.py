"""Matriz de trazabilidad requisito -> diseño -> función -> test (DOC-GEN-05, SAF-05)."""
import re
from pathlib import Path

from . import common as c
from . import impl
from . import reqs as rq

OUT_REL = Path("docs") / "03-implementacion" / "trazabilidad.md"
TEST_FN = re.compile(r"void\s+(test_TC_F(\d{3})_(\d{2})\w*)\s*\(")


def design_refs(root):
    path = root / "docs" / "02-diseno" / "diseno.md"
    refs = {}
    if not path.is_file():
        return refs
    heading = ""
    for line in path.read_text(encoding="utf-8").splitlines():
        m = re.match(r"^(#{2,4})\s+(.+)$", line)
        if m:
            heading = m.group(2).strip()
            continue
        for rid in re.findall(r"\b[A-Z]{2,}-[A-Z0-9]+(?:-[A-Z0-9]+)*\b", line):
            refs.setdefault(rid, [])
            if heading and heading not in refs[rid]:
                refs[rid].append(heading)
    return refs


def test_refs(root):
    refs = {}
    tdir = root / "tests"
    if not tdir.is_dir():
        return refs
    for f in sorted(tdir.rglob("*.c")):
        text = f.read_text(encoding="utf-8", errors="ignore")
        for m in TEST_FN.finditer(text):
            tc = "TC-F%s-%s" % (m.group(2), m.group(3))
            before = text[max(0, m.start() - 1500):m.start()]
            blk = before.rsplit("*/", 1)[0] if "*/" in before else ""
            blk = blk.rsplit("/*", 1)[-1] if "/*" in blk else ""
            for line in blk.splitlines():
                if "@req" in line:
                    for rid in re.findall(r"[A-Z]{2,}-[A-Z0-9]+(?:-[A-Z0-9]+)*", line):
                        refs.setdefault(rid, [])
                        if tc not in refs[rid]:
                            refs[rid].append(tc)
    return refs


def build(root=None):
    root = root or c.project_root()
    reqs = rq.load(root)
    plan = impl.load_plan(root)
    fn_refs = {}
    for f in plan.get("funciones", []):
        for r in f.get("requisitos", []):
            fn_refs.setdefault(r, []).append("%s `%s`" % (f["id"], f["nombre"]))
    d_refs, t_refs = design_refs(root), test_refs(root)
    rows = []
    for r in reqs:
        rid = r["id"]
        in_scope = rid in fn_refs
        tests = t_refs.get(rid, [])
        if not in_scope:
            est = "Fuera del alcance implementado"
        elif tests:
            est = "Cubierto"
        elif c.norm(r["campos"].get("verificacion", "")) == "test":
            est = "SIN TEST"
        else:
            est = "Verificación por %s" % r["campos"].get("verificacion", "?")
        rows.append({"id": rid, "prio": r["campos"].get("prioridad", ""),
                     "ver": r["campos"].get("verificacion", ""), "tipo": r["campos"].get("tipo", ""),
                     "design": d_refs.get(rid, []), "funcs": fn_refs.get(rid, []),
                     "tests": tests, "estado": est})
    return rows


def render(root=None):
    root = root or c.project_root()
    rows = build(root)
    L = ["# Matriz de trazabilidad", "",
         "Generada por `h trace` el %s. No editar a mano." % c.today(), "",
         "Tabla: Trazabilidad requisito → diseño → función → test", "",
         "| Requisito | Prioridad | Verificación | Diseño (apartados) | Funciones | Casos de test | Estado |",
         "|---|---|---|---|---|---|---|"]
    for r in rows:
        L.append("| %s | %s | %s | %s | %s | %s | %s |" % (
            r["id"], r["prio"], r["ver"], "<br>".join(r["design"]) or "—",
            "<br>".join(r["funcs"]) or "—", ", ".join(r["tests"]) or "—", r["estado"]))
    out = root / OUT_REL
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(L) + "\n", encoding="utf-8")
    c.info("trazabilidad: %s" % OUT_REL)
    gaps = [r["id"] for r in rows if r["estado"] == "SIN TEST"]
    return c.EXIT_FAIL if gaps else c.EXIT_OK


def safety_gaps(root=None):
    return ["%s (%s)" % (r["id"], r["tipo"]) for r in build(root) if r["estado"] == "SIN TEST"]
