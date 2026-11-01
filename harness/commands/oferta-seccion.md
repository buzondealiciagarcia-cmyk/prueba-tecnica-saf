---
description: Fase 4 · Itera con el usuario una sección de la oferta hasta aprobarla
argument-hint: "<objeto|introduccion|datos|objetivos|alcance|requisitos|descripcion|presupuesto|plazo>"
---
# Sección de la oferta

`H="@@HARNESS@@/bin/h"` · Sección: `$ARGUMENTS`

Skill: **oferta** (qué contiene cada sección y de dónde sale).

1. `$H gate require offer`; rama `docs/oferta`. `$H offer status`.
2. Prepara un borrador de la sección **solo** con información de las fases 1-3 y respuestas previas del usuario. Lo que no se conozca va como `[PENDIENTE: …]`.
3. Entrevista con `AskUserQuestion` para cerrar el contenido exacto: enfoque, nivel de detalle, qué incluir y excluir. Para `presupuesto` y `plazo`, pide partidas, horas, tarifas, otros costes, contingencia, impuestos, fecha de inicio, tareas, dependencias e hitos y escríbelos en `docs/04-oferta/presupuesto.json`; después `$H offer budget` (tabla y Gantt generados). **Nunca** propongas cifras por tu cuenta salvo que el usuario lo pida expresamente, y entonces márcalas `[PROPUESTA]`.
4. Escribe la sección en `oferta.md` debajo de su marcador y `$H offer section <sección> "en revision"`. Muestra el texto final al usuario.
5. Cuando el usuario la apruebe explícitamente: `$H offer section <sección> aprobado`. Commitea `docs(offer): <sección> section`.
6. Indica las secciones pendientes.
