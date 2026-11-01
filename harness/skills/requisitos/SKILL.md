---
name: requisitos
description: Formato, calidad e iteración de requisitos (STD-REQ) y su registro de dudas. Úsala en /req-iniciar, /req-iterar y siempre que se lean o escriban requisitos.
---
# Requisitos

CLI: `H="@@HARNESS@@/bin/h"` · Estándar: `@@HARNESS@@/standards/requisitos-redaccion.md`

## Formato de bloque (lo que parsea el harness)
```
### REQ-FUN-001 — Título corto
| Campo | Valor |
|---|---|
| Descripción | El sistema deberá … |
| Tipo | Funcional |
| Prioridad | Must |
| Origen | entradas/enunciado.md §2 |
| Método de verificación | Test |
| Criterio de aceptación | ≤ 100 ms en el percentil 99 con 512 ecos |
| Estado | Borrador |
```
- El formato del ID lo define `harness.config.json → requirements.id_regex` (se ajusta a la plantilla X al registrarla). Valores permitidos de Tipo/Prioridad/Verificación/Estado también ahí.
- Los `##` agrupan por tipo; el exportador mantiene la sección (`{{SECCION}}`).

## Comandos útiles
`$H req lint` (avisos de calidad) · `$H req lint --strict` (lo que exige el cierre) · `$H req questions` (dudas abiertas) · `$H req list` (JSON).

## Iteración
- Dudas en `dudas.md`: `| ID | Requisito | Pregunta | Respuesta | Estado |`, IDs `Q-NNN`, estados `Abierta/Resuelta/Descartada`. Nunca se borran.
- Preguntas al usuario por rondas (`AskUserQuestion`, ≤ 4, opción recomendada primero). Las respuestas se aplican literalmente.
- Cada ronda: versión menor +1 en tabla de control e historial; commit `docs(req): …` en `docs/requisitos`.
- `Estado = Aprobado` solo con confirmación explícita del usuario.
