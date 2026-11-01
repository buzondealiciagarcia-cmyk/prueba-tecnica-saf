---
description: Fase 4 · Crea la presentación PowerPoint de la oferta con la paleta y estilo del proyecto
---
# Presentación de la oferta

`H="@@HARNESS@@/bin/h"`

Skill: **oferta** (sección "Presentación").

1. `$H gate require offer`; rama `docs/oferta`.
2. Propón un guion de 8-14 diapositivas basado en `oferta.md` y en la información generada (diagramas de diseño, comparativa, métricas de verificación). Tipos disponibles: `titulo`, `seccion`, `vinetas` (≤ 6), `dos_columnas`, `imagen` (acepta `.mmd` de cualquier fase), `tabla`, `kpis` (≤ 4), `cierre`. Cada diapositiva puede llevar `notas` para el ponente.
3. Valida el guion con el usuario (`AskUserQuestion`): audiencia, duración, mensajes clave, qué cifras mostrar. Pregunta lo que no conozcas; no inventes cifras ni logos.
4. Escribe `docs/04-oferta/presentacion.json` y genera `$H pptx build` → `docs/export/oferta-presentacion.pptx`.
5. Revisa el resultado visualmente (convierte a PDF/PNG con LibreOffice si está disponible y mira cada diapositiva): textos que desborden, imágenes pequeñas, densidad. Corrige y regenera.
6. Commitea `docs(offer): add presentation script` y entrega la ruta del `.pptx`.
