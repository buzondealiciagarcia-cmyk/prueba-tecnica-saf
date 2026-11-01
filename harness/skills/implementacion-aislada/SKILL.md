---
name: implementacion-aislada
description: Ciclo de implementación con independencia entre código y tests (contrato, sandboxes, agentes implementador e ingeniero-tests, feedback saneado, disputas, log). Léela entera antes de /impl-funcion.
---
# Implementación aislada (STD-TST §1)

CLI: `H="@@HARNESS@@/bin/h"`

## Roles
| Rol | Ve | No ve | Escribe |
|---|---|---|---|
| Orquestador (tú) | todo | — | contrato, cabeceras, plan, mensajes a agentes; **nunca** código ni tests |
| `implementador` | contrato, `include/`, `src/` previo, requisitos, diseño, estándares, su feedback | `tests/`, cobertura, git | `sandbox/current/impl/out/src/...` |
| `ingeniero-tests` | contrato, `include/`, requisitos, diseño, estándares, `tests/` previos, su feedback | `src/`, cobertura, git | `sandbox/current/test/out/tests/...` |

Garantías: sandboxes con copias (`$H sandbox prepare F-NNN`), hook `guard_paths.py` en el frontmatter de cada agente (y global por `agent_type`), canarios `src/.h-canary` y `tests/.h-canary` que cada agente debe comprobar bloqueados, y solo los scripts mueven entregas al repo (`$H sandbox collect test|impl`). Auditoría en `.harness/audit/<rol>.log`.

## Contrato = única fuente compartida
Todo lo que necesiten saber ambos agentes está en el contrato. Si un agente pregunta (`DUDA:`), la respuesta va al contrato (commit) y se re-prepara el sandbox con `--keep`; nunca respondas en un mensaje a un solo agente información que cambie el comportamiento esperado.

## Mensajes a los agentes
- Implementador: "Sandbox: <ruta>/.harness/sandbox/current/impl. Función F-NNN. Lee ENCARGO.md." (+ en iteraciones: "Lee feedback/feedback-N.md").
- Ingeniero de tests: igual con `/test`. Para huecos de cobertura, describe **cláusulas del contrato** no ejercitadas, nunca líneas ni estructura del código.
- Prohibido copiar en un mensaje contenido del otro sandbox, salida de tests con código, o diffs.

## Bucle
`prepare` → agentes en paralelo → `collect test` + `collect impl` → `$H test F-NNN` → PASS: `$H git finish` · FAIL: feedback al rol correspondiente → `collect` → `$H log change …` → repetir. Máximo 5 iteraciones fallidas (TST-LOG-04), luego decide el usuario.

## Disputas
`DISPUTA: TC-… — motivo` del implementador → compara test y contrato. Test mal → ingeniero-tests lo corrige citando la cláusula. Contrato ambiguo → usuario decide → contrato actualizado → ambos agentes reciben "el contrato ha cambiado, vuelve a leerlo".

## Qué comprueba `h test F-NNN`
build host-test (C-LNG-02) · Unity con ASan/UBSan · cobertura 100 % líneas y ramas del fuente · cppcheck MISRA · clang-tidy · clang-format (fuente, cabecera y test) · lizard (CC ≤ 10, ≤ 60 líneas, ≤ 5 parámetros) · Doxygen (C-DOC-01/02) · símbolos prohibidos y marcadores (C-LIB, C-DOC-05). Informe en `.harness/reports/F-NNN/`, log en `docs/03-implementacion/log-tests.md` (commit automático en la rama).
