"""Ejecución de tests unitarios, de integración y de estándares (STD-TST §3-5).

`h test F-NNN`  -> build host-test, test unitario, cobertura del fichero, MISRA,
                   clang-tidy, formato, complejidad, Doxygen y símbolos prohibidos.
`h test --all`  -> lo mismo para todo el árbol + tests de integración.
Genera informe JSON, feedback saneado para cada agente y entrada en log-tests.md.
"""
import json
import os
import platform
import re
from pathlib import Path

from . import common as c
from . import impl

LOG_REL = Path("docs") / "03-implementacion" / "log-tests.md"
UNITY_LINE = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<test>\w+):(?P<status>PASS|FAIL|IGNORE)"
                        r"(?::\s*(?P<msg>.*))?$")
TC_RE = re.compile(r"\[(TC-F\d{3}-\d{2})\]")
REQ_IN_MSG = re.compile(r"\[([A-Z][A-Z0-9]*-[A-Z0-9-]+)\]")
FORBIDDEN = ["malloc", "calloc", "realloc", "free", "alloca", "printf", "fprintf", "sprintf",
             "vsprintf", "scanf", "sscanf", "fscanf", "atoi", "atof", "atol", "gets", "strcpy",
             "strcat", "setjmp", "longjmp", "signal", "rand", "srand", "time", "exit", "abort"]
DOX_TAGS_FUNC = ["@brief", "@return", "@pre", "@post", "@req", "@contract"]


# --------------------------------------------------------------------------- utilidades
def _strip_comments(code):
    code = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), code, flags=re.S)
    code = re.sub(r"//[^\n]*", "", code)
    code = re.sub(r'"(\\.|[^"\\])*"', '""', code)
    return code


def _chk(ok, details=None, extra=None):
    d = {"ok": bool(ok), "details": details or []}
    if extra:
        d.update(extra)
    return d


def _cmake_preset_available(root, preset):
    p = root / "CMakePresets.json"
    if not p.is_file():
        return False
    data = json.loads(p.read_text(encoding="utf-8"))
    return any(x.get("name") == preset for x in data.get("configurePresets", []))


def build(root, preset):
    bdir = root / "build" / preset
    if _cmake_preset_available(root, preset):
        cfg = c.run(["cmake", "--preset", preset], cwd=root)
    else:
        cfg = c.run(["cmake", "-S", ".", "-B", str(bdir), "-DH_TEST=ON", "-DH_COVERAGE=ON",
                     "-DH_SANITIZE=ON", "-DCMAKE_BUILD_TYPE=Debug"], cwd=root)
    if cfg.returncode != 0:
        return False, bdir, (cfg.stdout + cfg.stderr)[-4000:]
    b = c.run(["cmake", "--build", str(bdir), "-j", str(os.cpu_count() or 2)], cwd=root)
    return b.returncode == 0, bdir, (b.stdout + b.stderr)[-6000:]


def _compiler_id(bdir):
    cache = bdir / "CMakeCache.txt"
    if cache.is_file():
        m = re.search(r"CMAKE_C_COMPILER_ID:\w+=(\w+)", cache.read_text(errors="ignore"))
        if m:
            return m.group(1)
        m = re.search(r"CMAKE_C_COMPILER:\w+=(.+)", cache.read_text(errors="ignore"))
        if m and "clang" in m.group(1):
            return "Clang"
    return "GNU"


def _gcov_exe(bdir):
    if _compiler_id(bdir) in ("Clang", "AppleClang"):
        if platform.system() == "Darwin":
            return "xcrun llvm-cov gcov"
        for cand in ("llvm-cov",):
            if c.which(cand):
                return "llvm-cov gcov"
    return "gcov"


