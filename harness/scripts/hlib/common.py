"""Utilidades comunes: rutas, configuración, estado, salida y ejecución de comandos.

Cumple STD-SH §2 (Python >= 3.9, solo biblioteca estándar).
"""
import datetime as _dt
import json
import os
import re
import shutil
import subprocess
import sys
import unicodedata
from pathlib import Path

HARNESS_HOME = Path(__file__).resolve().parent.parent.parent
CONFIG_NAME = "harness.config.json"
STATE_REL = Path(".harness") / "state.json"

EXIT_OK = 0
EXIT_FAIL = 1
EXIT_USAGE = 2
EXIT_DEP = 3

PHASES = ["req", "design", "impl", "offer"]
PHASE_LABEL = {
    "req": "Requisitos",
    "design": "Diseño",
    "impl": "Implementación",
    "offer": "Oferta",
}
PHASE_TAG = {"req": "req", "design": "design", "impl": "impl", "offer": "offer"}
PHASE_BRANCH = {
    "req": "docs/requisitos",
    "design": "docs/diseno",
    "impl": "docs/implementacion",
    "offer": "docs/oferta",
}

DEFAULT_CONFIG = {
    "project": {
        "name": "SAF · Grid de ocupación dinámico",
        "code": "SAF",
        "author": "",
        "client": "",
    },
    "module": "ogrid",
    "requirements": {
        "id_regex": "^REQ-[A-Z]{3}-[0-9]{3}$",
        "types": [
            "Funcional", "Rendimiento", "Seguridad funcional", "Interfaz",
            "Entorno", "Restricción", "No funcional",
        ],
        "priorities": ["Must", "Should", "Could", "Won't"],
        "verification": ["Test", "Análisis", "Inspección", "Demostración"],
        "states": ["Borrador", "En revisión", "Aprobado"],
    },
    "export": {
        "template": "@HARNESS@/templates/docx/plantilla-base.docx",
        "template_map": "@HARNESS@/templates/docx/plantilla-base.map.json",
        "decisions": [],
    },
    "impl": {
        "max_iterations": 5,
        "coverage_line": 100.0,
        "coverage_branch": 100.0,
        "cc_max": 10,
        "lines_max": 60,
        "params_max": 5,
        "cmake_preset": "host-test",
    },
    "git": {"remote": "origin", "push": True, "main": "main", "develop": "develop"},
}


