"""Flujo git del proyecto (STD-GIT): ramas, merges --no-ff, tags de fase y push."""
import re

from . import common as c

BRANCH_RE = re.compile(
    r"^(feature/[A-Za-z0-9]+(-[A-Za-z0-9]+)+"
    r"|fix/[A-Za-z0-9]+(-[A-Za-z0-9]+)*"
    r"|docs/(requisitos|diseno|implementacion|oferta|dosier)"
    r"|test/[a-z0-9]+(-[a-z0-9]+)*"
    r"|ui/[a-z0-9]+(-[a-z0-9]+)*"
    r"|harness/[a-z0-9]+(-[a-z0-9]+)*"
    r"|release/[0-9]+\.[0-9]+\.[0-9]+)$")


def git(*args, check=True, root=None):
    cp = c.run(["git"] + list(args), cwd=root or c.project_root())
    if check and cp.returncode != 0:
        c.die("git %s falló: %s" % (" ".join(args), (cp.stderr or cp.stdout).strip()))
    return cp.stdout.strip()


def current_branch(root=None):
    return git("rev-parse", "--abbrev-ref", "HEAD", root=root)


def head(ref="HEAD", root=None):
    return git("rev-parse", ref, root=root)


def branch_exists(name, root=None):
    return c.run(["git", "show-ref", "--verify", "--quiet", "refs/heads/" + name],
                 cwd=root or c.project_root()).returncode == 0


def is_clean(root=None):
    return git("status", "--porcelain", root=root) == ""


def require_clean(root=None):
    if not is_clean(root):
        c.die("el árbol de trabajo tiene cambios sin commitear (git status)")


def names(root=None):
    cfg = c.load_config(root)["git"]
    return cfg["main"], cfg["develop"]


def ensure_develop(root=None):
    main, dev = names(root)
    if not branch_exists(dev, root):
        git("branch", dev, main, root=root)
        c.info("creada rama %s desde %s" % (dev, main))


def validate_branch_name(name):
    return bool(BRANCH_RE.match(name))


def start(name, root=None):
    """Crea (o cambia a) una rama de trabajo desde develop (GIT-BR-04)."""
    if not validate_branch_name(name):
        c.die("nombre de rama no válido según GIT-BR-04: %s" % name, c.EXIT_USAGE)
    ensure_develop(root)
    _, dev = names(root)
    if current_branch(root) == name:
        return
    require_clean(root)
    if branch_exists(name, root):
        git("checkout", name, root=root)
        # GIT-MG-05: la rama de trabajo se actualiza con develop antes de seguir
        cp = c.run(["git", "merge", "--no-edit", dev], cwd=root or c.project_root())
        if cp.returncode != 0:
            c.run(["git", "merge", "--abort"], cwd=root or c.project_root())
            c.die("conflicto al actualizar %s con %s; resuélvelo manualmente" % (name, dev))
    else:
        git("checkout", "-b", name, dev, root=root)
    c.info("rama activa: %s" % name)


def _merge(src, dst, refs, root=None):
    git("checkout", dst, root=root)
    msg = "Merge branch '%s' into %s" % (src, dst)
    if refs:
        msg += "\n\nRefs: %s" % refs
    cp = c.run(["git", "merge", "--no-ff", "-m", msg, src], cwd=root or c.project_root())
    if cp.returncode != 0:
        c.run(["git", "merge", "--abort"], cwd=root or c.project_root())
        git("checkout", src, root=root)
        c.die("conflicto al integrar %s en %s; resuélvelo en %s (GIT-MG-05)" % (src, dst, src))


def push(refs, root=None):
    cfg = c.load_config(root)["git"]
    if not cfg.get("push", True):
        c.info("push desactivado en harness.config.json")
        return True
    remote = cfg.get("remote", "origin")
    if c.run(["git", "remote", "get-url", remote], cwd=root or c.project_root()).returncode:
        c.log("WARN", "sin remoto configurado; push omitido", remote=remote)
        return False
    cp = c.run(["git", "push", remote] + list(refs) + ["--follow-tags"],
               cwd=root or c.project_root(), timeout=120)
    if cp.returncode != 0:
        c.log("WARN", "push fallido; queda pendiente (GIT-MG-04)",
              detail=(cp.stderr or "").strip().splitlines()[-1:] or "")
        return False
    return True


