# prueba-saf

Detección de obstáculos estáticos en la vía de un tranvía con un radar frontal 3D. El proyecto parte de la «Prueba práctica SAF: detección de obstáculos con radar» del equipo SAF (unidad DIG) de Ikerlan, que pide un algoritmo basado en un grid de ocupación dinámico para identificar obstáculos sobre la vía, y recorre cuatro fases: requisitos, diseño, implementación y oferta.

| | |
|---|---|
| Código | `SAF` · módulo C `ogrid` |
| Requisitos | 64 requisitos y 8 hipótesis (versión 1.2) |
| Diseño | sistema completo: radar 3D, ECU a bordo, HMI y nube (versión 1.2) |
| Implementación | módulo del grid de ocupación: 7 funciones en C, 209 casos de test (versión 1.2) |
| Oferta | oferta técnica y económica en dos fases, con presentación (versión 1.0) |

## El problema

Un tranvía a 50 km/h recorre unos 14 m cada segundo. El sistema debe detectar objetos estáticos de 0,6 m o más situados en la vía a menos de 220 m, con un único radar frontal que entrega una nube de puntos cada 50 ms. Debe entregarlos en menos de 128 ms, pasar a modo degradado o declarar un error ante fallos del sensor y no dar nunca la vía por libre sin haberlo comprobado.

## Qué hay implementado

El **grid de ocupación** (`include/ogrid/`, `src/ogrid/`): una rejilla fija de 880 × 4 celdas de 0,25 m, sin memoria dinámica. Cada celda guarda un log-odds: un eco suma 0,85, una onda que pasa sin eco resta 0,40 y una celda oculta tras un eco no cambia. Con tres ecos seguidos la celda pasa a ocupada. El estado de una celda es ocupado, probabilidad baja de obstáculo o zona no estudiada, y nunca «libre».

| Función | Qué hace |
|---|---|
| `ogrid_reset_grid` | Deja todo el grid en «zona no estudiada» |
| `ogrid_log_odds_to_prob` | Convierte el log-odds en probabilidad |
| `ogrid_clamp_log_odds` | Limita el log-odds a [−0,8; 3,4] |
| `ogrid_update_cell` | Aplica el voto de un haz a una celda |
| `ogrid_get_cell_state` | Deriva el estado de una celda |
| `ogrid_update_ray` | Aplica un haz del radar a las celdas que recorre |
| `ogrid_extract_occupied` | Devuelve la máscara de celdas ocupadas |

Lo demás (agrupación en objetos, geometría de vía, máquina de estados y comunicaciones) está diseñado y no implementado: es la Fase B de la oferta.

## Cómo construir y probar

Hace falta `cmake`, un compilador C11 (el proyecto usa `gcc-16` en los presets de test, instalable con `brew install gcc`) y, para los controles completos, las herramientas del harness (ver más abajo).

```bash
# tests con detección de errores de memoria y cobertura (usa el harness)
.claude/harness/bin/h test --all

# medida de tiempos en Release, sin sanitizers
cmake --preset host-bench && cmake --build build/host-bench
build/host-bench/bench_wcet_ogrid

# visor del grid con el código C real (http://localhost:8080)
python3 tools/ogrid_demo.py --serve
```

`h test --all` compila, ejecuta los 209 tests (ASan y UBSan) y comprueba cobertura del 100 % de líneas y ramas, MISRA C:2012, clang-tidy, formato, complejidad, Doxygen y símbolos prohibidos. Los tiempos medidos están en `docs/03-implementacion/wcet-medidas.md`.

## Documentación

Cada fase tiene su documento en Markdown, versionado en git, y se exporta a Word.

| Fase | Documentos |
|---|---|
| 1 · Requisitos | `docs/01-requisitos/requisitos.md` y `dudas.md` |
| 2 · Diseño | `docs/02-diseno/diseno.md`, `viabilidad-radar.md` y `justificacion-c.md`, con los diagramas en `docs/02-diseno/diagramas/` |
| 3 · Implementación | `docs/03-implementacion/implementacion.md`, con `contratos/`, `casos/`, `trazabilidad.md` y `wcet-medidas.md` |
| 4 · Oferta | `docs/04-oferta/oferta.md`, `presupuesto.json` y `presentacion.json` |
| Dosier | `docs/dosier/contexto.md` y `dosier.md` (generado), que reúne las fases 1 a 3 |

Los Word y la presentación se escriben en `docs/export/`, que no se versiona. Se regeneran con el harness:

```bash
.claude/harness/bin/h export requisitos     # o diseno, implementacion, oferta, dosier
.claude/harness/bin/h pptx build            # presentación de la oferta
```

## Estructura

```text
harness/            harness de ingeniería para Claude Code (se instala en .claude/)
docs/               documentos de cada fase y dosier
include/ogrid/      cabeceras públicas del módulo
src/ogrid/          código C del grid de ocupación (una función por fichero)
tests/              unit/ e integration/ (incluye el benchmark de tiempos)
frontend/ tools/    visor del grid y su generador de escenarios
config/ cmake/      desviaciones MISRA, supresiones y toolchain de compilación cruzada
third_party/        Unity y CMock
.harness/           estado de las fases (state.json)
```

## El harness

El proyecto se ha desarrollado con un harness de ingeniería para Claude Code, que impone el flujo por fases con puertas, la independencia entre código y tests, la verificación automática y el control de versiones. Está en `harness/` y se instala en `.claude/`. Cómo instalarlo, cómo lanzarlo y qué contiene cada componente (comandos, agentes, skills, estándares, hooks y CLI) se explica en [`harness/README.md`](harness/README.md).

Para empezar con él:

```bash
./harness/install.sh --target .     # instala el harness en ./.claude
claude                              # abre Claude Code en esta carpeta
```

y en Claude Code, `/harness-init` y `/fase-estado`.
