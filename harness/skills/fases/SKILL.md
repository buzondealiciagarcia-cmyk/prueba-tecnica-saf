---
name: fases
description: Proceso por fases con puertas del proyecto (requisitos, diseño, implementación, oferta). Úsala para saber en qué fase se está, qué se puede tocar y qué falta para cerrar.
---
# Fases y puertas (STD-PRO)

CLI: `H="@@HARNESS@@/bin/h"`

| Fase | Clave | Carpeta(s) que desbloquea | Rama | Cierre (solo usuario) | Tag |
|---|---|---|---|---|---|
| Requisitos | `req` | `docs/01-requisitos` | `docs/requisitos` | `/req-cerrar` | `req-vX.Y` |
| Diseño | `design` | `docs/02-diseno` | `docs/diseno` | `/diseno-cerrar` | `design-vX.Y` |
| Implementación | `impl` | `docs/03-implementacion`, `src`, `include`, `tests`, `frontend`, `deploy`, `config`, `tools` | `docs/implementacion` + `feature/*` | `/impl-cerrar` | `impl-vX.Y` |
| Oferta | `offer` | `docs/04-oferta`, `docs/dosier` | `docs/oferta`, `docs/dosier` | `/oferta-cerrar` | `offer-vX.Y` |

- `$H gate status` · `$H gate require <fase>` (falla si está bloqueada o cerrada) · `$H gate check <fase>` (checklist) · `$H gate close <fase>` · `$H gate reopen <fase> "<motivo>"`.
- Estados: `locked` (la anterior no está cerrada), `open`, `closed`. Cerrar abre la siguiente; reabrir bloquea todas las posteriores.
- El hook `phase_guard.py` bloquea Write/Edit en carpetas de fases no abiertas. Si te bloquea, no lo esquives: informa al usuario.
- Checklist por fase (resumen): **req** → lint STD-REQ estricto, todo `Aprobado`, sin dudas abiertas. **design** → sin marcadores, revisión sin hallazgos abiertos, todo Must/Should citado, diagramas al día, alternativas marcadas "no se implementa", comparativa válida y renderizada. **impl** → plan completo integrado, integración PASS contenida en `main`, documento sin marcadores, trazabilidad sin requisitos *Test* sin casos. **offer** → 9 secciones aprobadas, sin marcadores, PowerPoint generado.
- Nunca cierres ni reabras por iniciativa propia; ni siquiera si el checklist pasa.
