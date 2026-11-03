#!/usr/bin/env python3
"""Autotests del harness (unittest, solo biblioteca estándar + dependencias del harness).

Rápidos por defecto; `H_E2E=1` ejecuta además el recorrido completo de las 4 fases
con un proyecto C real (requiere cmake, cppcheck, clang-tidy, clang-format, gcovr, lizard,
pandoc, mmdc y acceso a GitHub para Unity).
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HARNESS = Path(__file__).resolve().parent.parent
H = str(HARNESS / "bin" / "h")
sys.path.insert(0, str(HARNESS / "scripts"))

from hlib import matrix, reqs  # noqa: E402


def run(cmd, cwd=None, inp=None, env=None):
    e = dict(os.environ)
    e.update(env or {})
    return subprocess.run(cmd, cwd=cwd, input=inp, capture_output=True, text=True, env=e)


def git_project(tmp):
    run(["git", "init", "-q", "-b", "main"], cwd=tmp)
    run(["git", "config", "user.email", "t@example.com"], cwd=tmp)
    run(["git", "config", "user.name", "Test"], cwd=tmp)
    Path(tmp, "README.md").write_text("# test\n")
    run(["git", "add", "."], cwd=tmp)
    run(["git", "commit", "-qm", "chore: init", "--no-verify"], cwd=tmp)
    r = run([H, "init", "--name", "Prueba", "--code", "PRB", "--author", "Test"], cwd=tmp)
    assert r.returncode == 0, r.stderr


REQ_OK = """# Requisitos del sistema

| Campo | Valor |
|---|---|
| Código | PRB-REQ |
| Versión | 1.0 |

Convenciones de la plantilla.

## Entradas

### SYS-IN-01 — Ciclo

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá procesar una nube cada 50 ms. |
| Tipo | Funcional |
| Origen | R1 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Latencia ≤ 50 ms |
| Estado | Aprobado |

## Seguridad y modos de funcionamiento

### SYS-SAF-01 — Pérdida de nubes

| Campo | Valor |
|---|---|
| Descripción | Si no llega ninguna nube durante 3 ciclos, entonces el sistema deberá marcar el grid como desconocido. |
| Tipo | Funcional |
| Origen | Derivado |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Transición en ≤ 150 ms |
| Estado | Aprobado |

## Hipótesis

### H-01 — Vía recta

