#!/usr/bin/env python3
"""Genera los escenarios del visor del grid ejecutando el código C real de `ogrid`.

Compila `src/ogrid/*.c` en una biblioteca dinámica, llama a las funciones públicas
(ogrid_reset_grid, ogrid_update_ray, ogrid_get_cell_state, ogrid_log_odds_to_prob,
ogrid_extract_occupied) con una serie de escenarios y guarda, ciclo a ciclo, el estado de una
ventana del grid en `frontend/data/escenarios.json`. Los números que muestra la página los
calcula ese código, no una copia en otro lenguaje.

Uso:
  tools/ogrid_demo.py            genera frontend/data/escenarios.json
  tools/ogrid_demo.py --serve    lo genera y sirve frontend/ en http://localhost:8080

Herramienta de la interfaz (STD-UI), solo lectura: no forma parte del sistema ni del núcleo C.
"""
import argparse
import ctypes
import json
import platform
import re
import shutil
import subprocess
import sys
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "frontend" / "data" / "escenarios.json"
BUILD = ROOT / "build" / "ui"

# Ventana del grid que se muestra: celdas s = S0 .. S0 + ROWS - 1, en las 4 columnas d.
S0, ROWS = 100, 24

STATE_NAMES = {0: "zona no estudiada", 1: "probabilidad baja de obstáculo", 2: "ocupado"}


def read_limits():
    """Lee las constantes del modelo de ogrid_limits.h para no duplicarlas aquí."""
    text = (ROOT / "include" / "ogrid" / "ogrid_limits.h").read_text(encoding="utf-8")

    def number(name):
        m = re.search(r"#define OGRID_%s\s+\((-?[0-9.]+)[FU]?\)" % name, text)
        if not m:
            raise RuntimeError("no se encontró OGRID_%s en ogrid_limits.h" % name)
        return float(m.group(1))

    return {
        "vote_echo": number("VOTE_ECHO"), "vote_pass": number("VOTE_PASS"),
        "lo_max": number("LOG_ODDS_MAX"), "lo_min": number("LOG_ODDS_MIN"),
        "occupied_prob": number("OCCUPIED_PROB_THRESHOLD"),
        "width_cells": int(number("WIDTH_CELLS")), "length_cells": int(number("LENGTH_CELLS")),
    }


def build_library():
    BUILD.mkdir(parents=True, exist_ok=True)
    suffix = ".dylib" if platform.system() == "Darwin" else ".so"
    lib = BUILD / ("libogrid" + suffix)
    sources = sorted(str(p) for p in (ROOT / "src" / "ogrid").glob("*.c"))
    cc = shutil.which("gcc-16") or shutil.which("cc") or "cc"
    cmd = [cc, "-std=c11", "-O0", "-shared", "-fPIC", "-I", str(ROOT / "include"),
           "-o", str(lib)] + sources + ["-lm"]
    subprocess.run(cmd, check=True)
    return lib


class Cell(ctypes.Structure):
    _fields_ = [("log_odds_lo", ctypes.c_float), ("observed", ctypes.c_uint8)]


def load(lib_path):
    lib = ctypes.CDLL(str(lib_path))
    count = 880 * 4

    class Grid(ctypes.Structure):
        _fields_ = [("cells", Cell * count)]

    lib.ogrid_reset_grid.argtypes = [ctypes.POINTER(Grid)]
    lib.ogrid_update_ray.argtypes = [ctypes.POINTER(Grid), ctypes.POINTER(ctypes.c_uint32),
                                     ctypes.c_uint32, ctypes.c_bool, ctypes.c_uint32]
    lib.ogrid_get_cell_state.argtypes = [ctypes.POINTER(Grid), ctypes.c_uint32,
                                         ctypes.POINTER(ctypes.c_int)]
    lib.ogrid_log_odds_to_prob.argtypes = [ctypes.c_float, ctypes.POINTER(ctypes.c_float)]
    lib.ogrid_extract_occupied.argtypes = [ctypes.POINTER(Grid),
                                           ctypes.POINTER(ctypes.c_bool)]
    return lib, Grid, count


