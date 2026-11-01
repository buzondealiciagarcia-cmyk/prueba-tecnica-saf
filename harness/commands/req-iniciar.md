---
description: Fase 1 · Arranca la especificación de requisitos a partir del material que aporta el usuario
argument-hint: "<fichero(s) o texto con los requisitos>"
---
# Iniciar requisitos

`H="@@HARNESS@@/bin/h"` · Entrada del usuario: `$ARGUMENTS`

Skill: **requisitos**. Estándares: `@@HARNESS@@/standards/requisitos-redaccion.md`, `documentacion.md`.

1. `$H gate require req` y `$H git start-phase req` (rama `docs/requisitos`).
2. Guarda el material original **sin modificar** en `docs/01-requisitos/entradas/` (copia los ficheros; si es texto, `entrada-<fecha>.md`). Es la fuente de verdad para el campo *Origen*.
3. Lanza el agente **analista-requisitos** con: rutas de las entradas, `docs/01-requisitos/requisitos.md`, `dudas.md`, el estándar STD-REQ y `harness.config.json → requirements`. Su encargo: normalizar cada necesidad en un bloque de requisito, clasificar, detectar ambigüedades, términos vagos, requisitos compuestos, contradicciones y huecos (rendimiento, seguridad funcional, interfaces, entorno, fallos), y registrar cada duda en `dudas.md` como `Abierta`. No inventa valores: lo que falte queda `[PENDIENTE: …]`.
4. Ejecuta `$H req lint` y revisa el resultado junto al informe del agente.
5. Presenta al usuario un resumen corto (nº de requisitos por tipo, nº de dudas) y haz **la primera ronda de preguntas** con `AskUserQuestion` (las 4 dudas más importantes; opción recomendada primero). Aplica las respuestas como en `/req-iterar`.
6. Actualiza la tabla de control (versión 0.1) y commitea: `docs(req): draft requirements from user input` con `Refs:` de los IDs creados.
7. Indica cómo seguir: `/req-iterar` para nuevas rondas, `/req-exportar` para ver el Word, `/req-cerrar` cuando no queden dudas.
