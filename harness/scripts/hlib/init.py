"""Inicialización del proyecto, andamiaje de fases y diagnóstico de dependencias."""
import platform
import shutil
from pathlib import Path

from . import common as c

TPL = c.HARNESS_HOME / "templates"
GITIGNORE_BLOCK = """
# --- harness (STD-GIT §6) ---
/build/
/.harness/sandbox/
/.harness/reports/
/.harness/audit/
/docs/export/
.h-canary
*.gcda
*.gcno
/.claude/
.DS_Store
"""
UNITY = ("https://github.com/ThrowTheSwitch/Unity", "v2.6.0")
CMOCK = ("https://github.com/ThrowTheSwitch/CMock", "v2.6.0")


def _render(text, values):
    for k, v in values.items():
        text = text.replace("@%s@" % k, str(v))
    return text


def copy_template(rel_src, dst, values=None, overwrite=False):
    src = TPL / rel_src
    dst = Path(dst)
    if dst.exists() and not overwrite:
        return False
    dst.parent.mkdir(parents=True, exist_ok=True)
    try:
        dst.write_text(_render(src.read_text(encoding="utf-8"), values or {}), encoding="utf-8")
    except UnicodeDecodeError:
        shutil.copy2(str(src), str(dst))
    if src.suffix == ".sh" or (src.stat().st_mode & 0o111):
        dst.chmod(0o755)
    return True


def _values(root):
    cfg = c.load_config(root)
    mod = cfg["module"]
    return {"PROJECT": cfg["project"]["name"], "CODE": cfg["project"]["code"],
            "AUTHOR": cfg["project"]["author"], "DATE": c.today(), "MOD": mod,
            "MOD_UPPER": mod.upper()}


def init(name=None, code=None, author=None, module=None, client=None):
    root = c.project_root()
    cfg_path = root / c.CONFIG_NAME
    cfg = c.load_json(cfg_path, {})
    proj = cfg.setdefault("project", {})
    for k, v in (("name", name), ("code", code), ("author", author), ("client", client)):
        if v:
            proj[k] = v
    if module:
        cfg["module"] = module
    for k, v in c.DEFAULT_CONFIG["project"].items():
        proj.setdefault(k, v)
    cfg.setdefault("module", c.DEFAULT_CONFIG["module"])
    c.save_json(cfg_path, cfg)
    if not (root / c.STATE_REL).is_file():
        c.save_state(c.default_state(), root)
    vals = _values(root)
    for rel, dst in (("docs/glosario.md", "docs/glosario.md"),
                     ("docs/requisitos.md", "docs/01-requisitos/requisitos.md"),
                     ("docs/dudas.md", "docs/01-requisitos/dudas.md")):
        copy_template(rel, root / dst, vals)
    (root / "docs" / "01-requisitos" / "entradas").mkdir(parents=True, exist_ok=True)
    gi = root / ".gitignore"
    text = gi.read_text(encoding="utf-8") if gi.is_file() else ""
    if "--- harness" not in text:
        gi.write_text(text.rstrip() + "\n" + GITIGNORE_BLOCK, encoding="utf-8")
    hooks_dst = root / ".githooks"
    hooks_dst.mkdir(exist_ok=True)
    for h in (c.HARNESS_HOME / "git-hooks").iterdir():
        if h.is_file():
            shutil.copy2(str(h), str(hooks_dst / h.name))
            (hooks_dst / h.name).chmod(0o755)
    if (root / ".git").exists():
        c.run(["git", "config", "core.hooksPath", ".githooks"], cwd=root)
        from . import gitflow
        gitflow.ensure_develop(root)
        if c.run(["git", "status", "--porcelain"], cwd=root).stdout.strip():
            branch = "harness/project-setup"
            if gitflow.current_branch(root) != branch:
                if gitflow.branch_exists(branch, root):
                    gitflow.git("checkout", branch, root=root)
                else:
                    gitflow.git("checkout", "-b", branch, gitflow.names(root)[1], root=root)
            gitflow.git("add", "-A", root=root)
            gitflow.git("commit", "-m", "chore(harness): initialize project structure", root=root)
            gitflow.finish(branch, root=root)
    c.info("proyecto inicializado en %s" % root)
    c.info("siguiente paso: /req-iniciar <requisitos>")
    return c.EXIT_OK


