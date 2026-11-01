---
description: Fase 2 · Documenta un diseño alternativo (no se implementa) y valida su viabilidad
argument-hint: "<idea de la alternativa>"
---
# Diseño alternativo

`H="@@HARNESS@@/bin/h"` · Idea: `$ARGUMENTS`

1. `$H gate require design`; rama `docs/diseno`.
2. Asigna el siguiente ID `ALT-NN` y crea `docs/02-diseno/alternativas/ALT-NN-<slug>.md` desde `@@HARNESS@@/templates/docs/alternativa.md` (sustituye `@ALT_ID@`, `@ALT_NAME@`, `@ALT_SLUG@`, `@CODE@`, `@DATE@`). Debe conservar la cabecera **"Alternativa no seleccionada — no se implementa"**.
3. Lanza el agente **arquitecto-alternativas** con la idea, `diseno.md`, `requisitos.md` y los estándares. Desarrolla la alternativa (puede relajar requisitos, exigir más hardware o capacidad), indica explícitamente qué requisitos incumple o relaja, qué recursos adicionales necesita y una valoración de viabilidad técnica, de coste, plazo y riesgos, con sus supuestos visibles.
4. Revisa con el usuario los supuestos y la viabilidad (`AskUserQuestion`); los datos que no se conozcan (coste de un sensor, etc.) se preguntan o quedan como supuesto explícito aceptado por el usuario.
5. Diagrama de la alternativa en `docs/02-diseno/diagramas/<alt-slug>.mmd` + `$H diagram render`.
6. Añade la alternativa a `comparativa.json` (sin puntuar) y commitea `docs(design): add alternative ALT-NN`. Sugiere `/diseno-comparar` cuando estén todas.