# --------------------------------------------------------------------------- salida
def _ts():
    t = _dt.datetime.now(_dt.timezone.utc)
    return t.strftime("%Y-%m-%dT%H:%M:%S.") + "%03dZ" % (t.microsecond // 1000)


def log(level, msg, **kv):
    """Escribe una línea STD-LOG en stderr."""
    extra = " ".join('%s="%s"' % (k, v) if " " in str(v) else "%s=%s" % (k, v)
                     for k, v in kv.items())
    sys.stderr.write("%s %-6s h %s%s\n" % (_ts(), level, 'msg="%s"' % msg,
                                            (" " + extra) if extra else ""))


def info(msg):
    print(msg)


def die(msg, code=EXIT_FAIL):
    log("ERROR", msg)
    sys.exit(code)


def today():
    return _dt.date.today().isoformat()


def now_iso():
    return _dt.datetime.now().replace(microsecond=0).isoformat()


# --------------------------------------------------------------------------- texto
def norm(text):
    """Minúsculas sin tildes ni espacios extremos (para comparar claves)."""
    text = unicodedata.normalize("NFKD", text or "")
    text = "".join(c for c in text if not unicodedata.combining(c))
    return re.sub(r"\s+", " ", text).strip().lower()


def slug(text):
    return re.sub(r"[^a-z0-9]+", "-", norm(text)).strip("-")


# --------------------------------------------------------------------------- rutas
def project_root(start=None):
    """Raíz del proyecto: carpeta con harness.config.json, o raíz git, o cwd."""
    env = os.environ.get("H_PROJECT_ROOT")
    if env:
        return Path(env).resolve()
    here = Path(start or os.getcwd()).resolve()
    for p in [here] + list(here.parents):
        if (p / CONFIG_NAME).is_file():
            return p
    try:
        out = subprocess.run(["git", "rev-parse", "--show-toplevel"], cwd=str(here),
                             capture_output=True, text=True, check=True).stdout.strip()
        if out:
            return Path(out)
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    return here


def expand(path_str, root=None):
    """Sustituye @HARNESS@ y resuelve rutas relativas al proyecto."""
    s = str(path_str).replace("@HARNESS@", str(HARNESS_HOME))
    p = Path(os.path.expanduser(s))
    if not p.is_absolute():
        p = (root or project_root()) / p
    return p


def _deep_merge(base, over):
    out = dict(base)
    for k, v in over.items():
        if isinstance(v, dict) and isinstance(out.get(k), dict):
            out[k] = _deep_merge(out[k], v)
        else:
            out[k] = v
    return out


def load_config(root=None):
    root = root or project_root()
    cfg_path = root / CONFIG_NAME
    user = {}
    if cfg_path.is_file():
        user = json.loads(cfg_path.read_text(encoding="utf-8"))
    return _deep_merge(DEFAULT_CONFIG, user)


def save_json(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    tmp.replace(path)


def load_json(path, default=None):
    path = Path(path)
    if not path.is_file():
        return default
    return json.loads(path.read_text(encoding="utf-8"))


def default_state():
    st = {"version": 1, "phases": {}}
    for i, ph in enumerate(PHASES):
        st["phases"][ph] = {
            "status": "open" if i == 0 else "locked",
            "version": "0.1",
            "closed_at": None,
            "tag": None,
            "history": [],
        }
    return st


def load_state(root=None):
    root = root or project_root()
    st = load_json(root / STATE_REL)
    return st if st else default_state()


def save_state(state, root=None):
    root = root or project_root()
    save_json(root / STATE_REL, state)


# --------------------------------------------------------------------------- procesos
def which(tool):
    return shutil.which(tool)


def run(cmd, cwd=None, check=False, env=None, timeout=None):
    """Ejecuta un comando y devuelve CompletedProcess con stdout/stderr en texto."""
    merged = dict(os.environ)
    if env:
        merged.update(env)
    try:
        cp = subprocess.run(cmd, cwd=str(cwd) if cwd else None, capture_output=True,
                            text=True, env=merged, timeout=timeout)
    except FileNotFoundError:
        cp = subprocess.CompletedProcess(cmd, 127, "", "not found: %s" % cmd[0])
    if check and cp.returncode != 0:
        die("comando falló (%d): %s\n%s" % (cp.returncode, " ".join(cmd), cp.stderr.strip()))
    return cp


def tokens():
    return json.loads((HARNESS_HOME / "tokens" / "tokens.json").read_text(encoding="utf-8"))


MARKER_RE = re.compile(r"\bTBD\b|\bTODO\b|\[PENDIENTE[^\]]*\]|\[PROPUESTA[^\]]*\]|\?\?")


def find_markers(text):
    """Devuelve [(nº línea, línea)] con marcadores que impiden cerrar una fase."""
    hits = []
    in_code = False
    text = re.sub(r"<!--.*?-->", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    for n, line in enumerate(text.splitlines(), 1):
        if line.strip().startswith("```"):
            in_code = not in_code
            continue
        if in_code:
            continue
        if MARKER_RE.search(line):
            hits.append((n, line.strip()))
    return hits


def md_tables(text):
    """Extrae tablas markdown como listas de filas (lista de celdas), cabecera incluida."""
    tables, cur = [], []
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("|") and s.endswith("|"):
            cells = [c.strip() for c in s.strip("|").split("|")]
            if all(re.fullmatch(r":?-{2,}:?", c) for c in cells if c):
                continue
            cur.append(cells)
        else:
            if cur:
                tables.append(cur)
                cur = []
    if cur:
        tables.append(cur)
    return tables