def scaffold(kind):
    """Crea el esqueleto de una fase (c-project, design, impl, offer, deploy, frontend)."""
    root = c.project_root()
    vals = _values(root)
    created = []
    mapping = {
        "design": [("docs/diseno.md", "docs/02-diseno/diseno.md"),
                   ("docs/revision.md", "docs/02-diseno/revision.md")],
        "impl": [("docs/implementacion.md", "docs/03-implementacion/implementacion.md"),
                 ("docs/plan.json", "docs/03-implementacion/plan.json")],
        "offer": [("docs/oferta.md", "docs/04-oferta/oferta.md"),
                  ("docs/presupuesto.json", "docs/04-oferta/presupuesto.json"),
                  ("docs/presentacion.json", "docs/04-oferta/presentacion.json")],
        "dossier": [("docs/contexto-dosier.md", "docs/dosier/contexto.md")],
    }
    if kind in mapping:
        for s, d in mapping[kind]:
            if copy_template(s, root / d, vals):
                created.append(d)
    elif kind in ("c-project", "deploy", "frontend"):
        base = TPL / kind
        for src in sorted(base.rglob("*")):
            if src.is_file():
                rel = str(src.relative_to(base)).replace("MOD", vals["MOD"])
                if copy_template(str(src.relative_to(TPL)), root / rel, vals):
                    created.append(rel)
        if kind == "c-project":
            for d in ("include/%s" % vals["MOD"], "src/%s" % vals["MOD"], "src/hal", "src/app",
                      "tests/unit", "tests/integration", "tests/data"):
                (root / d).mkdir(parents=True, exist_ok=True)
                keep = root / d / ".gitkeep"
                if not any((root / d).iterdir()):
                    keep.write_text("")
            fetch_third_party(root)
    else:
        c.die("tipo de andamiaje desconocido: %s" % kind, c.EXIT_USAGE)
    for x in created:
        c.info("creado %s" % x)
    return c.EXIT_OK


def fetch_third_party(root):
    for name, (url, tag) in (("unity", UNITY), ("cmock", CMOCK)):
        dst = root / "third_party" / name
        if dst.exists():
            continue
        cp = c.run(["git", "clone", "-q", "--depth", "1", "--branch", tag, url, str(dst)],
                   timeout=180)
        if cp.returncode != 0:
            c.log("WARN", "no se pudo descargar %s %s; descárgalo manualmente en %s" % (name, tag, dst))
            continue
        shutil.rmtree(str(dst / ".git"), ignore_errors=True)
        (dst / "VERSION.harness").write_text("%s %s\n" % (url, tag))
        c.info("descargado %s %s" % (name, tag))


def doctor():
    mac = platform.system() == "Darwin"
    rows = []
    tools = [
        ("git", "obligatorio", "brew install git"),
        ("python3", "obligatorio", "xcode-select --install"),
        ("pandoc", "export Word", "brew install pandoc"),
        ("mmdc", "diagramas", "npm install -g @mermaid-js/mermaid-cli"),
        ("cmake", "build C", "brew install cmake"),
        ("cppcheck", "MISRA", "brew install cppcheck"),
        ("clang-tidy", "análisis estático", "brew install llvm  (y añade $(brew --prefix llvm)/bin al PATH)"),
        ("clang-format", "formato", "brew install clang-format"),
        ("gcovr", "cobertura", "pip3 install --user gcovr"),
        ("lizard", "complejidad", "pip3 install --user lizard"),
        ("shellcheck", "STD-SH", "brew install shellcheck"),
        ("doxygen", "doc. de código", "brew install doxygen"),
        ("docker", "despliegue", "Docker Desktop"),
        ("mosquitto", "broker local (opcional)", "brew install mosquitto"),
        ("ruby", "CMock (mocks)", "incluido en macOS"),
        ("aarch64-linux-gnu-gcc", "build cruzado (opcional)", "brew install aarch64-unknown-linux-gnu"),
    ]
    missing = 0
    for t, use, how in tools:
        found = c.which(t)
        if not found and t == "clang-tidy" and mac and Path("/opt/homebrew/opt/llvm/bin/clang-tidy").is_file():
            found = "/opt/homebrew/opt/llvm/bin/clang-tidy"
        rows.append((t, use, "OK" if found else "FALTA", "" if found else how))
        if not found and use in ("obligatorio", "export Word", "build C", "MISRA", "formato",
                                 "cobertura", "complejidad"):
            missing += 1
    for mod, pkg, use in (("docx", "python-docx", "export Word"), ("pptx", "python-pptx", "PowerPoint"),
                          ("paho.mqtt", "paho-mqtt", "herramientas MQTT (opcional)")):
        try:
            __import__(mod)
            rows.append((pkg, use, "OK", ""))
        except ImportError:
            rows.append((pkg, use, "FALTA", "pip3 install --user %s" % pkg))
            if "opcional" not in use:
                missing += 1
    print("| Herramienta | Uso | Estado | Instalación |\n|---|---|---|---|")
    for r in rows:
        print("| %s | %s | %s | %s |" % r)
    print("\n%s" % ("Todo lo necesario está instalado." if not missing else
                    "%d dependencias necesarias ausentes." % missing))
    return c.EXIT_OK if not missing else c.EXIT_DEP
