---
description: Fase 4 · Exporta la oferta a Word con el mismo estilo que el resto de documentos
---
# Exportar la oferta

`H="@@HARNESS@@/bin/h"`

1. `$H offer budget` (si existe `presupuesto.json`) y `$H diagram render docs/04-oferta`.
2. `$H export oferta` → `docs/export/oferta.docx`. Revisa portada, tablas de presupuesto y Gantt.
3. Entrega la ruta. (PDF desactivado por decisión del usuario; solo Word.)
