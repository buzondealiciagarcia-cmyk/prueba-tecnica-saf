"""Requisitos: parser del formato markdown (STD-REQ / STD-DOC §3), lint y dudas."""
import re
from pathlib import Path

from . import common as c

REQ_DIR = Path("docs") / "01-requisitos"
REQ_FILE = REQ_DIR / "requisitos.md"
DUDAS_FILE = REQ_DIR / "dudas.md"

HEAD_RE = re.compile(r"^###\s+(\S+)\s+[—–-]\s+(.+?)\s*$")
SECTION_RE = re.compile(r"^##\s+(.+?)\s*$")

# claves canónicas -> variantes aceptadas (normalizadas)
FIELD_ALIASES = {
    "descripcion": ["descripcion", "description", "enunciado"],
    "tipo": ["tipo", "type", "categoria"],
    "prioridad": ["prioridad", "priority"],
    "origen": ["origen", "fuente", "source"],
    "verificacion": ["metodo de verificacion", "verificacion", "verification"],
    "criterio": ["criterio de aceptacion", "criterio", "acceptance criteria"],
    "estado": ["estado", "status"],
}
REQUIRED = ["descripcion", "tipo", "prioridad", "origen", "verificacion", "criterio", "estado"]
FIELD_LABEL = {
    "descripcion": "Descripción", "tipo": "Tipo", "prioridad": "Prioridad",
    "origen": "Origen", "verificacion": "Método de verificación",
    "criterio": "Criterio de aceptación", "estado": "Estado",
}

VAGUE = [
    "adecuado", "adecuada", "rapido", "rapida", "suficiente", "robusto", "robusta",
    "aproximadamente", "etc", "facil", "normalmente", "minimo posible", "maximo posible",
    "en tiempo real", "optimo", "optima", "eficiente", "flexible", "amigable", "user-friendly",
    "si es posible", "cuando sea necesario", "y/o",
]
UNIT_RE = re.compile(
    r"\d+([.,]\d+)?\s*(ms|s|min|h|hz|khz|mhz|ghz|m|cm|mm|km|km/h|m/s|mps|rad|deg|°|%|"
    r"db|dbsm|w|v|a|mb|kb|gb|bytes|b|celdas|ecos|puntos|nubes|fps|ciclos)\b",
    re.IGNORECASE)


def _canon(key):
    k = c.norm(key)
    for canon, aliases in FIELD_ALIASES.items():
        if k in aliases:
            return canon
    return k


def parse(text):
    """Devuelve lista de dicts: id, titulo, seccion, campos{canon: valor}, linea."""
    reqs = []
    section = ""
    cur = None
    text = re.sub(r"<!--.*?-->", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    for n, line in enumerate(text.splitlines(), 1):
        m_sec = SECTION_RE.match(line)
        if m_sec and not line.startswith("###"):
            section = m_sec.group(1)
            cur = None
            continue
        m = HEAD_RE.match(line)
        if m:
            cur = {"id": m.group(1), "titulo": m.group(2), "seccion": section,
                   "campos": {}, "linea": n}
            reqs.append(cur)
            continue
        if cur is None:
            continue
        s = line.strip()
        if s.startswith("|") and s.endswith("|"):
            cells = [x.strip() for x in s.strip("|").split("|")]
            if len(cells) >= 2 and not re.fullmatch(r":?-{2,}:?", cells[0]):
                key = _canon(cells[0])
                if key in ("campo", "field"):
                    continue
                cur["campos"][key] = "|".join(cells[1:]).strip()
    return reqs


def load(root=None):
    root = root or c.project_root()
    path = root / REQ_FILE
    if not path.is_file():
        return []
    return parse(path.read_text(encoding="utf-8"))


def lint(root=None, strict=False):
    """Comprueba STD-REQ. Devuelve (errores, avisos) como listas de str."""
    root = root or c.project_root()
    cfg = c.load_config(root)
    rcfg = cfg["requirements"]
    errors, warns = [], []
    path = root / REQ_FILE
    if not path.is_file():
        return ["no existe %s" % REQ_FILE], []
    text = path.read_text(encoding="utf-8")
    reqs = parse(text)
    if not reqs:
        errors.append("no se ha encontrado ningún requisito (bloques '### <ID> — <título>')")
    id_re = re.compile(rcfg["id_regex"])
    seen = {}
    allowed = {
        "tipo": [c.norm(x) for x in rcfg["types"]],
        "prioridad": [c.norm(x) for x in rcfg["priorities"]],
        "verificacion": [c.norm(x) for x in rcfg["verification"]],
        "estado": [c.norm(x) for x in rcfg["states"]],
    }
    for r in reqs:
        rid = r["id"]
        where = "%s (línea %d)" % (rid, r["linea"])
        if not id_re.match(rid):
            errors.append("%s: ID no cumple el formato %s" % (where, rcfg["id_regex"]))
        if rid in seen:
            errors.append("%s: ID duplicado (también en línea %d)" % (where, seen[rid]))
        seen[rid] = r["linea"]
        f = r["campos"]
        for k in REQUIRED:
            if not f.get(k):
                errors.append("%s: falta el campo '%s'" % (where, FIELD_LABEL[k]))
        for k, vals in allowed.items():
            v = f.get(k)
            if v and c.norm(v) not in vals:
                errors.append("%s: %s='%s' no permitido (%s)" % (
                    where, FIELD_LABEL[k], v, ", ".join(rcfg[{"tipo": "types",
                                                              "prioridad": "priorities",
                                                              "verificacion": "verification",
                                                              "estado": "states"}[k]])))
        desc = f.get("descripcion", "")
        if desc and "debera" not in c.norm(desc):
            errors.append("%s: REQ-Q-02 la descripción debe usar 'deberá'" % where)
        if c.norm(desc).count("debera") > 1:
            warns.append("%s: REQ-Q-01 posible requisito compuesto (varios 'deberá')" % where)
        blob = c.norm(desc + " " + f.get("criterio", ""))
        for w in VAGUE:
            if re.search(r"(^|[^a-z])%s([^a-z]|$)" % re.escape(w), blob):
                warns.append("%s: REQ-Q-03 término vago '%s'" % (where, w))
        crit = f.get("criterio", "")
        if crit and not UNIT_RE.search(crit) and c.norm(f.get("verificacion", "")) in (
                "test", "analisis"):
            warns.append("%s: REQ-Q-04 el criterio no contiene magnitud con unidad" % where)
        if strict and c.norm(f.get("estado", "")) != "aprobado":
            errors.append("%s: Estado debe ser 'Aprobado' para cerrar" % where)
    for n, ln in c.find_markers(text):
        (errors if strict else warns).append("requisitos.md:%d marcador abierto: %s" % (n, ln))
    return errors, warns


def open_questions(root=None):
    """Filas de dudas.md cuyo estado no es Resuelta/Descartada."""
    root = root or c.project_root()
    path = root / DUDAS_FILE
    if not path.is_file():
        return []
    out = []
    for table in c.md_tables(path.read_text(encoding="utf-8")):
        if not table:
            continue
        header = [c.norm(h) for h in table[0]]
        if "estado" not in header:
            continue
        i_est = header.index("estado")
        i_id = header.index("id") if "id" in header else 0
        for row in table[1:]:
            if len(row) <= i_est:
                continue
            if c.norm(row[i_est]) not in ("resuelta", "descartada", "resuelto", "descartado"):
                out.append("%s (%s)" % (row[i_id], row[i_est] or "sin estado"))
    return out


def ids(root=None):
    return [r["id"] for r in load(root)]
