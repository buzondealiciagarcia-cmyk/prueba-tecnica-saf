---
description: Fase 3 · Ejecuta los tests y checks de estándares de una función o de toda la implementación
argument-hint: "[F-NNN | --all]"
---
# Ejecutar tests

`H="@@HARNESS@@/bin/h"` · Objetivo: `$ARGUMENTS`

1. Si es `F-NNN`: `$H test F-NNN` (build host-test con ASan/UBSan, test unitario, cobertura 100 % líneas/ramas del fuente, MISRA, clang-tidy, formato, complejidad, Doxygen y símbolos prohibidos).
2. Si es `--all`: debe ejecutarse en una rama de trabajo (`$H git start test/integration`) y `$H test --all`.
3. Muestra el resumen por check y, si falla, el detalle relevante. No modifiques código ni tests desde aquí: si hay que corregir, sigue el ciclo de `/impl-funcion`.