def finish(branch=None, refs="", root=None, allow_untested=False):
    """Integra la rama de trabajo en develop con --no-ff (GIT-MG-01/02)."""
    require_clean(root)
    _, dev = names(root)
    branch = branch or current_branch(root)
    if branch in names(root):
        c.die("no se integra %s en sí misma" % branch, c.EXIT_USAGE)
    if branch.startswith("feature/") and not allow_untested:
        from . import impl  # import tardío para evitar ciclos
        fid = impl.function_for_branch(branch, root)
        if not fid or not impl.last_run_passed(fid, head(branch, root), root):
            c.die("GIT-MG-02: %s no tiene un 'h test %s' en PASS sobre su último commit"
                  % (branch, fid or "<F-NNN>"))
        if current_branch(root) != branch:
            git("checkout", branch, root=root)
        impl.set_state(fid, "integrada", root)
        git("add", str(impl.PLAN_REL), root=root)
        git("commit", "-m", "chore(impl): mark %s as integrated" % fid, "-m", "Refs: %s" % fid,
            root=root)
        refs = refs or fid
    _merge(branch, dev, refs, root)
    push([branch, dev], root)
    c.info("integrada %s en %s" % (branch, dev))


def release(tag=None, message="", root=None):
    """Integra develop en main (--no-ff) y etiqueta (GIT-MG-03, GIT-TG-*)."""
    require_clean(root)
    main, dev = names(root)
    _merge(dev, main, "", root)
    if tag:
        git("tag", "-a", tag, "-m", message or tag, root=root)
    push([dev, main], root)
    git("checkout", dev, root=root)
    c.info("develop integrada en %s%s" % (main, (" con tag " + tag) if tag else ""))


def close_phase(phase, version, root=None):
    """Commit del cierre en la rama de fase, merge a develop y a main, tag de fase."""
    branch = c.PHASE_BRANCH[phase]
    if current_branch(root) != branch:
        start(branch, root)
    git("add", "-A", root=root)
    if not is_clean(root):
        scope = {"req": "req", "design": "design", "impl": "impl", "offer": "offer"}[phase]
        git("commit", "-m", "docs(%s): close %s phase v%s" % (scope, phase, version), root=root)
    _merge(branch, names(root)[1], "", root)
    tag = "%s-v%s" % (c.PHASE_TAG[phase], version)
    release(tag, "Fase %s cerrada (v%s)" % (c.PHASE_LABEL[phase], version), root)
    return tag


def tag_exists(tag, root=None):
    return c.run(["git", "rev-parse", "-q", "--verify", "refs/tags/" + tag],
                 cwd=root or c.project_root()).returncode == 0


def cli(args):
    sub = args.git_cmd
    if sub == "start":
        start(args.name)
    elif sub == "start-phase":
        start(c.PHASE_BRANCH[args.phase])
    elif sub == "start-feature":
        from . import impl
        start(impl.branch_name(args.fid))
    elif sub == "finish":
        finish(args.branch, args.refs or "")
    elif sub == "release":
        release(args.tag, args.message or "")
    elif sub == "check-branch":
        ok = validate_branch_name(args.name)
        c.info("OK" if ok else "INVALID")
        return c.EXIT_OK if ok else c.EXIT_FAIL
    elif sub == "verify-push":
        from . import impl
        fid = impl.function_for_branch(args.branch)
        ok = bool(fid) and impl.last_run_passed(fid, args.sha)
        return c.EXIT_OK if ok else c.EXIT_FAIL
    elif sub == "push":
        return c.EXIT_OK if push(args.refs) else c.EXIT_FAIL
    return c.EXIT_OK