def _clear_gcda(bdir):
    for p in bdir.rglob("*.gcda"):
        p.unlink()


ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def parse_unity(output):
    tests, failures = [], []
    for line in ANSI_RE.sub("", output).splitlines():
        m = UNITY_LINE.match(line.strip())
        if not m:
            continue
        t = {"test": m.group("test"), "status": m.group("status"), "msg": (m.group("msg") or "")}
        tests.append(t)
        if t["status"] == "FAIL":
            tc = TC_RE.search(t["msg"]) or TC_RE.search(t["test"].replace("_", "-"))
            reqs = [r for r in REQ_IN_MSG.findall(t["msg"]) if not r.startswith("TC-")]
            failures.append({"tc": tc.group(1) if tc else t["test"], "reqs": reqs,
                             "detail": t["msg"]})
    return tests, failures


def _tc_from_name(name):
    m = re.match(r"test_TC_F(\d{3})_(\d{2})", name)
    return "TC-F%s-%s" % (m.group(1), m.group(2)) if m else name


def run_binary(exe, root):
    cp = c.run([str(exe)], cwd=root, timeout=300,
               env={"ASAN_OPTIONS": "detect_leaks=0:abort_on_error=0",
                    "UBSAN_OPTIONS": "print_stacktrace=1:halt_on_error=1"})
    out = cp.stdout + "\n" + cp.stderr
    tests, failures = parse_unity(out)
    for f in failures:
        if not f["tc"].startswith("TC-"):
            f["tc"] = _tc_from_name(f["tc"])
    san = []
    for line in out.splitlines():
        if "runtime error:" in line or "ERROR: AddressSanitizer" in line or \
                "SUMMARY: AddressSanitizer" in line or "SUMMARY: UndefinedBehaviorSanitizer" in line:
            san.append(line.strip())
    crashed = cp.returncode != 0 and not failures
    return {"tests": tests, "failures": failures, "sanitizer": san, "rc": cp.returncode,
            "crashed": crashed, "raw_tail": out[-3000:]}


def coverage(root, bdir, files):
    if not c.which("gcovr"):
        return _chk(False, ["gcovr no instalado (pip3 install --user gcovr)"])
    cmd = ["gcovr", "-r", str(root), "--object-directory", str(bdir), "--json-summary",
           "-", "--exclude-unreachable-branches", "--gcov-executable", _gcov_exe(bdir)]
    for f in files:
        cmd += ["--filter", re.escape(str(Path(f).as_posix()))]
    cp = c.run(cmd, cwd=root)
    if cp.returncode != 0:
        return _chk(False, ["gcovr falló: %s" % cp.stderr.strip()[-500:]])
    data = json.loads(cp.stdout or "{}")
    cfg = c.load_config(root)["impl"]
    details, ok = [], True
    files_out = data.get("files", [])
    if not files_out:
        return _chk(False, ["sin datos de cobertura para %s" % ", ".join(map(str, files))])
    for fd in files_out:
        lp = float(fd.get("line_percent") or 0.0)
        bp = float(fd.get("branch_percent") if fd.get("branch_total") else 100.0)
        if lp < cfg["coverage_line"] or bp < cfg["coverage_branch"]:
            ok = False
            details.append("%s: líneas %.1f %% (mín %.0f), ramas %.1f %% (mín %.0f)" % (
                fd["filename"], lp, cfg["coverage_line"], bp, cfg["coverage_branch"]))
    unc = []
    if not ok:
        cpd = c.run(["gcovr", "-r", str(root), "--object-directory", str(bdir), "--json", "-",
                     "--exclude-unreachable-branches", "--gcov-executable", _gcov_exe(bdir)]
                    + sum([["--filter", re.escape(str(f))] for f in files], []), cwd=root)
        try:
            for fd in json.loads(cpd.stdout).get("files", []):
                for ln in fd.get("lines", []):
                    if ln.get("count", 0) == 0 and not ln.get("gcovr/noncode"):
                        unc.append("%s:%d línea no ejecutada" % (fd["file"], ln["line_number"]))
                    for b in ln.get("branches", []):
                        if b.get("count", 0) == 0:
                            unc.append("%s:%d rama no ejecutada" % (fd["file"], ln["line_number"]))
                            break
        except (ValueError, KeyError):
            pass
    return _chk(ok, details, {"summary": {fd["filename"]: {
        "line": fd.get("line_percent"), "branch": fd.get("branch_percent")} for fd in files_out},
        "uncovered": unc[:200]})


