---
description: Fase 2 · Construye la comparativa con matrices ponderadas entre el diseño y sus alternativas
---
# Comparativa ponderada

`H="@@HARNESS@@/bin/h"`

Skill: **matriz-ponderada**.

1. `$H gate require design`; rama `docs/diseno`. Debe existir al menos una alternativa.
2. Propón un conjunto de criterios a partir de los requisitos y estándares (p.ej. cumplimiento de requisitos de seguridad, coste HW, capacidad de cómputo, latencia, robustez de detección, complejidad SW, mantenibilidad) marcados como propuesta. **El usuario decide** criterios, descripción y pesos (deben sumar 100) con `AskUserQuestion`.
3. Para cada alternativa y criterio, propone una puntuación en la escala acordada **con justificación** basada en los documentos; el usuario confirma o cambia cada valor (por bloques de criterio).
4. Rellena la matriz de cumplimiento de requisitos (`si/parcial/no/na`) a partir de los documentos de cada alternativa.
5. Escribe `docs/02-diseno/comparativa.json` y ejecuta `$H matrix render` (genera tablas, totales, clasificación, sensibilidad ±20 % y gráfico).
6. Redacta en `comparativa.md` los apartados 2 (alternativas), 5 (conclusiones) con las palabras del usuario. Si la sensibilidad muestra que la ganadora cambia, coméntalo con el usuario.
7. Commitea `docs(design): add weighted comparison` y sugiere `/diseno-exportar comparativa`.
