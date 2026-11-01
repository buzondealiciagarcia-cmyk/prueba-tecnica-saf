---
description: Reabre una fase cerrada indicando el motivo (bloquea de nuevo las fases posteriores)
argument-hint: "<req|design|impl|offer> <motivo>"
disable-model-invocation: true
---
# Reabrir una fase

`H="@@HARNESS@@/bin/h"` · Argumentos: `$ARGUMENTS`

1. Extrae la fase y el motivo de los argumentos. Si falta alguno, pregúntalo.
2. Explica al usuario las consecuencias (PRO-03): las fases posteriores quedan bloqueadas hasta volver a cerrar esta, y al recerrar se generará un tag `X-v1.N`. Pide confirmación con `AskUserQuestion`.
3. Ejecuta `$H gate reopen <fase> "<motivo>"`; el script cambia a la rama de la fase y commitea el cambio de estado.
4. Indica qué comandos usar para iterar en esa fase.
