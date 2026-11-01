---
description: Fase 3 · Redacta y exporta a Word el documento de implementación con diagramas
---
# Documento de implementación

`H="@@HARNESS@@/bin/h"`

1. `$H gate require impl`; `$H git start-phase impl` (rama `docs/implementacion`, actualizada con `develop`).
2. Lanza **redactor-tecnico** con: `plan.json`, contratos, `src/`, `include/`, `tests/`, `log-tests.md`, `trazabilidad.md`, informes de `.harness/reports/` y la plantilla `implementacion.md`. Redacta: alcance implementado, arquitectura del módulo, una ficha por función (contrato resumido, algoritmo explicado, casos de test, iteraciones y cambios), resultados de verificación, desviaciones MISRA, cómo construir/ejecutar/desplegar y la UI si existe.
3. Diagramas donde la explicación lo necesite (estructura del módulo, flujo por ciclo, secuencia de llamadas) con la skill **diagramas** + `$H diagram render`.
4. Revisa el borrador con el usuario (`AskUserQuestion` para lo que no esté claro) y commitea `docs(impl): write implementation document`.
5. `$H export implementacion` y entrega la ruta.
