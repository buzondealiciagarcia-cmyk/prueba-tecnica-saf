"""Diagramas Mermaid con el tema del harness (STD-UI §7).

`.mmd` -> `.svg` (para markdown, versionado) y `.png` 3x (para Word/PowerPoint).
El SVG lleva el hash de su fuente para detectar diagramas desactualizados.
"""
import hashlib
import json
import tempfile
from pathlib import Path

from . import common as c

CLASSDEFS = [
    "classDef hl fill:{accentTint},stroke:{accent},stroke-width:2px,color:{text}",
    "classDef warn fill:#FFFFFF,stroke:{warning},stroke-width:2px,color:{text}",
    "classDef err fill:#FFFFFF,stroke:{error},stroke-width:2px,color:{text}",
    "classDef muted fill:#FFFFFF,stroke:{border},color:{textMuted}",
]
HASH_PREFIX = "<!-- h-src-sha256: "


def _config_path():
    return c.HARNESS_HOME / "tokens" / "mermaid-config.json"


def source_hash(mmd_text):
    h = hashlib.sha256()
    h.update(mmd_text.encode("utf-8"))
    h.update(_config_path().read_bytes())
    return h.hexdigest()


def prepare_source(text):
    """Añade las classDef del harness a flowcharts (UI-DIA-02)."""
    first = ""
    for line in text.splitlines():
        s = line.strip()
        if s and not s.startswith("%%"):
            first = s.split()[0]
            break
    if first in ("flowchart", "graph"):
        pal = c.tokens()["color"]
        extra = "\n".join("    " + d.format(**pal) for d in CLASSDEFS)
        return text.rstrip() + "\n" + extra + "\n"
    return text


def _puppeteer_cfg(tmpdir):
    p = Path(tmpdir) / "puppeteer.json"
    p.write_text(json.dumps({"args": ["--no-sandbox", "--disable-setuid-sandbox"]}))
    return p


def render_text(text, out_path, fmt=None, scale=3):
    """Renderiza código Mermaid a out_path (.svg o .png). Devuelve (ok, error)."""
    mmdc = c.which("mmdc")
    if not mmdc:
        return False, "mmdc no encontrado (npm i -g @mermaid-js/mermaid-cli); ver h doctor"
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as td:
        src = Path(td) / "in.mmd"
        src.write_text(prepare_source(text), encoding="utf-8")
        cmd = [mmdc, "-i", str(src), "-o", str(out_path), "-c", str(_config_path()),
               "-b", "white", "-p", str(_puppeteer_cfg(td))]
        if (fmt or out_path.suffix.lstrip(".")) == "png":
            cmd += ["-s", str(scale)]
        cp = c.run(cmd, timeout=180)
        if cp.returncode != 0 or not out_path.is_file():
            return False, (cp.stderr or cp.stdout).strip()[-800:]
    return True, ""


def render_file(mmd):
    """Renderiza un .mmd a .svg hermano con hash de la fuente."""
    mmd = Path(mmd)
    text = mmd.read_text(encoding="utf-8")
    svg = mmd.with_suffix(".svg")
    ok, err = render_text(text, svg)
    if not ok:
        return False, err
    body = svg.read_text(encoding="utf-8")
    svg.write_text("%s%s -->\n%s" % (HASH_PREFIX, source_hash(text), body), encoding="utf-8")
    return True, ""


def is_fresh(mmd):
    mmd = Path(mmd)
    svg = mmd.with_suffix(".svg")
    if not svg.is_file():
        return False
    with svg.open(encoding="utf-8") as fh:
        first = fh.readline()
    if not first.startswith(HASH_PREFIX):
        return False
    return first[len(HASH_PREFIX):].split()[0] == source_hash(mmd.read_text(encoding="utf-8"))


def png_for(mmd, cache_dir):
    """PNG a 3x en caché (para exportación); se regenera si cambia la fuente."""
    mmd = Path(mmd)
    text = mmd.read_text(encoding="utf-8")
    png = Path(cache_dir) / ("%s-%s.png" % (mmd.stem, source_hash(text)[:12]))
    if png.is_file():
        return png, ""
    ok, err = render_text(text, png)
    return (png if ok else None), err


def render_all(paths, only_stale=True):
    root = c.project_root()
    files = []
    for p in paths or ["docs"]:
        p = Path(p)
        p = p if p.is_absolute() else root / p
        if p.is_file() and p.suffix == ".mmd":
            files.append(p)
        elif p.is_dir():
            files.extend(sorted(p.rglob("*.mmd")))
    failed = 0
    for f in files:
        if only_stale and is_fresh(f):
            continue
        ok, err = render_file(f)
        rel = f.relative_to(root) if root in f.parents else f
        if ok:
            c.info("renderizado %s" % rel)
        else:
            failed += 1
            c.log("ERROR", "no se pudo renderizar", file=rel, detail=err.splitlines()[-1:] or "")
    return c.EXIT_FAIL if failed else c.EXIT_OK


def check(paths):
    root = c.project_root()
    stale = []
    for p in paths or ["docs"]:
        p = root / p
        for f in (sorted(p.rglob("*.mmd")) if p.is_dir() else [p]):
            if not is_fresh(f):
                stale.append(str(f.relative_to(root)))
    for s in stale:
        c.info("desactualizado: %s" % s)
    return c.EXIT_FAIL if stale else c.EXIT_OK
