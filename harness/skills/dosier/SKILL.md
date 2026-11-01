---
name: dosier
description: Une los documentos de requisitos, diseño e implementación en un dosier ordenado, con contexto y exportable a Word. Úsala en /dosier-generar.
---
# Dosier (fases 1-3)

CLI: `H="@@HARNESS@@/bin/h"`

- `$H dossier build` genera `docs/dosier/dosier.md` (no editar a mano):
  1. Portada y tabla de control.
  2. **Contexto del proyecto** desde `docs/dosier/contexto.md` (lo redactas con el usuario).
  3. Guía de lectura, tabla de documentos incluidos (código, versión, fecha, fuente) y estado de fases.
  4. **Parte I — Requisitos**, **Parte II — Diseño** (diseño, alternativas, comparativa), **Parte III — Implementación** (documento de implementación y trazabilidad), cada una con una introducción. Los títulos de cada documento bajan un nivel y las rutas de figuras se reescriben.
- `$H export dosier` → Word con la plantilla registrada.
- Si un documento falta, el dosier lo indica; no lo sustituyas por texto inventado.
