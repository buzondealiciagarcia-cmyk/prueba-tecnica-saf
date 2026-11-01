---
description: Fase 3 · Implementa y verifica la siguiente función del plan con agentes aislados (código y test por separado)
argument-hint: "[F-NNN]"
---
# Implementar una función

`H="@@HARNESS@@/bin/h"` · Función: `$ARGUMENTS` (por defecto, la siguiente del plan)

Skill obligatoria: **implementacion-aislada** (léela entera antes de empezar). Tú eres el **orquestador**: no escribes código ni tests.

## 0. Preparación
1. `$H gate require impl`. `$H impl next` debe devolver la función pedida: las anteriores tienen que estar `integrada` (GIT-BR-05). Si no, detente y explica.
2. `$H git start-feature F-NNN` (rama `feature/<REQ>-<func>` desde `develop`).

## 1. Contrato (lo aprueba el usuario)
3. Redacta `docs/03-implementacion/contratos/F-NNN-<func>.md` desde `@@HARNESS@@/templates/docs/contrato.md`, usando solo diseño y requisitos cerrados: firma, parámetros con rango y unidad, precondiciones, postcondiciones (con tolerancias), errores, rendimiento y efectos laterales.
4. Añade el prototipo con Doxygen completo (C-DOC-02: `@brief @param @return @pre @post @req @contract`) en la cabecera indicada en el plan. Si hacen falta declaraciones internas compartidas, en `src/<mod>/<mod>_internal.h`.
5. Muestra el contrato y pide aprobación con `AskUserQuestion`. Cualquier ambigüedad se pregunta aquí: después los agentes no podrán preguntarte nada que no esté en el contrato.
6. Commitea `docs(impl): add contract for F-NNN` y `$H impl set F-NNN contrato`.

## 2. Encargos aislados (en paralelo)
7. `$H sandbox prepare F-NNN`.
8. Lanza a la vez:
   - **ingeniero-tests** con: `Sandbox: <ruta>/.harness/sandbox/current/test` y el ID de la función.
   - **implementador** con: `Sandbox: <ruta>/.harness/sandbox/current/impl` y el ID de la función.
   No incluyas en los mensajes nada del otro agente. Si alguno informa `ISOLATION_BROKEN`, detén todo y avisa al usuario.
9. `$H sandbox collect test` y `$H sandbox collect impl` (commits `test(...)` y `feat(...)` separados).

## 3. Verificación e iteración
10. `$H test F-NNN`. Genera informe, feedback saneado en cada sandbox y entrada en `log-tests.md` (commit automático).
11. Si **PASS** → paso 4. Si **FAIL**, clasifica y actúa:
    - Casos fallados, sanitizers, estándares o cobertura del fuente → reanuda al **implementador** (SendMessage o nuevo encargo con `$H sandbox prepare F-NNN --keep`) indicándole `feedback/feedback-N.md`.
    - Test que no compila o con formato incorrecto → reanuda al **ingeniero-tests** con su `feedback/feedback-N.md`.
    - Cobertura < 100 % sin fallos → pregunta al implementador si hay código inalcanzable (debe justificarlo o eliminarlo) y pide al ingeniero de tests que cubra las cláusulas del contrato que tú identifiques como no ejercitadas **describiéndolas con el contrato**, nunca con líneas de código.
    - **DISPUTA** del implementador sobre un caso → compara el test con el contrato (tú sí puedes leer ambos). Si el test es incorrecto, pide al ingeniero de tests que lo corrija citando la cláusula; si el contrato es ambiguo, pregunta al usuario y actualiza el contrato (commit) antes de seguir.
12. Tras cada corrección: `$H sandbox collect <rol> -m "<qué cambió>"`, `$H log change F-NNN --agent <rol> --cause "<causa raíz>" --desc "<cambio aplicado>"` y vuelve al paso 10.
13. Si se alcanzan 5 iteraciones fallidas (TST-LOG-04), detente y presenta al usuario el historial de `log-tests.md` para decidir.

## 4. Integración de la función
14. Opcional pero recomendado: agente **auditor-estandares** sobre el fuente (reglas no automatizables). Hallazgos → implementador, y repetir paso 10.
15. `$H git finish` (exige el último `h test` en PASS sobre HEAD, marca la función `integrada`, merge `--no-ff` en `develop`, push) y `$H sandbox clean`.
16. Resume: casos, iteraciones, cambios y siguiente función (`/impl-funcion`). Cuando no queden: `/impl-integrar`.
