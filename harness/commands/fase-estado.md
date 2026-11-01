---
description: Muestra el estado de las fases, la rama actual y qué falta para cerrar la fase abierta
---
# Estado del proyecto

`H="@@HARNESS@@/bin/h"`

1. Ejecuta `$H gate status` y `git branch --show-current`.
2. Para la fase abierta, ejecuta `$H gate check <fase>` (fases: `req`, `design`, `impl`, `offer`).
3. Si la fase abierta es `impl`, añade `$H impl status`; si es `offer`, `$H offer status`.
4. Resume en pocas líneas: fase abierta, qué bloquea su cierre (checklist en rojo) y el siguiente comando recomendado. No cierres nada.
