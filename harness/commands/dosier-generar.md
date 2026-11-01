---
description: Une requisitos, diseño e implementación en un dosier ordenado y con contexto, exportable a Word
---
# Dosier del proyecto (fases 1-3)

`H="@@HARNESS@@/bin/h"`

Skill: **dosier**.

1. Requiere la fase de implementación cerrada (`$H gate status`; si no lo está, avisa de que el dosier será provisional y pregunta si continuar).
2. `$H git start docs/dosier` y `$H scaffold dossier`.
3. Redacta con el usuario `docs/dosier/contexto.md`: origen del proyecto, problema, enfoque por fases, alcance implementado y cómo leer el dosier. Pregunta lo que no sepas.
4. `$H trace` (si procede) y `$H dossier build` → `docs/dosier/dosier.md` (portada, contexto, guía de lectura, documentos incluidos con versión, estado de fases y las tres partes con sus documentos, con títulos desplazados y rutas de figuras corregidas).
5. Revisa que el texto de transición entre partes tenga sentido; si algo no encaja, ajusta `contexto.md` (no edites `dosier.md`, es generado).
6. `$H export dosier` → `docs/export/dosier.docx`. Commitea `docs(dossier): build project dossier` e integra con `$H git finish`.