def check(status, what):
    if status != 0:
        raise RuntimeError("%s devolvió el código %d" % (what, status))


class Run:
    """Un grid vivo y las operaciones sobre él."""

    def __init__(self, lib, grid_type, count):
        self.lib, self.count = lib, count
        self.grid = grid_type()
        self.mask = (ctypes.c_bool * count)()

    def reset(self):
        check(self.lib.ogrid_reset_grid(ctypes.byref(self.grid)), "ogrid_reset_grid")

    def ray(self, d, s_from, s_to, echo_s):
        cells = [s * 4 + d for s in range(s_from, s_to + 1)]
        arr = (ctypes.c_uint32 * len(cells))(*cells)
        has_echo = echo_s is not None
        pos = cells.index(echo_s * 4 + d) if has_echo else 0
        check(self.lib.ogrid_update_ray(ctypes.byref(self.grid), arr, len(cells), has_echo, pos),
              "ogrid_update_ray")

    def snapshot(self, label, rays):
        check(self.lib.ogrid_extract_occupied(ctypes.byref(self.grid), self.mask),
              "ogrid_extract_occupied")
        window = []
        for s in range(S0, S0 + ROWS):
            for d in range(4):
                idx = s * 4 + d
                cell = self.grid.cells[idx]
                state = ctypes.c_int()
                check(self.lib.ogrid_get_cell_state(ctypes.byref(self.grid), idx,
                                                    ctypes.byref(state)), "ogrid_get_cell_state")
                p = None
                if cell.observed:
                    prob = ctypes.c_float()
                    check(self.lib.ogrid_log_odds_to_prob(cell.log_odds_lo, ctypes.byref(prob)),
                          "ogrid_log_odds_to_prob")
                    p = round(prob.value, 4)
                window.append({"s": s, "d": d, "lo": round(cell.log_odds_lo, 4),
                               "obs": int(cell.observed), "state": state.value, "p": p,
                               "occ": bool(self.mask[idx])})
        observed = sum(1 for i in range(self.count) if self.grid.cells[i].observed)
        occupied = sum(1 for i in range(self.count) if self.mask[i])
        return {"label": label, "rays": rays, "cells": window,
                "total_observed": observed, "total_occupied": occupied}