| Campo | Valor |
|---|---|
| Descripción | La vía es recta en el campo de visión del radar. |
| Tipo | Hipótesis |
| Origen | Derivado |
| Estado | Aprobado |
"""


class TestRequirements(unittest.TestCase):
    def test_parse_and_ignore_comments(self):
        text = "<!--\n### SYS-IN-99 — Ejemplo\n-->\n" + REQ_OK
        rs = reqs.parse(text)
        self.assertEqual([r["id"] for r in rs], ["SYS-IN-01", "SYS-SAF-01", "H-01"])
        self.assertEqual(rs[0]["campos"]["verificacion"], "Test")

    def test_lint_detects_vague_and_missing(self):
        with tempfile.TemporaryDirectory() as tmp:
            d = Path(tmp, "docs", "01-requisitos")
            d.mkdir(parents=True)
            Path(tmp, "harness.config.json").write_text("{}")
            (d / "requisitos.md").write_text(REQ_OK)
            errs, _ = reqs.lint(Path(tmp), strict=True)
            self.assertEqual(errs, [])
            bad = REQ_OK.replace("procesar una nube cada 50 ms", "ser rápido y adecuado") \
                        .replace("| Prioridad | Must |\n", "", 1) \
                        .replace("| Origen | R1 |", "| Origen | Enunciado |") \
                        .replace("### H-01", "### SYS-T-01") \
                        .replace("≤ 150 ms |", "≤ [PENDIENTE: 150 ms] |")
            (d / "requisitos.md").write_text(bad)
            errs, warns = reqs.lint(Path(tmp), strict=True)
            joined = "\n".join(errs)
            self.assertIn("Prioridad", joined)
            self.assertIn("Source", joined)
            self.assertIn("SYS-T-01", joined)          # sección Hipótesis exige prefijo H-
            self.assertIn("pendiente", joined)
            self.assertTrue(any("vago" in w for w in warns))


class TestMatrix(unittest.TestCase):
    DATA = {
        "escala": {"min": 1, "max": 5},
        "criterios": [{"id": "C1", "nombre": "A", "peso": 60}, {"id": "C2", "nombre": "B", "peso": 40}],
        "alternativas": [{"id": "BASE", "nombre": "b", "seleccionada": True},
                         {"id": "ALT-01", "nombre": "a"}],
        "puntuaciones": {"BASE": {"C1": 5, "C2": 1}, "ALT-01": {"C1": 2, "C2": 5}},
    }

    def test_totals(self):
        t = matrix.totals(self.DATA)
        self.assertAlmostEqual(t["BASE"], 3.4)
        self.assertAlmostEqual(t["ALT-01"], 3.2)

    def test_validation(self):
        bad = json.loads(json.dumps(self.DATA))
        bad["criterios"][0]["peso"] = 50
        bad["puntuaciones"]["BASE"]["C1"] = 9
        errs = matrix.validate(bad)
        self.assertTrue(any("suman" in e for e in errs))
        self.assertTrue(any("fuera de escala" in e for e in errs))

    def test_sensitivity_detects_change(self):
        rows = matrix.sensitivity(self.DATA, 30)
        changes = [k["id"] for k, lo, hi in rows if lo != "BASE" or hi != "BASE"]
        self.assertTrue(changes)


class TestGuards(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        Path(self.tmp, "harness.config.json").write_text("{}")
        for r in ("impl", "test"):
            Path(self.tmp, ".harness/sandbox/current", r, "out").mkdir(parents=True)

    def tearDown(self):
        shutil.rmtree(self.tmp)

    def guard(self, role, tool, ti):
        data = json.dumps({"tool_name": tool, "tool_input": ti, "cwd": self.tmp})
        return run([sys.executable, str(HARNESS / "hooks" / "guard_paths.py"), "--role", role],
                   inp=data, env={"CLAUDE_PROJECT_DIR": self.tmp}).returncode

    def test_impl_cannot_read_tests(self):
        self.assertEqual(self.guard("impl", "Read", {"file_path": self.tmp + "/tests/.h-canary"}), 2)
        self.assertEqual(self.guard("impl", "Read",
                                    {"file_path": self.tmp + "/.harness/sandbox/current/test/x"}), 2)

    def test_test_cannot_read_src(self):
        self.assertEqual(self.guard("test", "Read", {"file_path": self.tmp + "/src/.h-canary"}), 2)
        self.assertEqual(self.guard("test", "Grep", {"pattern": "x"}), 2)

    def test_sandbox_allowed_and_write_only_out(self):
        sb = self.tmp + "/.harness/sandbox/current/impl"
        self.assertEqual(self.guard("impl", "Read", {"file_path": sb + "/contrato/F.md"}), 0)
        self.assertEqual(self.guard("impl", "Write", {"file_path": sb + "/out/src/a.c"}), 0)
        self.assertEqual(self.guard("impl", "Write", {"file_path": sb + "/contrato/F.md"}), 2)

    def test_bash_allowlist(self):
        ok = H + " sandbox syntax impl out/src/a.c"
        self.assertEqual(self.guard("impl", "Bash", {"command": ok}), 0)
        self.assertEqual(self.guard("impl", "Bash", {"command": "cat ../../../../tests/x.c"}), 2)
        self.assertEqual(self.guard("impl", "Bash", {"command": ok + "; cat /etc/passwd"}), 2)
        self.assertEqual(self.guard("test", "Bash", {"command": "git log -p"}), 2)

    def test_non_isolated_agent_passes(self):
        data = json.dumps({"tool_name": "Read", "tool_input": {"file_path": "/x"}, "cwd": self.tmp})
        r = run([sys.executable, str(HARNESS / "hooks" / "guard_paths.py")], inp=data)
        self.assertEqual(r.returncode, 0)

    def test_phase_guard(self):
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            def pg(path):
                data = json.dumps({"tool_name": "Write", "tool_input": {"file_path": path},
                                   "cwd": tmp})
                return run([sys.executable, str(HARNESS / "hooks" / "phase_guard.py")], inp=data,
                           env={"CLAUDE_PROJECT_DIR": tmp}).returncode
            self.assertEqual(pg(tmp + "/docs/01-requisitos/requisitos.md"), 0)
            self.assertEqual(pg(tmp + "/docs/02-diseno/diseno.md"), 2)
            self.assertEqual(pg(tmp + "/src/ogrid/x.c"), 2)
            self.assertEqual(pg(tmp + "/README.md"), 0)


class TestGitHooks(unittest.TestCase):
    def test_commit_msg(self):
        hook = str(HARNESS / "git-hooks" / "commit-msg")
        with tempfile.NamedTemporaryFile("w", delete=False) as fh:
            fh.write("feat(ogrid): add cell update\n")
        self.assertEqual(run([hook, fh.name]).returncode, 0)
        for bad in ("added stuff", "feat: Ends with dot.", "feature(x): nope"):
            Path(fh.name).write_text(bad + "\n")
            self.assertNotEqual(run([hook, fh.name]).returncode, 0, bad)
        Path(fh.name).write_text("Merge branch 'x' into develop\n")
        self.assertEqual(run([hook, fh.name]).returncode, 0)
        os.unlink(fh.name)

    def test_branch_names(self):
        for ok in ("feature/SYS-PR-01-ogrid-init", "docs/requisitos", "harness/doc-export",
                   "test/integration", "ui/grid-viewer", "release/1.0.0"):
            self.assertEqual(run([H, "git", "check-branch", ok]).returncode, 0, ok)
        for bad in ("feature/x", "Docs/requisitos", "random", "docs/otra"):
            self.assertNotEqual(run([H, "git", "check-branch", bad]).returncode, 0, bad)

    def test_protected_branches(self):
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            run(["git", "checkout", "-q", "develop"], cwd=tmp)
            Path(tmp, "x.txt").write_text("x")
            run(["git", "add", "x.txt"], cwd=tmp)
            r = run(["git", "commit", "-m", "chore: direct commit"], cwd=tmp)
            self.assertNotEqual(r.returncode, 0)
            self.assertIn("GIT-BR-03", r.stderr)


class TestPhases(unittest.TestCase):
    def test_close_requirements_and_tags(self):
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            self.assertEqual(run([H, "gate", "require", "design"], cwd=tmp).returncode, 1)
            run([H, "git", "start-phase", "req"], cwd=tmp)
            Path(tmp, "docs/01-requisitos/requisitos.md").write_text(REQ_OK)
            run(["git", "add", "-A"], cwd=tmp)
            self.assertEqual(run(["git", "commit", "-qm", "docs(req): add requirements"],
                                 cwd=tmp).returncode, 0)
            r = run([H, "gate", "close", "req"], cwd=tmp)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            self.assertIn("req-v1.0", run(["git", "tag"], cwd=tmp).stdout)
            self.assertEqual(run([H, "gate", "require", "design"], cwd=tmp).returncode, 0)
            st = json.loads(Path(tmp, ".harness/state.json").read_text())
            self.assertEqual(st["phases"]["req"]["status"], "closed")
            r = run([H, "gate", "reopen", "req", "nuevo requisito del cliente"], cwd=tmp)
            self.assertEqual(r.returncode, 0, r.stderr)
            self.assertEqual(run([H, "gate", "require", "design"], cwd=tmp).returncode, 1)


@unittest.skipUnless(shutil.which("pandoc"), "pandoc no disponible")
class TestExport(unittest.TestCase):
    def test_requirements_in_template_format(self):
        from docx import Document
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            Path(tmp, "docs/01-requisitos/requisitos.md").write_text(
                REQ_OK.replace("cada 50 ms. |", "cada [PENDIENTE: 50 ms]. |"))
            r = run([H, "export", "requisitos"], cwd=tmp)
            self.assertEqual(r.returncode, 0, r.stderr)
            doc = Document(str(Path(tmp, "docs/export/requisitos.docx")))
            self.assertEqual(len(doc.tables), 1)            # sin tabla de control
            t = doc.tables[0]
            self.assertEqual([c.text for c in t.rows[0].cells][:4],
                             ["Code", "Description", "Type", "Source"])
            col0 = [r.cells[0].text for r in t.rows]
            self.assertEqual(col0, ["Code", "Entradas", "SYS-IN-01",
                                    "Seguridad y modos de funcionamiento", "SYS-SAF-01",
                                    "Hipótesis", "H-01"])
            self.assertIn("[50 ms]", t.rows[2].cells[1].text)
            self.assertTrue(doc.sections[0].page_width > doc.sections[0].page_height)
            texts = "\n".join(p.text for p in doc.paragraphs)
            self.assertNotIn("{{", texts)
            self.assertNotIn("Índice", texts)

    def test_generic_document_has_cover_and_grey_tables(self):
        from docx import Document
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            Path(tmp, "nota.md").write_text("# Nota\n\n## Uno\n\nTabla: Ejemplo\n\n"
                                            "| A | B |\n|---|---|\n| 1 | 2 |\n")
            r = run([H, "export", "nota.md"], cwd=tmp)
            self.assertEqual(r.returncode, 0, r.stderr)
            doc = Document(str(Path(tmp, "docs/export/nota.docx")))
            styles = [p.style.name for p in doc.paragraphs]
            self.assertIn("Title", styles)
            texts = "\n".join(p.text for p in doc.paragraphs)
            self.assertIn("Tabla 1. Ejemplo", texts)
            shd = doc.tables[-1].rows[0].cells[0]._tc.xml
            self.assertIn("D9D9D9", shd.upper())

    def test_table_of_contents_has_cached_entries(self):
        """El campo TOC lleva las entradas dentro, para visores que no lo actualizan (Pages)."""
        import re
        import zipfile
        with tempfile.TemporaryDirectory() as tmp:
            git_project(tmp)
            Path(tmp, "nota.md").write_text("# Nota\n\n## Uno\n\ntexto\n\n### Uno punto uno\n\n"
                                            "texto\n\n## Dos\n\ntexto\n")
            r = run([H, "export", "nota.md"], cwd=tmp)
            self.assertEqual(r.returncode, 0, r.stderr)
            xml = zipfile.ZipFile(str(Path(tmp, "docs/export/nota.docx"))).read(
                "word/document.xml").decode()
            start = xml.find("Table of Contents")
            seg = xml[start:xml.find("</w:sdt>", start)]
            texts = re.findall(r"<w:t[^>]*>([^<]*)</w:t>", seg)
            for title in ("Uno", "Uno punto uno", "Dos"):
                self.assertIn(title, texts)
            self.assertEqual(seg.count('fldCharType="begin"'), seg.count('fldCharType="end"'))


class TestInstaller(unittest.TestCase):
    def test_install_update_uninstall(self):
        with tempfile.TemporaryDirectory() as tmp:
            inst = [str(HARNESS / "install.sh"), "--target", tmp]
            self.assertEqual(run(inst).returncode, 0)
            cdir = Path(tmp, ".claude")
            self.assertTrue((cdir / "commands" / "impl-funcion.md").is_file())
            body = (cdir / "agents" / "implementador.md").read_text()
            self.assertNotIn("@@HARNESS@@", body)
            self.assertIn(str(cdir / "harness"), body)
            settings = json.loads((cdir / "settings.json").read_text())
            self.assertEqual(len(settings["hooks"]["PreToolUse"]), 2)
            self.assertEqual(run(inst + ["--update"]).returncode, 0)
            settings = json.loads((cdir / "settings.json").read_text())
            self.assertEqual(len(settings["hooks"]["PreToolUse"]), 2)
            self.assertEqual(run(inst + ["--uninstall"]).returncode, 0)
            self.assertFalse((cdir / "commands").exists())


class TestShell(unittest.TestCase):
    @unittest.skipUnless(shutil.which("shellcheck"), "shellcheck no disponible")
    def test_shellcheck(self):
        files = [str(HARNESS / "bin" / "h"), str(HARNESS / "install.sh"),
                 str(HARNESS / "tests" / "run.sh")]
        files += [str(p) for p in (HARNESS / "git-hooks").iterdir()]
        files += [str(p) for p in HARNESS.rglob("*.sh") if "templates" in p.parts]
        r = run(["shellcheck", "-S", "style"] + files)
        self.assertEqual(r.returncode, 0, r.stdout)


@unittest.skipUnless(os.environ.get("H_E2E") == "1", "H_E2E=1 para el recorrido completo")
class TestEndToEnd(unittest.TestCase):
    def test_e2e(self):
        script = HARNESS / "tests" / "e2e.sh"
        r = run(["bash", str(script)])
        self.assertEqual(r.returncode, 0, r.stdout[-4000:] + r.stderr[-4000:])


if __name__ == "__main__":
    unittest.main(verbosity=2)