def misra(files, includes, root):
    if not c.which("cppcheck"):
        return _chk(False, ["cppcheck no instalado"])
    cmd = ["cppcheck", "--addon=misra", "--enable=warning,style,performance,portability",
           "--std=c11", "--inline-suppr", "--quiet", "--error-exitcode=1",
           "--template={file}:{line}:{severity}:{id}:{message}"]
    sup = root / "config" / "cppcheck-suppressions.txt"
    if sup.is_file():
        cmd.append("--suppressions-list=%s" % sup)
    rules = root / "config" / "misra-rules.txt"
    if rules.is_file():
        addon_cfg = root / "build" / "misra-addon.json"
        addon_cfg.parent.mkdir(parents=True, exist_ok=True)
        addon_cfg.write_text(json.dumps({"script": "misra", "args": ["--rule-texts=%s" % rules]}))
        cmd[1] = "--addon=%s" % addon_cfg
    for inc in includes:
        cmd += ["-I", str(inc)]
    cp = c.run(cmd + [str(f) for f in files], cwd=root, timeout=600)
    lines = [ln for ln in (cp.stderr + cp.stdout).splitlines()
             if re.match(r"^[^:]+:\d+:\w+:", ln) and "missingInclude" not in ln
             and ":information:" not in ln]
    if cp.returncode != 0 and not lines:
        lines = ["cppcheck terminó con código %d: %s" % (cp.returncode,
                                                          (cp.stderr or cp.stdout).strip()[-500:])]
    return _chk(not lines, lines)


def tidy(files, includes, root):
    exe = c.which("clang-tidy")
    if not exe and platform.system() == "Darwin":
        brew = Path("/opt/homebrew/opt/llvm/bin/clang-tidy")
        exe = str(brew) if brew.is_file() else None
    if not exe:
        return _chk(False, ["clang-tidy no instalado (brew install llvm)"])
    args = ["--", "-std=c11"] + sum([["-I", str(i)] for i in includes], [])
    unity = root / "third_party" / "unity" / "src"
    if unity.is_dir():
        args += ["-I", str(unity)]
    found = []
    for f in files:
        cp = c.run([exe, "--quiet", str(f)] + args, cwd=root, timeout=300)
        for ln in (cp.stdout + cp.stderr).splitlines():
            if re.search(r":\d+:\d+: (warning|error):", ln):
                found.append(ln.strip())
    return _chk(not found, found)


def fmt_check(files, root):
    exe = c.which("clang-format")
    if not exe:
        return _chk(False, ["clang-format no instalado"])
    bad = []
    for f in files:
        cp = c.run([exe, "--dry-run", "--Werror", str(f)], cwd=root)
        if cp.returncode != 0:
            n = len([ln for ln in cp.stderr.splitlines() if "warning:" in ln or "error:" in ln])
            bad.append("%s: %d diferencias de formato (clang-format -i)" % (f, n))
    return _chk(not bad, bad)


def complexity(files, root):
    if not c.which("lizard"):
        return _chk(False, ["lizard no instalado (pip3 install --user lizard)"])
    cfg = c.load_config(root)["impl"]
    cp = c.run(["lizard", "-l", "c", "-C", str(cfg["cc_max"]), "-L", str(cfg["lines_max"]),
                "-a", str(cfg["params_max"]), "-w"] + [str(f) for f in files], cwd=root)
    found = [ln.strip() for ln in cp.stdout.splitlines() if "warning:" in ln]
    return _chk(not found, found)


