"""Fase de implementación: plan de funciones, contratos y sandboxes aislados.

Garantiza TST-IND-01..03: el implementador y el ingeniero de tests trabajan en
sandboxes separados y solo los scripts mueven sus resultados al repositorio.
"""
import re
import shutil
from pathlib import Path

from . import common as c

IMPL_DIR = Path("docs") / "03-implementacion"
PLAN_REL = IMPL_DIR / "plan.json"
SANDBOX_REL = Path(".harness") / "sandbox" / "current"
REPORTS_REL = Path(".harness") / "reports"
CANARY_NAME = ".h-canary"
CANARY_TEXT = "ISOLATION_BROKEN: si puedes leer este fichero, el aislamiento ha fallado. " \
              "Detente y avisa al orquestador.\n"
STATES = ["pendiente", "contrato", "en-curso", "integrada"]


# --------------------------------------------------------------------------- plan
def load_plan(root=None):
    root = root or c.project_root()
    return c.load_json(root / PLAN_REL, {"alcance": "", "modulo": "", "funciones": []})


def save_plan(plan, root=None):
    c.save_json((root or c.project_root()) / PLAN_REL, plan)


def get(fid, root=None, plan=None):
    plan = plan or load_plan(root)
    for f in plan.get("funciones", []):
        if f["id"] == fid:
            return f
    c.die("la función %s no está en %s" % (fid, PLAN_REL), c.EXIT_USAGE)
    return None


def validate_plan(plan, root=None):
    errs = []
    ids = [f.get("id") for f in plan.get("funciones", [])]
    if len(set(ids)) != len(ids):
        errs.append("IDs de función duplicados")
    req_ids = set()
    try:
        from . import reqs
        req_ids = set(reqs.ids(root))
    except Exception:  # noqa: BLE001
        pass
    for i, f in enumerate(plan.get("funciones", [])):
        for k in ("id", "nombre", "requisitos", "contrato", "cabecera", "fuente", "test"):
            if not f.get(k):
                errs.append("%s: falta '%s'" % (f.get("id", "#%d" % i), k))
        if f.get("id") and not re.match(r"^F-\d{3}$", f["id"]):
            errs.append("%s: el ID debe ser F-NNN" % f["id"])
        if f.get("nombre") and not re.match(r"^[a-z][a-z0-9_]*$", f["nombre"]):
            errs.append("%s: nombre C-NAM-01 inválido" % f["id"])
        for r in f.get("requisitos", []):
            if req_ids and r not in req_ids:
                errs.append("%s: requisito desconocido %s" % (f["id"], r))
        for d in f.get("depende_de", []):
            if d not in ids[:i]:
                errs.append("%s: depende de %s, que debe ir antes en el plan" % (f["id"], d))
        if f.get("estado", "pendiente") not in STATES:
            errs.append("%s: estado '%s' no válido" % (f["id"], f.get("estado")))
    return errs


def next_function(root=None):
    """Primera función no integrada; exige que todas las anteriores estén integradas."""
    plan = load_plan(root)
    for f in plan.get("funciones", []):
        if f.get("estado") != "integrada":
            return f
    return None


def set_state(fid, estado, root=None):
    if estado not in STATES:
        c.die("estado no válido: %s" % estado, c.EXIT_USAGE)
    plan = load_plan(root)
    f = get(fid, plan=plan)
    order = [x["id"] for x in plan["funciones"]]
    if estado in ("en-curso", "integrada"):
        for prev in plan["funciones"][:order.index(fid)]:
            if prev.get("estado") != "integrada":
                c.die("GIT-BR-05: %s no está integrada; no se puede avanzar %s" % (prev["id"], fid))
    f["estado"] = estado
    save_plan(plan, root)
    c.info("%s → %s" % (fid, estado))


def branch_name(fid, root=None):
    f = get(fid, root)
    req = (f.get("requisitos") or ["NOREQ"])[0]
    return "feature/%s-%s" % (req, f["nombre"].replace("_", "-"))


def function_for_branch(branch, root=None):
    for f in load_plan(root).get("funciones", []):
        if branch_name(f["id"], root) == branch or f.get("rama") == branch:
            return f["id"]
    return None


def last_run_passed(fid, commit, root=None):
    root = root or c.project_root()
    rep = c.load_json(root / REPORTS_REL / fid / "latest.json")
    return bool(rep) and rep.get("result") == "PASS" and rep.get("commit") == commit


# --------------------------------------------------------------------------- sandbox
def _copy(src, dst):
    if not src.exists():
        return
    if src.is_dir():
        shutil.copytree(str(src), str(dst), dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns(CANARY_NAME, "*.gcda", "*.gcno"))
    else:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(str(src), str(dst))


