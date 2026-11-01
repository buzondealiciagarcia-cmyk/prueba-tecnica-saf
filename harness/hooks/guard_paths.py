#!/usr/bin/env python3
"""Hook PreToolUse de aislamiento (TST-IND-03).

Restringe a los agentes `implementador` e `ingeniero-tests` a su sandbox:
  - solo leen/buscan dentro de .harness/sandbox/current/<rol>/
  - solo escriben dentro de .harness/sandbox/current/<rol>/out/
  - en Bash solo pueden ejecutar `h sandbox syntax|check-impl` y `clang-format -i` en out/
Uso: guard_paths.py --role impl|test   (desde el frontmatter del agente)
     guard_paths.py                     (global: deduce el rol de agent_type si existe)
Bloquea con código 2 y explica el motivo en stderr (el agente lo recibe).
"""
import datetime
import json
import os
import re
import shlex
import sys
from pathlib import Path

ROLE_BY_AGENT = {"implementador": "impl", "ingeniero-tests": "test"}
ALLOWED_TOOLS = {"Read", "Write", "Edit", "MultiEdit", "Glob", "Grep", "Bash", "TodoWrite"}
WRITE_TOOLS = {"Write", "Edit", "MultiEdit"}


def find_root(start):
    env = os.environ.get("CLAUDE_PROJECT_DIR")
    if env:
        return Path(env).resolve()
    p = Path(start).resolve()
    for q in [p] + list(p.parents):
        if (q / "harness.config.json").is_file():
            return q
    return p


def inside(path, base):
    try:
        Path(path).resolve().relative_to(base.resolve())
        return True
    except ValueError:
        return False


def audit(root, role, tool, target, verdict):
    try:
        d = root / ".harness" / "audit"
        d.mkdir(parents=True, exist_ok=True)
        with (d / ("%s.log" % role)).open("a", encoding="utf-8") as fh:
            fh.write("%s %s tool=%s target=%s verdict=%s\n" % (
                datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"), role, tool,
                str(target).replace(" ", "_")[:200], verdict))
    except OSError:
        pass


def deny(root, role, tool, target, why):
    audit(root, role, tool, target, "DENY")
    sys.stderr.write("BLOQUEADO por aislamiento (%s): %s\nSandbox permitido: "
                     ".harness/sandbox/current/%s/ (escritura solo en out/).\n" % (role, why, role))
    sys.exit(2)


def check_bash(cmd, sandbox):
    try:
        parts = shlex.split(cmd)
    except ValueError:
        return False
    if not parts or any(t in cmd for t in (";", "&&", "||", "|", "`", "$(", ">", "<")):
        return False
    exe = parts[0]
    if exe == "h" or exe.endswith("/bin/h"):
        rest = parts[1:]
        if rest[:2] == ["sandbox", "check-impl"] and len(rest) == 2:
            return True
        if len(rest) == 4 and rest[:2] == ["sandbox", "syntax"] and rest[2] in ("impl", "test"):
            target = Path(rest[3])
            target = target if target.is_absolute() else sandbox / target
            return inside(target, sandbox)
        if rest[:2] == ["sandbox", "current"] and len(rest) == 2:
            return True
        return False
    if exe == "clang-format" and len(parts) == 3 and parts[1] == "-i":
        target = Path(parts[2])
        target = target if target.is_absolute() else sandbox / target
        return inside(target, sandbox / "out")
    return False


def main():
    role = None
    if "--role" in sys.argv:
        role = sys.argv[sys.argv.index("--role") + 1]
    try:
        data = json.load(sys.stdin)
    except ValueError:
        data = {}
    if role is None:
        role = ROLE_BY_AGENT.get(data.get("agent_type") or data.get("subagent_type") or "")
    if role not in ("impl", "test"):
        sys.exit(0)  # no es un agente aislado
    root = find_root(data.get("cwd") or os.getcwd())
    sandbox = root / ".harness" / "sandbox" / "current" / role
    tool = data.get("tool_name", "")
    ti = data.get("tool_input") or {}
    if tool not in ALLOWED_TOOLS:
        deny(root, role, tool, "-", "herramienta %s no permitida" % tool)
    if tool == "TodoWrite":
        sys.exit(0)
    if tool == "Bash":
        cmd = ti.get("command", "")
        if not check_bash(cmd, sandbox):
            deny(root, role, tool, cmd, "comando no permitido; solo 'h sandbox syntax|check-impl' "
                 "y 'clang-format -i <fichero en out/>'")
        audit(root, role, tool, cmd, "ALLOW")
        sys.exit(0)
    if tool in ("Glob", "Grep"):
        target = ti.get("path")
        if not target:
            deny(root, role, tool, "-", "indica 'path' dentro de tu sandbox")
        pat = ti.get("pattern", "") + " " + ti.get("glob", "")
        if ".." in pat or re.search(r"(^|\s)/", pat):
            deny(root, role, tool, pat, "patrón fuera del sandbox")
    else:
        target = ti.get("file_path") or ti.get("notebook_path")
    if not target:
        deny(root, role, tool, "-", "ruta no indicada")
    p = Path(target)
    p = p if p.is_absolute() else Path(data.get("cwd") or root) / p
    base = sandbox / "out" if tool in WRITE_TOOLS else sandbox
    if not inside(p, base):
        deny(root, role, tool, p, "%s fuera de %s" % (p, base.relative_to(root)))
    audit(root, role, tool, p, "ALLOW")
    sys.exit(0)


if __name__ == "__main__":
    main()
