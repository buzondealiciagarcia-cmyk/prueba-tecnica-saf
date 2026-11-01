---
description: Fase 4 · Cierra la oferta (solo el usuario) si todas las secciones están aprobadas
disable-model-invocation: true
---
# Cerrar la oferta

`H="@@HARNESS@@/bin/h"`

1. `$H gate check offer` (secciones aprobadas, sin marcadores, presentación generada). Si falla, explica qué falta y detente.
2. Versión `1.0` / `Cerrado` en `oferta.md`; commitea.
3. `$H gate close offer` (merges, tag `offer-v1.0`, push).
4. `$H export oferta` y `$H pptx build`; entrega ambas rutas.