def _common_inputs(root, f, dst):
    _copy(root / f["contrato"], dst / "contrato" / Path(f["contrato"]).name)
    _copy(root / "include", dst / "include")
    _copy(root / "docs" / "01-requisitos" / "requisitos.md", dst / "docs" / "requisitos.md")
    _copy(root / "docs" / "02-diseno" / "diseno.md", dst / "docs" / "diseno.md")
    _copy(root / "docs" / "02-diseno" / "diagramas", dst / "docs" / "diagramas")
    _copy(root / "docs" / "glosario.md", dst / "docs" / "glosario.md")
    _copy(root / "config", dst / "config")
    _copy(c.HARNESS_HOME / "standards", dst / "standards")


def sandbox_dir(role, root=None):
    return (root or c.project_root()) / SANDBOX_REL / role


def prepare(fid, keep=False, root=None):
    root = root or c.project_root()
    f = get(fid, root)
    for k in ("contrato", "cabecera"):
        if not (root / f[k]).is_file():
            c.die("falta %s (%s); el orquestador debe crearlo y el usuario aprobarlo" % (k, f[k]))
    base = root / SANDBOX_REL
    cur = c.load_json(base / "current.json", {})
    if base.exists() and (not keep or cur.get("fid") != fid):
        shutil.rmtree(str(base))
    base.mkdir(parents=True, exist_ok=True)
    c.save_json(base / "current.json", {"fid": fid, "nombre": f["nombre"]})
    t, i = base / "test", base / "impl"
    for d in (t, i):
        d.mkdir(exist_ok=True)
        _common_inputs(root, f, d)
    # ingeniero-tests: tests existentes y datos, nunca src/
    _copy(root / "tests", t / "tests")
    (t / "out" / "tests" / "unit").mkdir(parents=True, exist_ok=True)
    (t / "out" / "tests" / "data").mkdir(parents=True, exist_ok=True)
    existing_test = root / f["test"]
    if existing_test.is_file() and not (t / "out" / f["test"]).is_file():
        _copy(existing_test, t / "out" / f["test"])
    # implementador: src existente (funciones previas), nunca tests/
    _copy(root / "src", i / "src")
    (i / "out").mkdir(exist_ok=True)
    (i / "feedback").mkdir(exist_ok=True)
    (t / "feedback").mkdir(exist_ok=True)
    existing_src = root / f["fuente"]
    if existing_src.is_file() and not (i / "out" / f["fuente"]).is_file():
        _copy(existing_src, i / "out" / f["fuente"])
    # canarios en las zonas prohibidas de cada rol
    for zone in ("tests", "src"):
        (root / zone).mkdir(exist_ok=True)
        (root / zone / CANARY_NAME).write_text(CANARY_TEXT, encoding="utf-8")
    _write_brief(root, f, t, "test")
    _write_brief(root, f, i, "impl")
    c.info("sandbox preparado para %s en %s" % (fid, base.relative_to(root)))
    c.info("  test: %s\n  impl: %s" % (t.relative_to(root), i.relative_to(root)))
    return c.EXIT_OK


