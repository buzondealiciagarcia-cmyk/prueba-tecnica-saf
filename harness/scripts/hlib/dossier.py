"""Dosier: une los documentos de requisitos, diseño e implementación con contexto."""
import os
import re
from pathlib import Path

from . import common as c
from . import gate

OUT_DIR = Path("docs") / "dosier"
PARTS = [
    ("Parte I — Requisitos", [("docs/01-requisitos/requisitos.md", True)],
     "Qué debe hacer el sistema: requisitos cerrados y aprobados que son la base contractual de "
     "las fases siguientes."),
    ("Parte II — Diseño", [("docs/02-diseno/diseno.md", True),
                          ("docs/02-diseno/alternativas/*.md", False),
                          ("docs/02-diseno/comparativa.md", True)],
     "Cómo se resuelve: diseño seleccionado, alternativas estudiadas que no se implementan y "
     "comparativa ponderada que justifica la elección."),
    ("Parte III — Implementación", [("docs/03-implementacion/implementacion.md", True),
                                   ("docs/03-implementacion/trazabilidad.md", False)],
     "Qué se ha construido y cómo se ha verificado: alcance implementado, funciones, resultados "
     "de test y trazabilidad hasta los requisitos."),
]


def _meta(text):
    from .export import _control_table
    return _control_table(text)


def _shift(text, src_dir, out_dir):
    lines, out, in_code = text.splitlines(), [], False
    first_h1_done = False
    skip_table = False
    for line in lines:
        if line.strip().startswith("```"):
            in_code = not in_code
        if not in_code:
            if re.match(r"^#\s", line) and not first_h1_done:
                first_h1_done = True
                out.append("#" + line)
                skip_table = True
                continue
            if skip_table:
                if line.strip().startswith("|"):
                    continue
                if line.strip():
                    skip_table = False
            if re.match(r"^#{1,5}\s", line):
                line = "#" + line
            line = re.sub(r"(!\[[^\]]*\]\()(?!https?://|/)([^)\s]+)\)",
                          lambda m: "%s%s)" % (m.group(1), os.path.relpath(
                              str((src_dir / m.group(2)).resolve()), str(out_dir.resolve()))), line)
        out.append(line)
    return "\n".join(out)


def build(root=None):
    root = root or c.project_root()
    cfg = c.load_config(root)
    out_dir = root / OUT_DIR
    out_dir.mkdir(parents=True, exist_ok=True)
    docs = []
    body = []
    for title, sources, intro in PARTS:
        body += ["", "# %s" % title, "", intro, ""]
        for pattern, required in sources:
            files = sorted(root.glob(pattern)) if "*" in pattern else [root / pattern]
            for f in files:
                if not f.is_file():
                    if required:
                        body += ["> Documento no disponible: `%s`." % pattern, ""]
                    continue
                text = f.read_text(encoding="utf-8")
                m = _meta(text)
                t = re.search(r"^#\s+(.+)$", text, flags=re.M)
                docs.append((t.group(1).strip() if t else f.stem, m.get("codigo", "—"),
                             m.get("version", "—"), m.get("fecha", "—"),
                             str(f.relative_to(root))))
                body += [_shift(text, f.parent, out_dir), ""]
    ctx = out_dir / "contexto.md"
    context = ctx.read_text(encoding="utf-8").strip() if ctx.is_file() else \
        "[PENDIENTE: redactar con el usuario el contexto del proyecto en docs/dosier/contexto.md]"
    context = re.sub(r"^#\s.*$", "", context, count=1, flags=re.M).strip()
    head = ["# Dosier del proyecto", "",
            "| Campo | Valor |", "|---|---|",
            "| Proyecto | %s |" % cfg["project"]["name"],
            "| Código | %s-DOS |" % cfg["project"]["code"],
            "| Versión | 1.0 |", "| Fecha | %s |" % c.today(),
            "| Autor | %s |" % cfg["project"]["author"], "| Estado | Generado |", "",
            "## Contexto del proyecto", "", context, "",
            "## Guía de lectura", "",
            "Este dosier reúne, en el orden en que se produjeron, los documentos cerrados de las "
            "tres primeras fases del proyecto. Cada parte comienza con una breve explicación de su "
            "propósito.", "",
            "Tabla: Documentos incluidos", "",
            "| Documento | Código | Versión | Fecha | Fuente |", "|---|---|---|---|---|"]
    for d in docs:
        head.append("| %s | %s | %s | %s | `%s` |" % d)
    head += ["", "Tabla: Estado de las fases", "", gate.status(root), ""]
    out = out_dir / "dosier.md"
    out.write_text("\n".join(head + body) + "\n", encoding="utf-8")
    c.info("dosier generado: %s" % out.relative_to(root))
    return c.EXIT_OK