def scenarios():
    """Cada escenario: título, explicación, celda a seguir y lista de ciclos (cada ciclo, sus haces).

    Un haz es (d, s_inicio, s_fin, s_del_eco o None): recorre las celdas de la columna d desde
    s_inicio hasta s_fin; si hay eco, en la celda s_del_eco.
    """
    return [
        {"id": "eco-aislado", "title": "Un eco aislado no confirma",
         "text": "Un haz recorre la columna 1 y devuelve un eco en la celda 110. Las celdas antes del "
                 "eco reciben «pasó sin eco» (−0,40) y la del eco recibe +0,85: p ≈ 0,70, todavía "
                 "probabilidad baja. Las celdas detrás del eco quedan sin cambio.",
         "focus": [110, 1],
         "cycles": [("Ciclo 1: un eco en la celda 110", [(1, 100, 113, 110)])]},
        {"id": "tres-ecos", "title": "Tres ecos seguidos confirman",
         "text": "El mismo eco en tres ciclos seguidos suma 0,85, 1,70 y 2,55. Solo en el tercero la "
                 "probabilidad supera 0,85 y la celda pasa a ocupado (borde rojo).",
         "focus": [110, 1],
         "cycles": [("Ciclo %d: eco en la celda 110" % i, [(1, 100, 113, 110)]) for i in (1, 2, 3)]},
        {"id": "racha-de-pasos", "title": "Tras una racha de pasos, tres ecos siguen confirmando",
         "text": "Tres ciclos sin eco bajan la celda 108 al mínimo, −0,8. Después, tres ecos la suben a "
                 "0,05, 0,90 y 1,75. El límite inferior de −0,8 garantiza que tres ecos bastan "
                 "siempre para confirmar un obstáculo.",
         "focus": [108, 2],
         "cycles": [("Ciclo %d: haz sin eco" % i, [(2, 100, 113, None)]) for i in (1, 2, 3)] +
                   [("Ciclo %d: eco en la celda 108" % (i + 3), [(2, 100, 113, 108)])
                    for i in (1, 2, 3)]},
        {"id": "celda-oculta", "title": "Una celda detrás de un eco no cambia",
         "text": "Primero un haz sin eco deja las celdas en −0,40. En el segundo ciclo hay un eco en "
                 "la celda 105: las anteriores bajan a −0,80, la del eco sube a 0,45 y las de detrás "
                 "no cambian, porque el radar no ve a través del obstáculo.",
         "focus": [108, 0],
         "cycles": [("Ciclo 1: haz sin eco", [(0, 100, 115, None)]),
                    ("Ciclo 2: eco en la celda 105", [(0, 100, 115, 105)])]},
        {"id": "varios-haces", "title": "Varios haces por ciclo y un objeto de varias celdas",
         "text": "Cuatro haces por ciclo, uno por columna. Los de las columnas 1 y 2 chocan con un "
                 "objeto en la celda 110; los de las columnas 0 y 3 pasan sin eco. A los tres ciclos "
                 "quedan dos celdas ocupadas, que la máscara de celdas ocupadas devuelve juntas.",
         "focus": [110, 1],
         "cycles": [("Ciclo %d: cuatro haces" % i,
                     [(0, 100, 113, None), (1, 100, 113, 110), (2, 100, 113, 110),
                      (3, 100, 113, None)]) for i in (1, 2, 3)]},
        {"id": "reinicio", "title": "El reinicio olvida todo",
         "text": "Tres ecos confirman un obstáculo y después se reinicia el grid, como al volver de un "
                 "fallo. Todas las celdas vuelven a «zona no estudiada»: las mediciones anteriores "
                 "dejan de ser fiables.",
         "focus": [110, 1],
         "cycles": [("Ciclo %d: eco en la celda 110" % i, [(1, 100, 113, 110)]) for i in (1, 2, 3)] +
                   [("Reinicio del grid", "reset")]},
    ]


def generate():
    lib_path = build_library()
    lib, grid_type, count = load(lib_path)
    out = {"version": 1, "window": {"s0": S0, "rows": ROWS, "cols": 4},
           "constants": read_limits(), "state_names": STATE_NAMES, "scenarios": []}
    for sc in scenarios():
        run = Run(lib, grid_type, count)
        run.reset()
        steps = [run.snapshot("Estado inicial: grid reiniciado", [])]
        for label, rays in sc["cycles"]:
            if rays == "reset":
                run.reset()
                steps.append(run.snapshot(label, []))
                continue
            for d, s_from, s_to, echo_s in rays:
                run.ray(d, s_from, s_to, echo_s)
            steps.append(run.snapshot(label, [{"d": d, "s_from": a, "s_to": b, "echo_s": e}
                                              for d, a, b, e in rays]))
        out["scenarios"].append({"id": sc["id"], "title": sc["title"], "text": sc["text"],
                                 "focus": {"s": sc["focus"][0], "d": sc["focus"][1]},
                                 "steps": steps})
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(out, ensure_ascii=False, separators=(",", ":")) + "\n",
                   encoding="utf-8")
    print("escenarios escritos en %s (%d escenarios)" % (OUT.relative_to(ROOT),
                                                          len(out["scenarios"])))


def serve(port):
    handler = partial(SimpleHTTPRequestHandler, directory=str(ROOT / "frontend"))
    server = ThreadingHTTPServer(("127.0.0.1", port), handler)
    print("visor en http://localhost:%d (Ctrl+C para parar)" % port)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--serve", action="store_true", help="sirve frontend/ tras generar los datos")
    ap.add_argument("--port", type=int, default=8080)
    args = ap.parse_args()
    generate()
    if args.serve:
        serve(args.port)
    return 0


if __name__ == "__main__":
    sys.exit(main())