def doxygen_check(src, header, func):
    issues = []
    text = Path(src).read_text(encoding="utf-8")
    head = text[:2000]
    for tag in ("@file", "@brief"):
        if tag not in head:
            issues.append("%s: falta %s en la cabecera Doxygen del fichero (C-DOC-01)" % (src, tag))
    if header and Path(header).is_file() and func:
        htxt = Path(header).read_text(encoding="utf-8")
        m = re.search(r"/\*\*((?:(?!\*/).)*?)\*/\s*(?:[\w\s\*]+?)\b%s\s*\(([^)]*)\)" % re.escape(func),
                      htxt, flags=re.S)
        if not m:
            issues.append("%s: %s sin bloque Doxygen inmediatamente antes del prototipo (C-DOC-02)"
                          % (header, func))
        else:
            block, params = m.group(1), m.group(2)
            for tag in DOX_TAGS_FUNC:
                if tag not in block:
                    issues.append("%s: %s sin %s (C-DOC-02)" % (header, func, tag))
            for p in [x.strip() for x in params.split(",") if x.strip() and x.strip() != "void"]:
                name = re.findall(r"(\w+)\s*(?:\[[^\]]*\])?$", p)
                if name and not re.search(r"@param(\[[a-z, ]+\])?\s+%s\b" % name[0], block):
                    issues.append("%s: %s sin @param para '%s' (C-DOC-02)" % (header, func, name[0]))
    return issues


def forbidden_check(files):
    issues = []
    for f in files:
        raw = Path(f).read_text(encoding="utf-8")
        for n, ln in enumerate(raw.splitlines(), 1):
            if re.search(r"\b(TODO|FIXME|XXX)\b", ln):
                issues.append("%s:%d: marcador prohibido (C-DOC-05)" % (f, n))
        code = _strip_comments(raw)
        for n, ln in enumerate(code.splitlines(), 1):
            for sym in FORBIDDEN:
                if re.search(r"\b%s\s*\(" % sym, ln):
                    issues.append("%s:%d: uso de '%s' prohibido (C-LIB-01/02)" % (f, n, sym))
            if re.search(r"^\s*#\s*pragma\s+once", ln):
                issues.append("%s:%d: #pragma once prohibido (C-PRE-01)" % (f, n))
            if re.search(r"\b(?<!u)(?<!_)int\s+\w+\s*[=;,)]", ln) and "int32_t" not in ln:
                issues.append("%s:%d: tipo 'int' sin ancho fijo (C-TYP-01)" % (f, n))
    return issues


def static_checks(src, includes, root, header=None, func=None):
    """Checks de estándares sobre un fichero fuente (usado también por el implementador)."""
    files = [Path(src)]
    res = {
        "misra": misra(files, includes, root),
        "tidy": tidy(files, includes, root),
        "format": fmt_check(files + ([Path(header)] if header else []), root),
        "complexity": complexity(files, root),
    }
    dox = doxygen_check(src, header, func)
    res["doxygen"] = _chk(not dox, dox)
    forb = forbidden_check(files)
    res["forbidden"] = _chk(not forb, forb)
    return res


# --------------------------------------------------------------------------- informes
def _rel(root, s):
    return str(s).replace(str(root) + "/", "")


def _next_run(rdir):
    runs = [int(p.stem.split("-")[1]) for p in rdir.glob("run-*.json")
            if p.stem.split("-")[1].isdigit()]
    return (max(runs) + 1) if runs else 1


def _git(root, *args):
    return c.run(["git"] + list(args), cwd=root).stdout.strip()


LOG_PATHS = (str(LOG_REL), str(impl.PLAN_REL))


def _dirty(root):
    """Cambios sin commitear, ignorando el log de tests y el plan (los commitea h test)."""
    out = c.run(["git", "status", "--porcelain", "--untracked-files=no"], cwd=root).stdout
    return [ln for ln in out.splitlines() if ln.strip() and ln[3:].strip() not in LOG_PATHS]


