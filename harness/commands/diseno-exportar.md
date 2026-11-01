---
description: Fase 2 · Exporta a Word el documento de diseño y/o la comparativa de alternativas
argument-hint: "[diseno|comparativa|todo]"
---
# Exportar diseño

`H="@@HARNESS@@/bin/h"` · Qué: `$ARGUMENTS` (por defecto `todo`)

Skill: **export-word**.

1. `$H diagram render` (diagramas al día) y, si existe `comparativa.json`, `$H matrix render`.
2. `$H export diseno` y/o `$H export comparativa`.
3. Revisa los `.docx` (portada, figuras legibles, tablas con el formato de la plantilla). Si una tabla o figura no encaja con la plantilla, pregunta al usuario cómo generalizar y registra la decisión en `config/template-map.json → decisions`.
4. Entrega las rutas en `docs/export/`.
