---
description: Fase 2 · Entrevista de revisión del diseño: fallos, huecos, mejoras y validaciones
argument-hint: "[apartado concreto a revisar]"
---
# Revisar el diseño

`H="@@HARNESS@@/bin/h"` · Alcance: `$ARGUMENTS` (vacío = todo el diseño)

1. `$H gate require design`; rama `docs/diseno`.
2. Lanza el agente **revisor-diseno** con `diseno.md`, `requisitos.md` (cerrados), `revision.md` y los estándares. Devuelve hallazgos clasificados (**Fallo / Hueco / Mejora / Validado**) con apartado, requisito o regla afectada (p.ej. `SAF-02`, `NET-TMO-01`, `MQ-QOS-03`) y, si procede, una propuesta marcada como tal. No modifica el diseño.
3. Registra los hallazgos nuevos en `revision.md` con estado `Abierto` (IDs `H-NNN`, nunca reutilizados).
4. **Entrevista** al usuario por rondas con `AskUserQuestion` (máx. 4 por ronda, primero Fallos, luego Huecos, luego Mejoras). Para cada hallazgo ofrece: aceptar la propuesta, otra solución (texto libre), aceptar el riesgo tal cual, descartar. Para los *Validado*, pide confirmación en bloque.
5. Aplica a `diseno.md` solo lo que el usuario decida, con sus palabras; anota la decisión en `revision.md` y pasa el hallazgo a `Resuelto`/`Aceptado`/`Descartado`/`Validado`. Actualiza diagramas afectados y `$H diagram render`.
6. Si una decisión del usuario contradice un requisito cerrado, avisa: hay que `/fase-reabrir req` o descartar el cambio.
7. Commitea `docs(design): apply review round N` y resume hallazgos abiertos. Si una idea del usuario abre una vía distinta, sugiere `/diseno-alternativa`.
