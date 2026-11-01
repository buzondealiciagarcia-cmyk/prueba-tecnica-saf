---
description: Fase 1 · Cierra la fase de requisitos (solo el usuario) si el checklist automático pasa
disable-model-invocation: true
---
# Cerrar requisitos

`H="@@HARNESS@@/bin/h"`

1. `$H gate check req`. Si falla, muestra el checklist y detente: explica qué falta y qué comando lo resuelve.
2. Si pasa, actualiza `requisitos.md`: versión `1.0`, estado `Cerrado`, fila de historial. Commitea `docs(req): finalize requirements v1.0`.
3. `$H gate close req`: commit de estado, merge `--no-ff` de `docs/requisitos` en `develop` y de `develop` en `main`, tag `req-v1.0`, push.
4. `$H export requisitos` para dejar el Word final y entrega su ruta.
5. Anuncia que la fase de diseño está abierta: `/diseno-iniciar <documentos de diseño>`.
