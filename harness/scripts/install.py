#!/usr/bin/env python3
"""Instalador del harness en <destino>/.claude (lo invoca install.sh).

- commands/ agents/ skills/   -> <destino>/.claude/{commands,agents,skills}/
- runtime (bin, scripts, standards, tokens, templates, hooks, git-hooks, settings,
  CLAUDE.harness.md, VERSION) -> <destino>/.claude/harness/
- Sustituye @@HARNESS@@ por la ruta absoluta de <destino>/.claude/harness.
- Fusiona hooks y permisos en <destino>/.claude/settings.json (entradas marcadas h-harness).
- Añade '@harness/CLAUDE.harness.md' a <destino>/.claude/CLAUDE.md.
- Manifiesto con sha256 para actualizar/desinstalar sin tocar ficheros ajenos o modificados.
Códigos de salida (STD-SH SH-08): 0 OK, 1 fallo, 2 uso incorrecto.
"""
import argparse
import hashlib
import json
import os
import shutil
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parent.parent
TOP_LEVEL = {"commands": "commands", "agents": "agents", "skills": "skills"}
RUNTIME = ["bin", "scripts", "standards", "tokens", "templates", "hooks", "git-hooks",
           "settings", "CLAUDE.harness.md", "VERSION"]
TEXT_SUFFIXES = {".md", ".json", ".py", ".sh", ".txt", ".css", ".js", ".html", ".yml",
                 ".cmake", ".conf", ""}
MANIFEST = Path("harness") / ".manifest.json"
IMPORT_LINE = "@harness/CLAUDE.harness.md"
MARK = "h-harness"


