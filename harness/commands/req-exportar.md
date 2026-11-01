---
description: Fase 1 · Exporta la especificación de requisitos a Word con la plantilla registrada
---
# Exportar requisitos a Word

`H="@@HARNESS@@/bin/h"`

Skill: **export-word**.

1. Comprueba la plantilla activa (`harness.config.json → export.template`). Si sigue siendo la base del harness y el usuario tiene la suya, sugiere `/plantilla-registrar`.
2. `$H req lint` (avisa si hay errores, pero exporta igualmente un borrador si el usuario quiere).
3. `$H export requisitos` → `docs/export/requisitos.docx`.
4. Verifica el resultado: abre el `.docx` con python-docx o conviértelo a PDF con LibreOffice si está disponible y revisa portada, tablas de requisito y figuras. Corrige el mapa de plantilla si algo no encaja (preguntando al usuario).
5. Entrega la ruta del fichero al usuario. No se commitea (`docs/export/` no se versiona).
