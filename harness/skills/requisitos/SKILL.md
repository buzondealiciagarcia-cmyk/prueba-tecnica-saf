---
name: requisitos
description: Formato de la plantilla de requisitos del usuario (ISO 29148 + EARS, códigos SYS/H, 9 secciones), calidad e iteración con registro de dudas. Úsala en /req-iniciar, /req-iterar y siempre que se lean o escriban requisitos.
---
# Requisitos

CLI: `H="@@HARNESS@@/bin/h"` · Estándar: `@@HARNESS@@/standards/requisitos-redaccion.md`

## Formato (plantilla del usuario `Requirements_2`)
`requisitos.md` = título, tabla de control, los 3 párrafos de convenciones de la plantilla y las 9 secciones `##` **con estos nombres exactos**: Normativa, Entradas, Procesamiento, Detección y prestaciones, Salida, Tiempo, Seguridad y modos de funcionamiento, Implementación, Hipótesis. Dentro, un bloque por requisito:
```
### SYS-T-01 — Latencia de publicación
| Campo | Valor |
|---|---|
| Descripción | El sistema deberá publicar el grid en menos de [PENDIENTE: 50 ms] desde la recepción de la nube. |
| Tipo | No funcional |
| Origen | R2 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Latencia ≤ 50 ms en el percentil 99 con 512 ecos |
| Estado | Borrador |
```
- Códigos: `SYS-<GEN|IN|PR|DET|OUT|T|SAF|IMP>-<nn>` según la sección; hipótesis `H-<nn>` (Tipo `Hipótesis`, sin «deberá»; prioridad, verificación y criterio opcionales).
- EARS: «El sistema deberá…» (siempre activo) o «Si…, entonces el sistema deberá…» (respuesta ante fallos).
- Tipo: Funcional · No funcional · Restricción · Hipótesis. Origen (Source): `R1`–`R4` o `Derivado` (`Derivado de <código>`).
- Valores pendientes: `[PENDIENTE: valor]` / `[PENDIENTE]` → en Word `[valor]` / `[X]`. Bloquean el cierre.
- Word: una sola tabla `Code | Description | Type | Source | Priority | Verification | Acceptance criterion | Status` con filas de sección, A4 apaisado (`$H export requisitos`).
- Todo esto vive en `harness.config.json → requirements` y en `templates/docx/plantilla-requisitos.*`.

## Comandos útiles
`$H req lint` (avisos de calidad) · `$H req lint --strict` (lo que exige el cierre) · `$H req questions` (dudas abiertas) · `$H req list` (JSON).

## Iteración
- Dudas en `dudas.md`: `| ID | Requisito | Pregunta | Respuesta | Estado |`, IDs `Q-NNN`, estados `Abierta/Resuelta/Descartada`. Nunca se borran.
- Preguntas al usuario por rondas (`AskUserQuestion`, ≤ 4, opción recomendada primero). Las respuestas se aplican literalmente.
- Cada ronda: versión menor +1 en tabla de control e historial; commit `docs(req): …` en `docs/requisitos`.
- `Estado = Aprobado` solo con confirmación explícita del usuario.