def sha(path):
    h = hashlib.sha256()
    with open(str(path), "rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def claude_dir(target):
    t = Path(os.path.expanduser(target)).resolve()
    return t if t.name == ".claude" else t / ".claude"


def plan_files():
    """[(origen, destino relativo a .claude)]"""
    out = []
    for src_dir, dst_dir in TOP_LEVEL.items():
        for f in sorted((SRC / src_dir).rglob("*")):
            if f.is_file():
                out.append((f, Path(dst_dir) / f.relative_to(SRC / src_dir)))
    for item in RUNTIME:
        p = SRC / item
        if p.is_file():
            out.append((p, Path("harness") / item))
        elif p.is_dir():
            for f in sorted(p.rglob("*")):
                if f.is_file() and "__pycache__" not in f.parts:
                    out.append((f, Path("harness") / f.relative_to(SRC)))
    return out


def render(src, dst, harness_abs):
    data = src.read_bytes()
    if src.suffix in TEXT_SUFFIXES or src.name.startswith("."):
        try:
            text = data.decode("utf-8")
            data = text.replace("@@HARNESS@@", str(harness_abs)).encode("utf-8")
        except UnicodeDecodeError:
            pass
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(data)
    if os.access(str(src), os.X_OK):
        dst.chmod(0o755)


def load_manifest(cdir):
    p = cdir / MANIFEST
    return json.loads(p.read_text(encoding="utf-8")) if p.is_file() else {"files": {}}


def is_ours(entry):
    s = json.dumps(entry)
    return MARK in s or "@@HARNESS@@" in s


def merge_settings(cdir, harness_abs, remove_only=False):
    path = cdir / "settings.json"
    cur = json.loads(path.read_text(encoding="utf-8")) if path.is_file() else {}
    frag = json.loads((SRC / "settings" / "settings.fragment.json").read_text(encoding="utf-8")
                      .replace("@@HARNESS@@", str(harness_abs)))
    frag_perms = {k: set(v) for k, v in frag.get("permissions", {}).items()}
    # quitar lo nuestro
    perms = cur.get("permissions", {})
    for k in ("allow", "deny"):
        if k in perms:
            perms[k] = [x for x in perms[k] if x not in frag_perms.get(k, set())
                        and str(harness_abs) not in x]
    hooks = cur.get("hooks", {})
    for ev, groups in list(hooks.items()):
        new_groups = []
        for g in groups:
            g2 = dict(g)
            g2["hooks"] = [h for h in g.get("hooks", []) if MARK not in h.get("command", "")]
            if g2["hooks"]:
                new_groups.append(g2)
        hooks[ev] = new_groups
        if not new_groups:
            del hooks[ev]
    if not remove_only:
        for k, vals in frag.get("permissions", {}).items():
            perms.setdefault(k, [])
            for v in vals:
                if v not in perms[k]:
                    perms[k].append(v)
        for ev, groups in frag.get("hooks", {}).items():
            hooks.setdefault(ev, []).extend(groups)
    if perms:
        cur["permissions"] = {k: v for k, v in perms.items() if v or k not in ("allow", "deny")}
    if hooks:
        cur["hooks"] = hooks
    elif "hooks" in cur:
        del cur["hooks"]
    if cur.get("permissions") == {}:
        del cur["permissions"]
    if cur:
        path.write_text(json.dumps(cur, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    elif path.is_file():
        path.unlink()


def set_import(cdir, present):
    p = cdir / "CLAUDE.md"
    text = p.read_text(encoding="utf-8") if p.is_file() else ""
    lines = [ln for ln in text.splitlines() if ln.strip() != IMPORT_LINE
             and "harness: reglas del proyecto" not in ln]
    if present:
        lines += ["", "<!-- harness: reglas del proyecto (gestionado por install.sh) -->", IMPORT_LINE]
    new = "\n".join(lines).strip("\n")
    if new:
        p.write_text(new + "\n", encoding="utf-8")
    elif p.is_file():
        p.unlink()


def uninstall(cdir, force, dry):
    man = load_manifest(cdir)
    kept = []
    for rel, digest in sorted(man["files"].items()):
        p = cdir / rel
        if not p.is_file():
            continue
        if sha(p) != digest and not force:
            kept.append(rel)
            continue
        if not dry:
            p.unlink()
    if not dry:
        harness_abs = cdir / "harness"
        merge_settings(cdir, harness_abs, remove_only=True)
        set_import(cdir, False)
        mp = cdir / MANIFEST
        if mp.is_file():
            mp.unlink()
        for d in sorted({(cdir / r).parent for r in man["files"]}, key=lambda x: -len(x.parts)):
            q = d
            while q != cdir and q.is_dir() and not any(q.iterdir()):
                q.rmdir()
                q = q.parent
    return kept


def install(cdir, force, dry, update):
    harness_abs = cdir / "harness"
    man = load_manifest(cdir)
    if man["files"] and not update and not force:
        print("ya instalado en %s (usa --update)" % cdir)
        return 1
    files = plan_files()
    new_man = {"version": (SRC / "VERSION").read_text().strip(), "source": str(SRC), "files": {}}
    skipped, conflicts = [], []
    for src, rel in files:
        dst = cdir / rel
        prev = man["files"].get(str(rel))
        if dst.exists():
            if prev is None and not force:
                conflicts.append(str(rel))
                continue
            if prev is not None and sha(dst) != prev and not force:
                skipped.append(str(rel))
                new_man["files"][str(rel)] = prev
                continue
        if dry:
            print("copiar %s -> %s" % (src.relative_to(SRC), dst))
            continue
        render(src, dst, harness_abs)
        new_man["files"][str(rel)] = sha(dst)
    if not dry:
        # ficheros de versiones anteriores que ya no existen en el origen
        for rel, digest in man["files"].items():
            if rel not in new_man["files"]:
                p = cdir / rel
                if p.is_file() and sha(p) == digest:
                    p.unlink()
        (cdir / MANIFEST).parent.mkdir(parents=True, exist_ok=True)
        (cdir / MANIFEST).write_text(json.dumps(new_man, indent=2) + "\n", encoding="utf-8")
        merge_settings(cdir, harness_abs)
        set_import(cdir, True)
    for c in conflicts:
        print("AVISO  existe y no es del harness (no se toca; --force para sobrescribir): %s" % c,
              file=sys.stderr)
    for s in skipped:
        print("AVISO  modificado localmente (no se actualiza; --force para sobrescribir): %s" % s,
              file=sys.stderr)
    print("harness %s instalado en %s (%d ficheros)" % (new_man["version"], cdir,
                                                       len(new_man["files"])))
    print("CLI: %s/bin/h   ·   en Claude Code: /harness-init" % harness_abs)
    return 1 if conflicts else 0


def main():
    ap = argparse.ArgumentParser(description="Instala el harness en <destino>/.claude")
    ap.add_argument("--target", required=True, help="carpeta destino (o su .claude)")
    ap.add_argument("--update", action="store_true", help="actualiza una instalación existente")
    ap.add_argument("--uninstall", action="store_true")
    ap.add_argument("--force", action="store_true", help="sobrescribe/borra aunque haya cambios")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    cdir = claude_dir(args.target)
    if args.uninstall:
        kept = uninstall(cdir, args.force, args.dry_run)
        for k in kept:
            print("AVISO  modificado localmente, no se borra: %s" % k, file=sys.stderr)
        print("harness desinstalado de %s" % cdir)
        return 0
    cdir.mkdir(parents=True, exist_ok=True)
    return install(cdir, args.force, args.dry_run, args.update)


if __name__ == "__main__":
    sys.exit(main())