def _commit_log(root, rep, label):
    """Commitea log-tests.md y plan.json tras una ejecución (TST-LOG-01)."""
    branch = rep["branch"]
    cfg = c.load_config(root)["git"]
    if branch in (cfg["main"], cfg["develop"]):
        c.log("WARN", "en %s no se commitea el log (GIT-BR-03); usa una rama de trabajo" % branch)
        return
    c.run(["git", "add", "--"] + [p for p in LOG_PATHS if (root / p).exists()], cwd=root)
    if not _git(root, "diff", "--cached", "--name-only"):
        return
    msg = "docs(impl): log test run %d for %s (%s)" % (rep["run"], label, rep["result"])
    cp = c.run(["git", "commit", "-m", msg, "-m", "Refs: %s" % label], cwd=root)
    if cp.returncode == 0:
        rep["commit_tested"] = rep["commit"]
        rep["commit"] = _git(root, "rev-parse", "HEAD")


SOURCE_CHECKS = ("misra", "tidy", "format", "complexity", "doxygen", "forbidden")


DIAG_START = re.compile(r"^(In file included from |\s+from )?([\w./-]+\.[ch]):\d+")


def filter_log(log, allowed):
    """Conserva solo los diagnósticos de ficheros permitidos (evita filtrar código ajeno)."""
    out, keep = [], False
    for line in log.splitlines():
        m = DIAG_START.match(line)
        if m:
            keep = m.group(2).startswith(allowed)
        elif not line.startswith((" ", "\t")) or "|" not in line:
            keep = keep and line.startswith((" ", "\t"))
        if keep:
            out.append(line)
    return "\n".join(out) or "(sin diagnósticos en tus ficheros; el fallo está en otra parte: " \
                             "el orquestador lo gestiona)"


def feedback_impl(rep, f, root):
    """Feedback para el implementador: sin código ni nombres de fichero de test (TST-IND-04)."""
    L = ["# Feedback %s · ejecución %d" % (f["id"], rep["run"]), "",
         "Resultado: **%s**" % rep["result"], ""]
    u = rep["checks"].get("unit", {})
    if u.get("failures"):
        L += ["## Casos fallados", "", "| Caso | Requisito | Esperado vs obtenido |", "|---|---|---|"]
        for fl in u["failures"]:
            detail = re.sub(r"\[TC-[^\]]+\]|\[[A-Z]+-[A-Z0-9-]+\]", "", fl["detail"]).strip()
            L.append("| %s | %s | %s |" % (fl["tc"], ", ".join(fl["reqs"]) or "-",
                                           detail.replace("|", "\\|")))
        L.append("")
    if u.get("sanitizer"):
        L += ["## Sanitizers (ASan/UBSan)", ""]
        L += ["- %s" % s for s in u["sanitizer"] if "tests/" not in s][:20]
        L.append("")
    if u.get("crashed"):
        L += ["## El ejecutable de test terminó de forma anómala (rc=%s) sin fallos de aserción" %
              u.get("rc"), ""]
    cov = rep["checks"].get("coverage", {})
    if cov and not cov.get("ok"):
        L += ["## Cobertura del fichero de la función", ""] + ["- %s" % d for d in cov["details"]]
        L += ["- %s" % x for x in cov.get("uncovered", [])[:40]]
        L += ["", "Si una línea/rama es inalcanzable por diseño (código defensivo), justifícalo al "
              "orquestador o elimínala si no la exige el contrato.", ""]
    for name in SOURCE_CHECKS:
        chk = rep["checks"].get(name)
        if chk and not chk["ok"]:
            L += ["## Estándares · %s" % name, ""] + ["- %s" % d for d in chk["details"][:40]] + [""]
    b = rep["checks"].get("build", {})
    if b and not b.get("ok") and "impl" in b.get("sides", []):
        L += ["## Compilación", "", "```", filter_log(b.get("log", ""), ("src/", "include/"))[-2500:],
              "```"]
    return "\n".join(L) + "\n"


