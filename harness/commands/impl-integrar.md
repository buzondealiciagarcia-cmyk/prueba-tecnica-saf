---
description: Fase 3 · Verifica la implementación completa (tests de integración + estándares) y la integra en main
---
# Integración completa

`H="@@HARNESS@@/bin/h"`

1. `$H gate require impl`. `$H impl status`: todas las funciones deben estar `integrada`.
2. `$H git start test/integration`.
3. Tests de integración (caja negra, desde requisitos): `$H sandbox prepare <última F-NNN>` y lanza **ingeniero-tests** pidiéndole `out/tests/integration/test_integration_<ámbito>.c` que verifique los requisitos del alcance a través de la API pública (sin ver `src/`). Recoge con `$H sandbox collect test` (en una rama `test/*` solo se copian `tests/integration/` y `tests/data/`; commit `test(<mod>): add integration tests`).
4. `$H test --all`. Si falla un test de integración, analiza con el usuario si es un defecto de implementación (abre `fix/<REQ>-<desc>` y repite el ciclo de `/impl-funcion` sobre la función afectada) o de test.
5. Con PASS: `$H trace` (matriz de trazabilidad; no puede haber requisitos del alcance con verificación *Test* sin casos) y commitea `docs(impl): update traceability`.
6. `$H git finish` (test/integration → develop) y `$H git release --tag vX.Y.Z --message "<resumen>"` (develop → main). Pregunta al usuario el número de versión SemVer (recomendado `v1.0.0` la primera vez).
7. Siguiente: `/impl-exportar` para el documento de implementación y `/impl-cerrar`.
