---
name: export-word
description: Exporta cualquier markdown del proyecto a Word con la plantilla registrada (requisitos en el formato de la plantilla, tablas generalizadas, diagramas, portada). Úsala en todos los *-exportar y al registrar la plantilla X.
---
# Exportación a Word

CLI: `H="@@HARNESS@@/bin/h"`

## Uso
- `$H export <requisitos|diseno|comparativa|implementacion|oferta|dosier|ruta.md> [-o salida.docx] [--no-toc]` → `docs/export/<nombre>.docx`.
- Plantillas activas (derivadas de `templates/docx/origen/Requirements_2.pages`, la plantilla del usuario):
  - `plantilla-requisitos.docx` (`export.documents.requisitos`): A4 apaisado, título + párrafos de convenciones + **una tabla** `Code | Description | Type | Source | Priority | Verification | Acceptance criterion | Status` con filas de sección; sin portada, índice ni tabla de control.
  - `plantilla-documentos.docx` (`export.template`): resto de documentos, mismo estilo (Arial, negro, cabecera de tabla `#D9D9D9`, filas de sección `#F2F2F2`, bordes negros finos) + portada simple, índice y pie con código/versión/página.
- Valores pendientes `[PENDIENTE: v]` → `[v]`, `[PENDIENTE]` → `[X]` en el Word (convención de la plantilla).
- El dosier inserta la tabla de requisitos en una sección apaisada (`<!-- h:landscape:begin/end -->` + `⟦H-REQTABLE:<ruta>⟧`).
- `$H template inspect <docx>` describe estilos, cabecera/pie, cuerpo y tablas; `$H template register <docx>` registra la plantilla X; `$H template validate` comprueba el mapa.

## Qué hace el exportador
1. Diagramas: `![…](diagramas/x.svg)` con `x.mmd` hermano y bloques ```mermaid → PNG 3× con el tema del proyecto, ancho ajustado a la caja de texto. Numera "Figura N." automáticamente.
2. Línea `Tabla: <título>` justo antes de una tabla → leyenda "Tabla N. <título>".
3. El `# Título` del markdown pasa a la portada; los `##` son Título 1 en Word.
4. Pandoc con la plantilla como *reference doc* (estilos, márgenes, cabecera y pie de la plantilla).
5. Post-proceso: portada de la plantilla (`cover.mode = template`, copia el cuerpo hasta el primer salto de página y rellena `{{TITULO}} {{SUBTITULO}} {{CODIGO}} {{VERSION}} {{FECHA}} {{AUTOR}} {{CLIENTE}} {{PROYECTO}}`, también en cabecera/pie), índice, requisitos en el formato de la plantilla y resto de tablas con el formato de la tabla de referencia.

## Mapa de plantilla (`config/template-map.json`)
```json
{
  "document": {"cover": "title-only", "toc": false, "control_table": false,
               "header_footer": false, "strip_sections": ["Historial de cambios"]},
  "requirement_table": {"table_index": 0, "mode": "rows", "header_rows": 1, "section_row": 1,
                        "data_row": 2, "columns": {"ID": 0, "DESCRIPCION": 1, "TIPO": 2, "ORIGEN": 3,
                        "PRIORIDAD": 4, "VERIFICACION": 5, "CRITERIO": 6, "ESTADO": 7}},
  "table_reference": {"table_index": 0, "header_rows": 1, "body_rows": [2]},
  "decisions": ["…"]
}
```
- `document.cover`: `template` (copia la portada de la plantilla), `pandoc` (portada simple + salto de página), `title-only` (solo el título, como la plantilla de requisitos) o `none`.
- `requirement_table.mode`: `per-requirement` (una copia de la tabla por requisito, sustituye el bloque `### ID — título` + tabla de campos) o `rows` (una fila por requisito; requiere `columns: {"ID": 0, ...}`).
- `fill`: `placeholders` (la tabla contiene `{{ID}}`, `{{TITULO}}`, `{{DESCRIPCION}}`, `{{TIPO}}`, `{{PRIORIDAD}}`, `{{ORIGEN}}`, `{{VERIFICACION}}`, `{{CRITERIO}}`, `{{ESTADO}}`, `{{SECCION}}`) o `cells` (`fields: {"DESCRIPCION": [1, 1], ...}` con `[fila, columna]`).
- `table_reference`: tabla cuyo formato (propiedades de tabla, sombreado y fuente de cabecera y filas) se aplica a todas las tablas generadas. Si es `null`, se usa el estilo `Table` de la plantilla.

## Generalizar una plantilla que solo trae requisitos
La plantilla X del usuario solo define el formato de requisitos. Para el resto de documentos:
1. Estilos de párrafo y títulos, márgenes, cabecera y pie: siempre los de la plantilla.
2. Tablas genéricas (matrices, presupuesto, trazabilidad): formato de la tabla de requisitos como referencia (`table_reference.table_index` = la misma tabla) salvo que el usuario elija otra cosa.
3. Portada: la de la plantilla si tiene una; si no, la de pandoc con los estilos Title/Subtitle de la plantilla.
4. Cualquier caso dudoso (tablas muy anchas, cabeceras de dos filas, colores no presentes en la plantilla) se pregunta y se anota en `decisions`.

Verifica siempre el resultado (python-docx o PDF vía LibreOffice si existe) antes de entregarlo.
