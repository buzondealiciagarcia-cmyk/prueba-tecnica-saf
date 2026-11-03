# STD-DOC · Estándar de documentación

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Documentos de fase (requisitos, diseño, alternativas, comparativa, implementación, oferta, dosier), documentación del código y diagramas |
| Idioma | Documentos en **español**; código, comentarios y git en **inglés** |
| Exportación | Solo Word (`.docx`) por ahora, con `h export` y la plantilla registrada |

## 1. Principios

| ID | Nivel | Regla |
|---|---|---|
| DOC-GEN-01 | OBL | **No se inventa.** Todo contenido técnico procede del usuario o de documentos cerrados de fases anteriores. Lo que proponga Claude se marca `[PROPUESTA]` hasta que el usuario lo acepte; el cierre de fase falla si queda alguna marca. |
| DOC-GEN-02 | OBL | Lo que falta se marca `[PENDIENTE: pregunta]` y se registra en el log de dudas de la fase. Prohibido rellenar huecos con supuestos silenciosos. |
| DOC-GEN-03 | OBL | El markdown es la fuente; el `.docx` es un derivado que nunca se edita a mano. |
| DOC-GEN-04 | OBL | Cada documento empieza con una tabla de control: título, código, versión, fecha, autor, estado (`Borrador`, `En revisión`, `Cerrado`) y un historial de cambios al final. |
| DOC-GEN-05 | OBL | Trazabilidad: todo apartado de diseño, contrato, función y test cita los IDs de requisito que cubre. `h trace` genera la matriz requisito → diseño → función → test. |
| DOC-GEN-06 | OBL | Frases cortas, voz activa, terminología del glosario (`docs/glosario.md`). Unidades SI con espacio (`50 ms`, `0,2 m`). |

## 2. Estructura de carpetas

```
docs/
  glosario.md
  01-requisitos/   requisitos.md · dudas.md · entradas/ (material original del usuario)
  02-diseno/       diseno.md · revision.md · alternativas/ALT-NN-*.md · comparativa.md · comparativa.json · diagramas/
  03-implementacion/ plan.json · contratos/F-NNN-*.md · implementacion.md · log-tests.md · trazabilidad.md · diagramas/
  04-oferta/       oferta.md · presupuesto.json · presentacion.json · diagramas/
  dosier/          contexto.md · dosier.md (generado)
  export/          *.docx/*.pptx generados (versionados; se actualizan al cerrar cada fase)
```

## 3. Requisitos (resumen; detalle en `requisitos-redaccion.md`)

Formato de la plantilla del usuario (ISO/IEC/IEEE 29148 + EARS): códigos `SYS-<cat>-<nn>` e hipótesis `H-<nn>`, agrupados en las 9 secciones de la plantilla. Cada requisito es un bloque `### <código> — <título corto>` con la tabla `Campo | Valor`: Descripción, Tipo, Origen, Prioridad, Método de verificación, Criterio de aceptación y Estado.

## 4. Diseño

- `diseno.md` describe **lo que el usuario ha definido**: contexto, arquitectura HW, arquitectura SW, comunicaciones, datos, algoritmos, modos y estados, gestión de fallos, rendimiento, trazabilidad a requisitos.
- `revision.md` es el registro de hallazgos de la revisión: `ID | Tipo (Fallo/Hueco/Mejora/Validado) | Apartado | Descripción | Requisitos | Propuesta | Decisión del usuario | Estado`.
- Alternativas: un fichero por alternativa con la cabecera **"Alternativa no seleccionada — no se implementa"**, requisitos que incumple o relaja, hardware/capacidad adicional, viabilidad.
- `comparativa.md`: matrices ponderadas generadas por `h matrix` desde `comparativa.json`; los pesos los decide el usuario.

## 5. Implementación

- `implementacion.md`: alcance implementado, arquitectura del módulo, una ficha por función (contrato resumido, algoritmo, diagrama si aporta), resultados de verificación (resumen de `log-tests.md`, cobertura, MISRA y desviaciones), cómo construir/ejecutar/desplegar.
- Código: Doxygen según C-DOC-01..05. `h doxygen` genera HTML en `build/doxygen/` con la paleta del proyecto.

## 6. Diagramas

Mermaid según UI-DIA-01..04. Tipos recomendados por propósito:

| Propósito | Tipo Mermaid |
|---|---|
| Arquitectura de bloques HW/SW | `flowchart LR` con `subgraph` |
| Interacción entre nodos / mensajes MQTT | `sequenceDiagram` |
| Modos del sistema, estado seguro | `stateDiagram-v2` |
| Flujo del algoritmo por ciclo | `flowchart TB` |
| Estructuras de datos | `classDiagram` |
| Plazos de la oferta | `gantt` |
| Resultados de matrices | `xychart-beta` (barras) |

## 7. Documentos Word

| ID | Nivel | Regla |
|---|---|---|
| DOC-DOCX-01 | OBL | Se generan con `h export <doc>` con dos plantillas derivadas del formato del usuario: `plantilla-requisitos.docx` (requisitos) y `plantilla-documentos.docx` (resto). Estilo común: Arial, texto negro, tablas con cabecera `#D9D9D9`, filas de sección `#F2F2F2` y bordes negros finos. |
| DOC-DOCX-02 | OBL | Documento de requisitos idéntico a la plantilla (título, párrafos de convenciones y tabla; sin portada, índice ni tabla de control). Resto de documentos: portada simple, índice automático, pie con código/versión/página y numeración de figuras y tablas. |
| DOC-DOCX-03 | OBL | Diagramas como PNG a 3× con el tema del harness, ancho máximo el de la caja de texto. |
| DOC-DOCX-04 | OBL | Si la plantilla no cubre un elemento (p.ej. matrices de la comparativa), se generaliza el formato de su tabla de referencia; las dudas de generalización se preguntan al usuario y quedan en `harness.config.json → export.decisions`. |
