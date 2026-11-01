---
description: Fase 1 · Nueva ronda de iteración de requisitos (respuestas, cambios y nuevas dudas)
argument-hint: "[respuestas o cambios del usuario]"
---
# Iterar requisitos

`H="@@HARNESS@@/bin/h"` · Aportación del usuario: `$ARGUMENTS`

1. `$H gate require req`; asegúrate de estar en `docs/requisitos` (`$H git start-phase req`).
2. Si el usuario aporta respuestas o cambios, aplícalos **literalmente** a `requisitos.md`; marca las dudas afectadas en `dudas.md` como `Resuelta` (con la respuesta) o `Descartada`. Nunca borres filas de `dudas.md`.
3. Lanza **analista-requisitos** en modo revisión sobre el documento actualizado: nuevas dudas, inconsistencias introducidas, requisitos aún no verificables. Ejecuta `$H req lint` y `$H req questions`.
4. Si quedan dudas abiertas, pregunta la siguiente ronda con `AskUserQuestion` (máx. 4). Si el usuario responde, vuelve al paso 2 dentro de este mismo comando.
5. Cuando un requisito esté completo y el usuario lo confirme, pon su `Estado` en `Aprobado` (pide confirmación explícita por bloque o por lote).
6. Incrementa la versión menor en la tabla de control y en el historial, y commitea `docs(req): iterate requirements vX.Y`.
7. Informa: dudas abiertas, requisitos no aprobados y si `$H gate check req` ya pasa (en ese caso, sugiere `/req-exportar` y `/req-cerrar`).
