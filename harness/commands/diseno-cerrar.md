---
description: Fase 2 · Cierra la fase de diseño (solo el usuario) si el checklist automático pasa
disable-model-invocation: true
---
# Cerrar diseño

`H="@@HARNESS@@/bin/h"`

1. `$H diagram render` y, si hay alternativas, `$H matrix render`.
2. `$H gate check design`. Si falla, muestra el checklist, explica qué falta y detente.
3. Si pasa, actualiza versión `1.0` y estado `Cerrado` en `diseno.md` y `comparativa.md`, historial incluido. Commitea `docs(design): finalize design v1.0`.
4. `$H gate close design` (merges `--no-ff`, tag `design-v1.0`, push).
5. `$H export diseno` y `$H export comparativa`; entrega las rutas.
6. Anuncia que la fase de implementación está abierta: `/impl-iniciar <parte del sistema a implementar>`.
