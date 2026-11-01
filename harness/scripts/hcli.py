#!/usr/bin/env python3
"""CLI `h` del harness. Ejecuta `h --help` o `h <comando> --help`."""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from hlib import common as c  # noqa: E402


def build_parser():
    p = argparse.ArgumentParser(prog="h", description="Harness del proyecto: fases, documentos, "
                                "tests y git. Estándares en harness/standards.")
    sp = p.add_subparsers(dest="cmd", required=True)

    x = sp.add_parser("init", help="inicializa el proyecto (config, estado, docs, hooks git)")
    x.add_argument("--name")
    x.add_argument("--code")
    x.add_argument("--author")
    x.add_argument("--client")
    x.add_argument("--module")

    sp.add_parser("doctor", help="comprueba dependencias e indica cómo instalarlas")

    x = sp.add_parser("scaffold", help="crea el esqueleto de una fase o componente")
    x.add_argument("kind", choices=["design", "impl", "offer", "dossier", "c-project", "deploy",
                                    "frontend"])

    x = sp.add_parser("gate", help="puertas de fase")
    gs = x.add_subparsers(dest="gate_cmd", required=True)
    gs.add_parser("status")
    for name in ("require", "check"):
        y = gs.add_parser(name)
        y.add_argument("phase", choices=c.PHASES)
    y = gs.add_parser("close")
    y.add_argument("phase", choices=c.PHASES)
    y.add_argument("--no-git", action="store_true")
    y = gs.add_parser("reopen")
    y.add_argument("phase", choices=c.PHASES)
    y.add_argument("reason")
    y.add_argument("--no-git", action="store_true")
    y = gs.add_parser("can-write", help="(hooks) ¿se puede escribir en esta ruta?")
    y.add_argument("path")

    x = sp.add_parser("req", help="requisitos")
    rs = x.add_subparsers(dest="req_cmd", required=True)
    y = rs.add_parser("lint")
    y.add_argument("--strict", action="store_true")
    rs.add_parser("questions", help="dudas abiertas")
    rs.add_parser("list", help="lista de requisitos (JSON)")

    x = sp.add_parser("diagram", help="diagramas Mermaid con el tema del proyecto")
    ds = x.add_subparsers(dest="diagram_cmd", required=True)
    y = ds.add_parser("render")
    y.add_argument("paths", nargs="*")
    y.add_argument("--all", action="store_true", help="renderiza también los actualizados")
    y = ds.add_parser("check")
    y.add_argument("paths", nargs="*")

    x = sp.add_parser("matrix", help="matrices ponderadas de la comparativa")
    ms = x.add_subparsers(dest="matrix_cmd", required=True)
    ms.add_parser("render")
    ms.add_parser("validate")

    x = sp.add_parser("export", help="exporta un markdown a Word con la plantilla registrada")
    x.add_argument("doc", help="requisitos|diseno|comparativa|implementacion|oferta|dosier|<ruta.md>")
    x.add_argument("-o", "--out")
    x.add_argument("--no-toc", action="store_true")

    x = sp.add_parser("template", help="plantillas Word")
    ts = x.add_subparsers(dest="template_cmd", required=True)
    y = ts.add_parser("build-base")
    y.add_argument("--out", default=str(c.HARNESS_HOME / "templates" / "docx" / "plantilla-base.docx"))
    y = ts.add_parser("inspect")
    y.add_argument("path")
    y.add_argument("--json", action="store_true")
    y = ts.add_parser("register")
    y.add_argument("path")
    y.add_argument("--name")
    ts.add_parser("validate")

    x = sp.add_parser("git", help="flujo git (STD-GIT)")
    gt = x.add_subparsers(dest="git_cmd", required=True)
    y = gt.add_parser("start")
    y.add_argument("name")
    y = gt.add_parser("start-phase")
    y.add_argument("phase", choices=c.PHASES)
    y = gt.add_parser("start-feature")
    y.add_argument("fid")
    y = gt.add_parser("finish")
    y.add_argument("--branch")
    y.add_argument("--refs")
    y = gt.add_parser("release")
    y.add_argument("--tag")
    y.add_argument("--message")
    y = gt.add_parser("check-branch")
    y.add_argument("name")
    y = gt.add_parser("verify-push", help="(pre-push) ¿la rama feature tiene test PASS en sha?")
    y.add_argument("branch")
    y.add_argument("sha")
    y = gt.add_parser("push")
    y.add_argument("refs", nargs="+")

    x = sp.add_parser("impl", help="plan de implementación")
    im = x.add_subparsers(dest="impl_cmd", required=True)
    im.add_parser("validate")
    im.add_parser("next")
    im.add_parser("status")
    y = im.add_parser("set")
    y.add_argument("fid")
    y.add_argument("estado")
    y = im.add_parser("branch")
    y.add_argument("fid")

    x = sp.add_parser("sandbox", help="sandboxes aislados de implementación y test")
    sb = x.add_subparsers(dest="sb_cmd", required=True)
    y = sb.add_parser("prepare")
    y.add_argument("fid")
    y.add_argument("--keep", action="store_true", help="conserva entregas previas (iteraciones)")
    y = sb.add_parser("collect")
    y.add_argument("role", choices=["test", "impl"])
    y.add_argument("-m", "--message")
    y = sb.add_parser("syntax")
    y.add_argument("role", choices=["test", "impl"])
    y.add_argument("file")
    sb.add_parser("check-impl")
    sb.add_parser("current")
    sb.add_parser("clean")

    x = sp.add_parser("test", help="tests + estándares de una función o de todo")
    x.add_argument("fid", nargs="?")
    x.add_argument("--all", action="store_true")

    x = sp.add_parser("log", help="log de tests y cambios")
    ls = x.add_subparsers(dest="log_cmd", required=True)
    y = ls.add_parser("change")
    y.add_argument("fid")
    y.add_argument("--agent", required=True)
    y.add_argument("--cause", required=True)
    y.add_argument("--desc", required=True)

    sp.add_parser("trace", help="matriz de trazabilidad")

    x = sp.add_parser("offer", help="oferta")
    of = x.add_subparsers(dest="offer_cmd", required=True)
    of.add_parser("status")
    y = of.add_parser("section")
    y.add_argument("section")
    y.add_argument("state")
    of.add_parser("budget")

    x = sp.add_parser("pptx", help="presentación PowerPoint")
    pp = x.add_subparsers(dest="pptx_cmd", required=True)
    y = pp.add_parser("build")
    y.add_argument("spec", nargs="?")
    y.add_argument("-o", "--out")

    x = sp.add_parser("dossier", help="dosier de las fases 1-3")
    dd = x.add_subparsers(dest="dossier_cmd", required=True)
    dd.add_parser("build")
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    cmd = args.cmd
    if cmd == "init":
        from hlib import init
        return init.init(args.name, args.code, args.author, args.module, args.client)
    if cmd == "doctor":
        from hlib import init
        return init.doctor()
    if cmd == "scaffold":
        from hlib import init
        return init.scaffold(args.kind)
    if cmd == "gate":
        from hlib import gate
        g = args.gate_cmd
        if g == "status":
            print(gate.status())
            return c.EXIT_OK
        if g == "require":
            gate.require(args.phase)
            return c.EXIT_OK
        if g == "check":
            cl = gate.check(args.phase)
            print(cl.render())
            return c.EXIT_OK if cl.ok else c.EXIT_FAIL
        if g == "close":
            return gate.close(args.phase, use_git=not args.no_git)
        if g == "reopen":
            return gate.reopen(args.phase, args.reason, use_git=not args.no_git)
        if g == "can-write":
            ok, why = gate.write_allowed(args.path)
            print("OK" if ok else why)
            return c.EXIT_OK if ok else c.EXIT_FAIL
    if cmd == "req":
        from hlib import reqs
        if args.req_cmd == "lint":
            errs, warns = reqs.lint(strict=args.strict)
            for e in errs:
                print("ERROR  %s" % e)
            for w in warns:
                print("AVISO  %s" % w)
            print("%d errores, %d avisos" % (len(errs), len(warns)))
            return c.EXIT_FAIL if errs else c.EXIT_OK
        if args.req_cmd == "questions":
            oq = reqs.open_questions()
            print("\n".join(oq) if oq else "sin dudas abiertas")
            return c.EXIT_OK
        if args.req_cmd == "list":
            import json
            print(json.dumps(reqs.load(), indent=2, ensure_ascii=False))
            return c.EXIT_OK
    if cmd == "diagram":
        from hlib import diagram
        if args.diagram_cmd == "render":
            return diagram.render_all(args.paths, only_stale=not args.all)
        return diagram.check(args.paths)
    if cmd == "matrix":
        from hlib import matrix
        if args.matrix_cmd == "render":
            return matrix.render()
        errs = matrix.validate(c.load_json(c.project_root() / matrix.JSON_REL, {}))
        print("\n".join(errs) if errs else "OK")
        return c.EXIT_FAIL if errs else c.EXIT_OK
    if cmd == "export":
        from hlib import export
        if args.out or args.no_toc:
            path = {"requisitos": "docs/01-requisitos/requisitos.md",
                    "diseno": "docs/02-diseno/diseno.md",
                    "comparativa": "docs/02-diseno/comparativa.md",
                    "implementacion": "docs/03-implementacion/implementacion.md",
                    "oferta": "docs/04-oferta/oferta.md",
                    "dosier": "docs/dosier/dosier.md"}.get(args.doc, args.doc)
            p = Path(path)
            return export.export(p if p.is_absolute() else c.project_root() / p, args.out,
                                 toc=not args.no_toc,
                                 kind=args.doc if "/" not in args.doc else None)
        return export.export_named(args.doc)
    if cmd == "template":
        from hlib import template
        t = args.template_cmd
        if t == "build-base":
            return template.build_base(args.out)
        if t == "inspect":
            return template.inspect(args.path, args.json)
        if t == "register":
            return template.register(args.path, args.name)
        if t == "validate":
            cfg = c.load_config()
            tp = c.expand(cfg["export"]["template"])
            mp = c.load_json(c.expand(cfg["export"]["template_map"]), {})
            errs = template.validate_map(mp, tp)
            print("\n".join(errs) if errs else "OK: mapa coherente con %s" % tp.name)
            return c.EXIT_FAIL if errs else c.EXIT_OK
    if cmd == "git":
        from hlib import gitflow
        return gitflow.cli(args)
    if cmd == "impl":
        from hlib import impl
        i = args.impl_cmd
        if i == "validate":
            errs = impl.validate_plan(impl.load_plan())
            print("\n".join(errs) if errs else "OK")
            return c.EXIT_FAIL if errs else c.EXIT_OK
        if i == "next":
            f = impl.next_function()
            print(f["id"] + " " + f["nombre"] if f else "todas las funciones están integradas")
            return c.EXIT_OK
        if i == "status":
            plan = impl.load_plan()
            print("| Función | Nombre | Requisitos | Estado | Iteraciones fallidas |")
            print("|---|---|---|---|---|")
            for f in plan.get("funciones", []):
                print("| %s | `%s` | %s | %s | %s |" % (f["id"], f["nombre"],
                                                       ", ".join(f.get("requisitos", [])),
                                                       f.get("estado", "pendiente"),
                                                       f.get("iteraciones", 0)))
            return c.EXIT_OK
        if i == "set":
            impl.set_state(args.fid, args.estado)
            return c.EXIT_OK
        if i == "branch":
            print(impl.branch_name(args.fid))
            return c.EXIT_OK
    if cmd == "sandbox":
        from hlib import impl
        s = args.sb_cmd
        if s == "prepare":
            return impl.prepare(args.fid, keep=args.keep)
        if s == "collect":
            return impl.collect(args.role, message=args.message)
        if s == "syntax":
            return impl.syntax(args.file, args.role)
        if s == "check-impl":
            return impl.check_impl()
        if s == "current":
            print(impl.current() or "sin sandbox activo")
            return c.EXIT_OK
        if s == "clean":
            return impl.cleanup()
    if cmd == "test":
        from hlib import testrun
        if args.all:
            return testrun.run_all()
        if not args.fid:
            c.die("indica F-NNN o --all", c.EXIT_USAGE)
        return testrun.run_function(args.fid)
    if cmd == "log":
        from hlib import testrun
        return testrun.log_change(args.fid, args.agent, args.cause, args.desc)
    if cmd == "trace":
        from hlib import trace
        return trace.render()
    if cmd == "offer":
        from hlib import offer
        if args.offer_cmd == "status":
            return offer.status()
        if args.offer_cmd == "section":
            return offer.set_section_state(args.section, args.state)
        return offer.render_budget()
    if cmd == "pptx":
        from hlib import pptx_build
        return pptx_build.build(args.spec, args.out)
    if cmd == "dossier":
        from hlib import dossier
        return dossier.build()
    return c.EXIT_USAGE


if __name__ == "__main__":
    sys.exit(main())