def feedback_tests(rep, f):
    """Feedback para el ingeniero de tests: sin código de la implementación."""
    L = ["# Feedback de tests %s · ejecución %d" % (f["id"], rep["run"]), ""]
    b = rep["checks"].get("build", {})
    if b and not b.get("ok") and "test" in b.get("sides", []):
        L += ["## El fichero de test no compila", "", "```",
              filter_log(b.get("log", ""), ("tests/",))[-2500:], "```", ""]
    tf = rep["checks"].get("test_format", {})
    if tf and not tf["ok"]:
        L += ["## Formato/estándares del fichero de test", ""] + ["- %s" % d for d in tf["details"]]
    cov = rep["checks"].get("coverage", {})
    if cov and not cov.get("ok"):
        L += ["## Cobertura insuficiente", "",
              "La cobertura de la función no alcanza el mínimo. El orquestador indicará qué "
              "cláusulas del contrato (pre/post/err) parecen no ejercitadas.", ""]
    return "\n".join(L) + "\n"


def append_log(root, rep, f=None):
    path = root / LOG_REL
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.is_file():
        path.write_text("# Log de tests y cambios\n\nRegistro append-only (TST-LOG-01/02). "
                        "Generado por `h test` y `h log change`.\n", encoding="utf-8")
    name = f["id"] + " · " + f["nombre"] if f else "Integración completa"
    L = ["", "## %s · %s · ejecución %d · %s" % (rep["timestamp"][:16].replace("T", " "), name,
                                                 rep["run"], rep["result"]), "",
         "- Rama: `%s` · commit `%s`" % (rep["branch"], rep["commit"][:10]), "",
         "| Check | Resultado | Detalle |", "|---|---|---|"]
    for k, v in rep["checks"].items():
        det = ""
        if k == "unit":
            det = "%d casos, %d fallos" % (len(v.get("tests", [])), len(v.get("failures", [])))
        elif k == "coverage" and v.get("summary"):
            det = "; ".join("%s L %.1f %% R %s %%" % (Path(fn).name, float(s["line"] or 0),
                                                      s["branch"]) for fn, s in v["summary"].items())
        elif not v["ok"]:
            det = "%d hallazgos" % len(v.get("details", []))
        L.append("| %s | %s | %s |" % (k, "PASS" if v["ok"] else "FAIL", det))
    fails = rep["checks"].get("unit", {}).get("failures", [])
    if fails:
        L += ["", "Casos fallados:", ""]
        for fl in fails:
            L.append("- `%s` %s — %s" % (fl["tc"], ", ".join(fl["reqs"]),
                                         re.sub(r"\[[^\]]+\]", "", fl["detail"]).strip()))
    with path.open("a", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")


def log_change(fid, agent, cause, desc, root=None):
    root = root or c.project_root()
    path = root / LOG_REL
    if not path.is_file():
        c.die("no existe %s" % LOG_REL)
    commit = _git(root, "rev-parse", "--short=10", "HEAD")
    entry = ("\n### Cambio · %s · %s\n\n- Agente: %s\n- Causa raíz: %s\n- Cambio aplicado: %s\n"
             "- Commit: `%s`\n" % (c.now_iso().replace("T", " ")[:16], fid, agent, cause, desc, commit))
    with path.open("a", encoding="utf-8") as fh:
        fh.write(entry)
    c.info("cambio registrado en %s" % LOG_REL)
    return c.EXIT_OK


# --------------------------------------------------------------------------- ejecución
def run_function(fid, root=None):
    root = root or c.project_root()
    cfg = c.load_config(root)
    plan = impl.load_plan(root)
    f = impl.get(fid, plan=plan)
    mod = plan.get("modulo") or cfg["module"]
    rdir = root / impl.REPORTS_REL / fid
    rdir.mkdir(parents=True, exist_ok=True)
    rep = {"fid": fid, "run": _next_run(rdir), "timestamp": c.now_iso(),
           "branch": _git(root, "rev-parse", "--abbrev-ref", "HEAD"),
           "commit": _git(root, "rev-parse", "HEAD"), "checks": {}}
    if _dirty(root):
        c.log("WARN", "hay cambios sin commitear: el informe no habilitará el merge (GIT-MG-02)")
        rep["dirty"] = True
    src, test, header = root / f["fuente"], root / f["test"], root / f["cabecera"]
    for p, side in ((src, "impl"), (test, "test"), (header, "orquestador")):
        if not p.is_file():
            rep["checks"]["build"] = _chk(False, ["falta %s" % p.relative_to(root)],
                                          {"sides": [side], "log": ""})
            return _finish(root, rep, f, rdir)
    ok, bdir, blog = build(root, cfg["impl"]["cmake_preset"])
    blog = _rel(root, blog)
    sides = []
    if not ok:
        if re.search(r"^tests/[^:\n]+:\d+(:\d+)?: (fatal )?error", blog, flags=re.M):
            sides.append("test")
        if re.search(r"^(src|include)/[^:\n]+:\d+(:\d+)?: (fatal )?error", blog, flags=re.M) \
                or not sides:
            sides.append("impl")
    rep["checks"]["build"] = _chk(ok, [] if ok else ["la compilación falló (%s)" % ", ".join(sides)],
                                  {"sides": sides, "log": blog if not ok else ""})
    if not ok:
        return _finish(root, rep, f, rdir)
    exe_name = test.stem
    exe = next((p for p in bdir.rglob(exe_name) if p.is_file() and os.access(str(p), os.X_OK)), None)
    if exe is None:
        rep["checks"]["unit"] = _chk(False, ["no se encontró el ejecutable %s" % exe_name])
        return _finish(root, rep, f, rdir)
    _clear_gcda(bdir)
    ur = run_binary(exe, root)
    unit_ok = not ur["failures"] and not ur["sanitizer"] and ur["rc"] == 0 and ur["tests"]
    rep["checks"]["unit"] = _chk(unit_ok, [fl["tc"] for fl in ur["failures"]], {
        "tests": ur["tests"], "failures": ur["failures"], "sanitizer": ur["sanitizer"],
        "rc": ur["rc"], "crashed": ur["crashed"]})
    rep["checks"]["coverage"] = coverage(root, bdir, [f["fuente"]])
    inc = [root / "include", root / "src" / mod, root / "src"]
    rep["checks"].update(static_checks(src, inc, root, header=header, func=f["nombre"]))
    tf = fmt_check([test], root)
    rep["checks"]["test_format"] = tf
    return _finish(root, rep, f, rdir)


def _finish(root, rep, f, rdir):
    rep["result"] = "PASS" if all(v["ok"] for v in rep["checks"].values()) and \
        not rep.get("dirty") else "FAIL"
    if f is not None:
        sb = root / impl.SANDBOX_REL
        if (sb / "impl").is_dir():
            (sb / "impl" / "feedback" / ("feedback-%d.md" % rep["run"])).write_text(
                feedback_impl(rep, f, root), encoding="utf-8")
            (sb / "test" / "feedback" / ("feedback-%d.md" % rep["run"])).write_text(
                feedback_tests(rep, f), encoding="utf-8")
        plan = impl.load_plan(root)
        pf = impl.get(f["id"], plan=plan)
        pf["iteraciones"] = int(pf.get("iteraciones", 0)) + (0 if rep["result"] == "PASS" else 1)
        if pf.get("estado") in ("pendiente", "contrato"):
            pf["estado"] = "en-curso"
        impl.save_plan(plan, root)
        maxit = c.load_config(root)["impl"]["max_iterations"]
        if pf["iteraciones"] >= maxit and rep["result"] != "PASS":
            c.log("CRIT", "TST-LOG-04: límite de iteraciones alcanzado; consultar al usuario",
                  fid=f["id"], iteraciones=pf["iteraciones"])
    append_log(root, rep, f)
    if not rep.get("dirty"):
        _commit_log(root, rep, f["id"] if f else "integration")
    c.save_json(rdir / ("run-%d.json" % rep["run"]), rep)
    c.save_json(rdir / "latest.json", rep)
    print(_summary(rep))
    return c.EXIT_OK if rep["result"] == "PASS" else c.EXIT_FAIL


def _summary(rep):
    L = ["%s · ejecución %d · %s" % (rep.get("fid", "INTEGRACIÓN"), rep["run"], rep["result"])]
    for k, v in rep["checks"].items():
        L.append("  [%s] %s" % ("x" if v["ok"] else " ", k))
        if not v["ok"]:
            for d in (v.get("details") or [])[:8]:
                L.append("        - %s" % d)
    return "\n".join(L)


def run_all(root=None):
    root = root or c.project_root()
    cfg = c.load_config(root)
    plan = impl.load_plan(root)
    mod = plan.get("modulo") or cfg["module"]
    rdir = root / impl.REPORTS_REL / "integration"
    rdir.mkdir(parents=True, exist_ok=True)
    rep = {"fid": "INTEGRACION", "run": _next_run(rdir), "timestamp": c.now_iso(),
           "branch": _git(root, "rev-parse", "--abbrev-ref", "HEAD"),
           "commit": _git(root, "rev-parse", "HEAD"), "checks": {}}
    if _dirty(root):
        rep["dirty"] = True
    ok, bdir, blog = build(root, cfg["impl"]["cmake_preset"])
    rep["checks"]["build"] = _chk(ok, [] if ok else ["la compilación falló"], {"log": blog[-3000:]})
    if ok:
        _clear_gcda(bdir)
        tests, fails, san, all_ok = [], [], [], True
        exes = sorted(p for p in bdir.rglob("test_*") if p.is_file() and os.access(str(p), os.X_OK)
                      and p.suffix == "")
        for exe in exes:
            r = run_binary(exe, root)
            tests += r["tests"]
            fails += r["failures"]
            san += r["sanitizer"]
            all_ok = all_ok and r["rc"] == 0
        rep["checks"]["unit+integration"] = _chk(all_ok and not fails and not san and bool(tests),
                                                 [x["tc"] for x in fails],
                                                 {"tests": tests, "failures": fails,
                                                  "sanitizer": san, "executables": len(exes)})
        srcs = sorted((root / "src" / mod).glob("*.c"))
        rep["checks"]["coverage"] = coverage(root, bdir, [str(s.relative_to(root)) for s in srcs])
        inc = [root / "include", root / "src" / mod, root / "src"]
        all_src = sorted((root / "src").rglob("*.c"))
        rep["checks"]["misra"] = misra(all_src, inc, root)
        rep["checks"]["tidy"] = tidy(all_src, inc, root)
        hdrs = sorted((root / "include").rglob("*.h")) + sorted((root / "src").rglob("*.h"))
        tests_c = sorted((root / "tests").rglob("*.c"))
        rep["checks"]["format"] = fmt_check(all_src + hdrs + tests_c, root)
        rep["checks"]["complexity"] = complexity(all_src, root)
        dox = []
        for fn in plan.get("funciones", []):
            dox += doxygen_check(root / fn["fuente"], root / fn["cabecera"], fn["nombre"])
        rep["checks"]["doxygen"] = _chk(not dox, dox)
        forb = forbidden_check(all_src)
        rep["checks"]["forbidden"] = _chk(not forb, forb)
        if c.which("aarch64-linux-gnu-gcc") and _cmake_preset_available(root, "cross-aarch64"):
            okx, _, logx = build(root, "cross-aarch64")
            rep["checks"]["cross-aarch64"] = _chk(okx, [] if okx else [logx[-1500:]])
    rc = _finish(root, rep, None, rdir)
    c.save_json(root / impl.REPORTS_REL / "integration-latest.json",
                {"result": rep["result"], "commit": rep["commit"], "at": rep["timestamp"],
                 "run": rep["run"], "branch": rep["branch"]})
    return rc
