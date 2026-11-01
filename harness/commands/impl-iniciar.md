---
description: Fase 3 · Define el alcance a implementar y descompone el diseño en funciones con contrato
argument-hint: "<parte del sistema a implementar>"
---
# Iniciar implementación

`H="@@HARNESS@@/bin/h"` · Alcance indicado por el usuario: `$ARGUMENTS`

Skills: **implementacion-aislada**, **git-flujo**. Estándares: STD-C, STD-TST, STD-GIT.

1. `$H gate require impl`, `$H git start-phase impl`, `$H scaffold impl`, `$H scaffold c-project` (CMake, presets, `.clang-format`, `.clang-tidy`, configuración de cppcheck, Unity y CMock en `third_party/`).
2. Si `$ARGUMENTS` está vacío o es ambiguo, pregunta qué parte del diseño se implementa. Localiza en `diseno.md` los apartados y requisitos afectados y confírmalos con el usuario.
3. Resuelve con el usuario las decisiones aplazadas al diseño que afecten al código si no estuvieran cerradas (representación numérica del grid, límites de compilación…). Si faltan en el diseño, es un hueco de diseño: propón `/fase-reabrir design`.
4. Propón la descomposición en funciones (una función pública por fichero, C-NAM-06), en **orden de dependencias** y con: `id` (`F-NNN`), `nombre` (`<mod>_<verbo>_<objeto>`), requisitos, `depende_de`, rutas de contrato, cabecera, fuente y test. Las funciones van de las más básicas (tipos, validación, inicialización) a las compuestas (actualización por ciclo). El usuario aprueba o modifica la lista.
5. Escribe `docs/03-implementacion/plan.json` (estado `pendiente`) y valida con `$H impl validate`.
6. Crea las cabeceras base del módulo (`include/<mod>/<mod>.h` con el enum de estado, `<mod>_types.h`, `<mod>_limits.h`) según el diseño, con Doxygen completo; nada de lógica.
7. Commitea `docs(impl): add implementation plan` + `build(<mod>): scaffold module headers` e integra con `$H git finish` (la rama `docs/implementacion` se reutilizará para el documento de implementación).
8. Valora con el usuario si un front-end ayudaría a entender o interactuar con la parte implementada (`/impl-frontend`).
9. Siguiente paso: `/impl-funcion` (primera función del plan).