def _write_brief(root, f, d, role):
    canary = root / ("src" if role == "test" else "tests") / CANARY_NAME
    out = ("out/" + f["test"]) if role == "test" else ("out/" + f["fuente"])
    lines = [
        "# Encargo %s · %s (%s)" % ("de tests" if role == "test" else "de implementación",
                                    f["id"], f["nombre"]),
        "",
        "- Sandbox (única carpeta de trabajo): `%s`" % d,
        "- Canario que DEBE estar bloqueado: `%s`" % canary,
        "- Contrato: `contrato/%s`" % Path(f["contrato"]).name,
        "- Cabecera pública (solo lectura): `include/`",
        "- Requisitos: %s" % ", ".join(f.get("requisitos", [])),
        "- Fichero a entregar: `%s`" % out,
    ]
    if role == "test":
        lines.append("- Lista de casos a entregar: `out/casos-%s.md`" % f["id"])
        lines.append("- Datos de test (opcional): `out/tests/data/`")
    lines.append("- Feedback del orquestador (si hay iteraciones): `feedback/`")
    (d / "ENCARGO.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def current(root=None):
    root = root or c.project_root()
    return c.load_json(root / SANDBOX_REL / "current.json", {})


def collect(role, root=None, message=None):
    """Copia la entrega del sandbox al repo y la commitea en la rama feature."""
    from . import gitflow
    root = root or c.project_root()
    cur = current(root)
    if not cur:
        c.die("no hay sandbox activo (h sandbox prepare F-NNN)")
    fid = cur["fid"]
    f = get(fid, root)
    branch = gitflow.current_branch(root)
    integration = branch.startswith("test/")
    if not branch.startswith("feature/") and not (integration and role == "test"):
        c.die("GIT-BR-05: la entrega se commitea en la rama feature de %s (o test/* para tests de "
              "integración), no en %s" % (fid, branch))
    sb = root / SANDBOX_REL / role / "out"
    mod = load_plan(root).get("modulo") or c.load_config(root)["module"]
    paths = []
    if role == "test" and integration:
        for sub in ("integration", "data"):
            d = sb / "tests" / sub
            if d.is_dir() and any(d.iterdir()):
                _copy(d, root / "tests" / sub)
                paths.append("tests/" + sub)
        if not paths:
            c.die("el ingeniero-tests no ha entregado out/tests/integration/")
        kind = "test"
        default_msg = "add integration tests"
    elif role == "test":
        src = sb / f["test"]
        if not src.is_file():
            c.die("el ingeniero-tests no ha entregado %s" % f["test"])
        _copy(src, root / f["test"])
        paths.append(f["test"])
        data = sb / "tests" / "data"
        if data.is_dir() and any(data.iterdir()):
            _copy(data, root / "tests" / "data")
            paths.append("tests/data")
        integ = sb / "tests" / "integration"
        if integ.is_dir() and any(integ.iterdir()):
            _copy(integ, root / "tests" / "integration")
            paths.append("tests/integration")
        casos = sb / ("casos-%s.md" % fid)
        if casos.is_file():
            dst = root / IMPL_DIR / "casos" / casos.name
            _copy(casos, dst)
            paths.append(str(dst.relative_to(root)))
        kind = "test"
        default_msg = "add unit tests for %s" % f["nombre"]
    else:
        src = sb / f["fuente"]
        if not src.is_file():
            c.die("el implementador no ha entregado %s" % f["fuente"])
        _copy(src, root / f["fuente"])
        paths.append(f["fuente"])
        prev = c.run(["git", "log", "--oneline", "--", f["fuente"]], cwd=root).stdout.strip()
        kind = "fix" if prev else "feat"
        default_msg = ("implement %s" % f["nombre"]) if kind == "feat" else (
            "correct %s after test feedback" % f["nombre"])
    gitflow.git("add", "--", *paths, root=root)
    if gitflow.git("diff", "--cached", "--name-only", root=root):
        body = "Refs: %s" % ", ".join(f.get("requisitos", []) + [fid])
        gitflow.git("commit", "-m", "%s(%s): %s" % (kind, mod, message or default_msg),
                    "-m", body, root=root)
        c.info("commit %s(%s) en %s" % (kind, mod, branch))
    else:
        c.info("sin cambios que commitear")
    return c.EXIT_OK


def syntax(path, role, root=None):
    """Compilación de comprobación permitida dentro del sandbox (sin enlazar)."""
    root = root or c.project_root()
    sb = root / SANDBOX_REL / role
    p = Path(path)
    p = p if p.is_absolute() else sb / p
    if sb.resolve() not in p.resolve().parents:
        c.die("solo se compilan ficheros dentro del sandbox %s" % role, c.EXIT_USAGE)
    cc = c.which("cc") or c.which("gcc") or c.which("clang")
    inc = ["-I", str(sb / "include"), "-I", str(sb / "src"), "-I", str(sb / "out" / "src")]
    mod_dirs = [d for d in (sb / "src").glob("*") if d.is_dir()] if (sb / "src").is_dir() else []
    for d in mod_dirs:
        inc += ["-I", str(d)]
    unity = root / "third_party" / "unity" / "src"
    if role == "test":
        inc += ["-I", str(unity)]
    flags = ["-std=c11", "-pedantic", "-Wall", "-Wextra", "-Wconversion", "-Wshadow",
             "-Wstrict-prototypes", "-Werror", "-fsyntax-only"]
    if role == "test":
        flags += ["-DUNITY_INCLUDE_DOUBLE", "-DUNITY_INCLUDE_FLOAT"]
    cp = c.run([cc] + flags + inc + [str(p)])
    out = (cp.stdout + cp.stderr).replace(str(root) + "/", "")
    print(out.strip() or "OK: compila sin avisos")
    return c.EXIT_OK if cp.returncode == 0 else c.EXIT_FAIL


def check_impl(root=None):
    """Autocomprobación de estándares para el implementador (solo su fichero)."""
    from . import testrun
    root = root or c.project_root()
    cur = current(root)
    if not cur:
        c.die("no hay sandbox activo")
    f = get(cur["fid"], root)
    sb = root / SANDBOX_REL / "impl"
    src = sb / "out" / f["fuente"]
    if not src.is_file():
        c.die("aún no existe %s" % src.relative_to(root))
    rc = syntax(src, "impl", root)
    res = testrun.static_checks(src, [sb / "include", sb / "src", src.parent], root,
                                header=sb / f["cabecera"], func=f["nombre"])
    for name, chk in res.items():
        print("[%s] %s" % ("OK" if chk["ok"] else "FALLO", name))
        for d in chk["details"][:30]:
            print("    - %s" % str(d).replace(str(sb) + "/", ""))
    ok = rc == 0 and all(x["ok"] for x in res.values())
    return c.EXIT_OK if ok else c.EXIT_FAIL


def cleanup(root=None):
    root = root or c.project_root()
    base = root / SANDBOX_REL
    if base.exists():
        shutil.rmtree(str(base))
    for zone in ("tests", "src"):
        p = root / zone / CANARY_NAME
        if p.exists():
            p.unlink()
    c.info("sandbox eliminado")
    return c.EXIT_OK
