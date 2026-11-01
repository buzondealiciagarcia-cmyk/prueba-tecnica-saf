---
description: Registra la plantilla Word X del usuario y acuerda cómo generalizar su formato a todos los documentos
argument-hint: "<ruta a plantilla .docx/.dotx>"
---
# Registrar la plantilla Word

`H="@@HARNESS@@/bin/h"` · Plantilla: `$ARGUMENTS`

Usa la skill **export-word** (formato del mapa y reglas de generalización).

1. Si no hay ruta, pide al usuario que adjunte la plantilla.
2. Ejecuta `$H template register "<ruta>"`. Copia la plantilla a `docs/plantillas/`, apunta `harness.config.json` a ella, crea `config/template-map.json` y muestra la inspección (estilos, cabecera/pie, párrafos, tablas con su contenido y marcadores `{{…}}`).
3. Analiza la inspección y deduce, sin inventar:
   - qué tabla es el formato de requisito y si es **una tabla por requisito** (`per-requirement`) o **una fila por requisito** (`rows`);
   - cómo rellenarla: `placeholders` (si tiene `{{ID}}`, `{{DESCRIPCION}}`…) o `cells` (coordenadas `[fila, columna]` por campo: `ID, TITULO, DESCRIPCION, TIPO, PRIORIDAD, ORIGEN, VERIFICACION, CRITERIO, ESTADO, SECCION`);
   - qué tabla sirve de **referencia de formato** para el resto de tablas (matrices, presupuesto…);
   - si la portada se toma de la plantilla (`cover.mode = template`, hasta el primer salto de página) o la genera pandoc.
4. Pregunta al usuario con `AskUserQuestion` todo lo que no sea evidente: correspondencia de campos, formato de ID de requisito (actualiza `requirements.id_regex` en `harness.config.json`), atributos extra de la plantilla que no existan en STD-REQ, y cómo generalizar a documentos que la plantilla no cubre (diseño, comparativa, oferta). Registra cada decisión en `config/template-map.json → decisions`.
5. Escribe el mapa, valida con `$H template validate` y prueba con `$H export requisitos` (si aún no hay requisitos, crea un ejemplo temporal en `build/` y bórralo después). Revisa el `.docx` resultante y enséñaselo al usuario para validarlo.
6. Commitea en la rama de la fase actual: `docs(req): register word template` (o el ámbito de la fase abierta).
