---
description: Fase 1 · Exporta la especificación de requisitos a Word con la plantilla registrada
---
# Exportar requisitos a Word

`H="@@HARNESS@@/bin/h"`

Skill: **export-word**.

1. La plantilla de requisitos es la del usuario (`Requirements_2`, recreada en `plantilla-requisitos.docx`). Si el usuario aporta una versión exportada desde Pages a Word, regístrala con `/plantilla-registrar`.
2. `$H req lint` (avisa si hay errores, pero exporta igualmente un borrador si el usuario quiere).
3. `$H export requisitos` → `docs/export/requisitos.docx`.
4. Verifica el resultado: abre el `.docx` con python-docx o conviértelo a PDF con LibreOffice si está disponible y revisa portada, tablas de requisito y figuras. Corrige el mapa de plantilla si algo no encaja (preguntando al usuario).
5. Entrega la ruta del fichero al usuario. Los Word de `docs/export/` se versionan: commitea la exportación al cerrar la fase.
