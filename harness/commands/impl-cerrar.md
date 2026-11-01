---
description: Fase 3 · Cierra la fase de implementación (solo el usuario) si el checklist automático pasa
disable-model-invocation: true
---
# Cerrar implementación

`H="@@HARNESS@@/bin/h"`

1. `$H trace` y `$H diagram render`.
2. `$H gate check impl` (funciones integradas, integración PASS contenida en `main`, documento sin marcadores, trazabilidad SAF-05). Si falla, muestra el checklist y detente.
3. Actualiza `implementacion.md` a versión `1.0` / `Cerrado` y commitea en `docs/implementacion`.
4. `$H gate close impl` (merges, tag `impl-v1.0`, push).
5. `$H export implementacion`; entrega la ruta.
6. Anuncia que la fase de oferta está abierta (`/oferta-iniciar`) y que ya se puede generar el dosier (`/dosier-generar`).
