---
description: Crea o actualiza un diagrama Mermaid con el estilo del proyecto y lo renderiza a SVG/PNG
argument-hint: "<fase> <nombre> <qué debe mostrar>"
---
# Diagrama

`H="@@HARNESS@@/bin/h"` · Petición: `$ARGUMENTS`

Usa la skill **diagramas**.

1. Comprueba que la fase de destino está abierta (`$H gate require <fase>`).
2. El contenido del diagrama sale **solo** de documentos existentes o de lo que diga el usuario. Si falta información, pregúntala.
3. Escribe `docs/<carpeta de la fase>/diagramas/<nombre>.mmd` (sin colores propios; usa las clases `hl`, `warn`, `err`, `muted` si hace falta resaltar).
4. Renderiza con `$H diagram render docs/<carpeta>/diagramas/<nombre>.mmd` y referencia el SVG desde el markdown con `![Figura. <descripción>](diagramas/<nombre>.svg)`.
5. Revisa visualmente el SVG/PNG generado (léelo con Read) y corrige solapes o etiquetas ilegibles.
