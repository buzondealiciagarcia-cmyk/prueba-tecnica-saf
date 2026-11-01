#!/usr/bin/env python3
"""Hook PreToolUse de puertas de fase (PRO-01).

Impide que Claude (o cualquier agente) escriba con Write/Edit en carpetas de una fase
bloqueada (la anterior no está cerrada) o ya cerrada (requiere /fase-reabrir).
Bloquea con código 2; el motivo llega al agente por stderr.
"""
import json
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "scripts"))


def main():
    try:
        data = json.load(sys.stdin)
    except ValueError:
        sys.exit(0)
    ti = data.get("tool_input") or {}
    target = ti.get("file_path") or ti.get("notebook_path")
    if not target:
        sys.exit(0)
    cwd = Path(data.get("cwd") or os.getcwd())
    root = Path(os.environ.get("CLAUDE_PROJECT_DIR") or cwd).resolve()
    if not (root / "harness.config.json").is_file():
        sys.exit(0)  # proyecto sin harness inicializado
    p = Path(target)
    p = (p if p.is_absolute() else cwd / p).resolve()
    try:
        rel = str(p.relative_to(root))
    except ValueError:
        sys.exit(0)
    os.environ["H_PROJECT_ROOT"] = str(root)
    from hlib import gate  # noqa: E402
    ok, why = gate.write_allowed(rel, root)
    if not ok:
        sys.stderr.write("BLOQUEADO por puerta de fase: %s\nConsulta el estado con /fase-estado.\n"
                         % why)
        sys.exit(2)
    sys.exit(0)


if __name__ == "__main__":
    main()
