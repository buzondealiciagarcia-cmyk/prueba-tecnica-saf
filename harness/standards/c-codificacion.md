# STD-C · Estándar de codificación en C

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Todo el código C de producción (`src/`, `include/`) y de test (`tests/`) |
| Base normativa | ISO C11 (ISO/IEC 9899:2011) + MISRA C:2012 (Amendment 2/3) |
| Contexto | Software crítico en seguridad (ferroviario, referencia EN 50128). Simulación en macOS lo más fiel posible al hardware objetivo |
| Verificación | `h test` (automático) + agente `auditor-estandares` (revisión de reglas no automatizables) |

Las reglas tienen identificador (`C-xxx-nn`). Los informes de test, los hallazgos de revisión y las desviaciones citan siempre ese identificador.
Nivel: **OBL** = obligatoria, bloquea la integración · **REC** = recomendada, genera aviso.

---

## 1. Lenguaje y compilación

| ID | Nivel | Regla |
|---|---|---|
| C-LNG-01 | OBL | C11 estricto: `-std=c11 -pedantic`. Prohibidas extensiones GNU/Clang (`typeof`, statement expressions, `__attribute__` fuera de `include/<mod>/<mod>_compiler.h`). |
| C-LNG-02 | OBL | Compila sin avisos con `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wdouble-promotion -Werror` en GCC y Clang. |
| C-LNG-03 | OBL | Cumplimiento MISRA C:2012 verificado con `cppcheck --addon=misra`. Toda desviación se registra en `config/misra-desviaciones.md` (regla, fichero:línea, justificación, riesgo, aprobación) y se suprime en línea con `/* cppcheck-suppress misra-c2012-X.Y ; DEV-NNN */`. Sin registro → no hay desviación. |
| C-LNG-04 | OBL | `clang-tidy` con la configuración del proyecto (`.clang-tidy`) sin hallazgos. |
| C-LNG-05 | OBL | Prohibido comportamiento indefinido o dependiente de implementación no documentado. Los tests corren con `-fsanitize=address,undefined -fno-sanitize-recover=all`. |
| C-LNG-06 | OBL | La simulación en el host usa los mismos tipos, tamaños y endianness que el objetivo: todo supuesto de plataforma se comprueba con `_Static_assert` en `include/<mod>/<mod>_platform.h` (p.ej. `sizeof(float) == 4`, little-endian). |
| C-LNG-07 | REC | El código debe compilar también con el preset de compilación cruzada (`cross-aarch64`) para detectar dependencias del host. |

## 2. Biblioteca estándar y funciones prohibidas

| ID | Nivel | Regla |
|---|---|---|
| C-LIB-01 | OBL | Prohibidos: `malloc`, `calloc`, `realloc`, `free`, `alloca`, VLAs (MISRA Dir 4.12, R18.8). Todo el almacenamiento es estático y dimensionado en compilación. |
| C-LIB-02 | OBL | Prohibidos: `printf`/`scanf` y familia en código de producción, `atoi/atof/atol`, `gets`, `strcpy`, `strcat`, `sprintf`, `setjmp/longjmp`, `signal`, `exit/abort` (salvo en el `main` de la aplicación), `rand`, `time` en el núcleo del algoritmo. |
| C-LIB-03 | OBL | Solo se permiten en el núcleo (`src/<mod>/`): `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, `<string.h>` (`memcpy`, `memset`, `memcmp`), `<math.h>` (funciones con dominio comprobado previamente), `<float.h>`, `<limits.h>`. |
| C-LIB-04 | OBL | La E/S (red, MQTT, ficheros, reloj) vive exclusivamente en la capa de adaptación `src/hal/` o `src/app/`; el núcleo del algoritmo es puro y determinista (misma entrada → misma salida). |

## 3. Tipos

| ID | Nivel | Regla |
|---|---|---|
| C-TYP-01 | OBL | Solo tipos de ancho fijo (`int32_t`, `uint8_t`…) y `bool`. Prohibidos `int`, `long`, `short`, `char` sin calificar (excepto `char` para texto). |
| C-TYP-02 | OBL | Tipos de coma flotante solo a través de typedefs del proyecto (`<mod>_real_t`), definidos en `include/<mod>/<mod>_types.h`. La elección float32/double/punto fijo se decide en la fase de diseño y se fija ahí. |
| C-TYP-03 | OBL | Prohibidas conversiones implícitas que pierdan información (MISRA R10.x). Toda conversión explícita lleva comentario si no es obvia. |
| C-TYP-04 | OBL | Comparaciones de coma flotante nunca con `==`/`!=`; usar `<mod>_real_eq(a, b, eps)` con épsilon documentado. |
| C-TYP-05 | OBL | Toda magnitud física lleva la unidad en el nombre o en el typedef: `range_m`, `azimuth_rad`, `t_ms`, `speed_mps`. |
| C-TYP-06 | OBL | Enumeraciones con valor explícito para el primer elemento y un elemento final `<MOD>_<ENUM>_COUNT` cuando se usen como índice. |

## 4. Nombres

| ID | Nivel | Regla |
|---|---|---|
| C-NAM-01 | OBL | `snake_case` para funciones, variables y tipos; `UPPER_SNAKE_CASE` para macros y constantes de enumeración. |
| C-NAM-02 | OBL | Todo símbolo con enlace externo lleva el prefijo del módulo: `ogrid_update()`, `ogrid_cell_t`, `OGRID_MAX_CELLS`. |
| C-NAM-03 | OBL | Sufijos de tipo: `_t` (typedef), `_e` no se usa (el enum también es `_t`). |
| C-NAM-04 | OBL | Prefijos de ámbito: `s_` variable `static` de fichero, `g_` variable global (requiere justificación en diseño), `k_` constante `static const`. Parámetros de salida terminan en `_out`, de entrada/salida en `_io`. |
| C-NAM-05 | OBL | Nombres descriptivos en inglés; abreviaturas solo del glosario `docs/glosario.md` (p.ej. `ogrid`, `pcl` = point cloud, `lo` = log-odds). Variables de una letra solo como índices de bucle (`i`, `j`, `k`). |
| C-NAM-06 | OBL | Ficheros: `<mod>_<func>.c` (una función pública por fichero + sus `static`), cabecera pública `include/<mod>/<mod>.h`, interna `src/<mod>/<mod>_internal.h`. |

## 5. Funciones

| ID | Nivel | Regla |
|---|---|---|
| C-FUN-01 | OBL | Complejidad ciclomática ≤ 10, cuerpo ≤ 60 líneas lógicas, ≤ 5 parámetros (verificado con `lizard`). |
| C-FUN-02 | OBL | Toda función pública devuelve `<mod>_status_t` y entrega resultados por punteros `_out`. Excepción: funciones puras de consulta sin modo de fallo (`bool`, getters), justificadas en el contrato. |
| C-FUN-03 | OBL | Un único punto de retorno (MISRA R15.5) salvo cláusulas de guarda al inicio, que sí se permiten y se registran como desviación de proyecto DEV-001. |
| C-FUN-04 | OBL | Toda función pública valida sus parámetros (punteros no nulos, rangos del contrato) antes de usarlos y devuelve `<MOD>_ERR_PARAM` sin efectos laterales si fallan. |
| C-FUN-05 | OBL | El valor de retorno de toda función no `void` se usa o se descarta explícitamente con `(void)` (MISRA R17.7). |
| C-FUN-06 | OBL | Prohibida la recursión (MISRA R17.2). Bucles con cota superior estática demostrable. |
| C-FUN-07 | OBL | Parámetros puntero que no se modifican son `const T *`. |
| C-FUN-08 | OBL | Sin estado oculto: el estado del módulo se pasa explícitamente (`ogrid_t *grid`). Variables `static` mutables de fichero prohibidas salvo en la capa `app/`. |
| C-FUN-09 | REC | Funciones del bucle de 50 ms con WCET estimado en el contrato; prohibido trabajo no acotado dentro del ciclo. |

## 6. Gestión de errores

| ID | Nivel | Regla |
|---|---|---|
| C-ERR-01 | OBL | Un enum de estado por módulo: `typedef enum { OGRID_OK = 0, OGRID_ERR_PARAM, OGRID_ERR_RANGE, OGRID_ERR_STATE, … } ogrid_status_t;` |
| C-ERR-02 | OBL | Los errores no se ignoran ni se "tragan": se propagan o se tratan llevando el sistema a estado seguro (ver STD-SAF). |
| C-ERR-03 | OBL | `assert()` prohibido en producción; los invariantes internos se comprueban con la macro `<MOD>_INVARIANT(cond)` que en el build de test aborta y en producción registra el fallo y fuerza estado seguro. |

## 7. Preprocesador y cabeceras

| ID | Nivel | Regla |
|---|---|---|
| C-PRE-01 | OBL | Guardas `<MOD>_<FICHERO>_H` (p.ej. `OGRID_OGRID_TYPES_H` para `include/ogrid/ogrid_types.h`) con `#ifndef`/`#define`/`#endif /* … */` (no `#pragma once`). |
| C-PRE-02 | OBL | Macros solo para constantes y compilación condicional. Prohibidas macros función (usar `static inline`), salvo `<MOD>_INVARIANT` y `UNUSED`. |
| C-PRE-03 | OBL | Constantes numéricas con sufijo y tipo: `(10U)`, `(0.5F)`. Nada de números mágicos: todo literal distinto de 0/1 es una constante con nombre o un parámetro de configuración (STD-CFG). |
| C-PRE-04 | OBL | Orden de includes: cabecera propia, cabeceras del proyecto, estándar. Cada cabecera es autosuficiente. |

## 8. Formato (automático)

| ID | Nivel | Regla |
|---|---|---|
| C-FMT-01 | OBL | `clang-format` con `.clang-format` del proyecto: estilo Allman, 4 espacios, 100 columnas, punteros a la derecha (`uint8_t *p`). `h test` falla si `clang-format --dry-run --Werror` reporta diferencias. |
| C-FMT-02 | OBL | Llaves obligatorias en todo `if/else/for/while/do` aunque sea una sentencia (MISRA R15.6). |
| C-FMT-03 | OBL | Un `switch` siempre con `default` y `break` en cada caso (MISRA R16.x). |
| C-FMT-04 | OBL | Fin de línea LF, UTF-8 sin BOM, sin espacios finales, salto de línea final. |

## 9. Comentarios y documentación del código

| ID | Nivel | Regla |
|---|---|---|
| C-DOC-01 | OBL | Cabecera Doxygen de fichero: `@file`, `@brief`, `@details` (rol en el sistema), `@ingroup <mod>`. |
| C-DOC-02 | OBL | Toda función pública documentada en la cabecera con: `@brief`, `@param[in/out/in,out]` con rango y unidad, `@return` con cada código posible, `@pre`, `@post`, `@req` (IDs de requisito), `@contract` (ID del contrato `F-NNN`). |
| C-DOC-03 | OBL | Funciones `static`: `@brief` y `@param` como mínimo. |
| C-DOC-04 | OBL | Comentarios en inglés, explican el *porqué*, no el *qué*. Prohibido código comentado. |
| C-DOC-05 | OBL | Marcadores `TODO`/`FIXME`/`XXX` prohibidos en ramas que se integran en `develop`. |

## 10. Concurrencia y tiempo

| ID | Nivel | Regla |
|---|---|---|
| C-CON-01 | OBL | El núcleo es monohilo y no bloqueante. La concurrencia (hilo MQTT, hilo de proceso) está solo en `src/app/` y se comunica por colas de tamaño fijo. |
| C-CON-02 | OBL | El tiempo se recibe como parámetro (timestamp de la nube de puntos), nunca se lee del reloj dentro del núcleo. |

## 11. Estructura del proyecto C

```
include/<mod>/   cabeceras públicas (escritas por el orquestador a partir del contrato)
src/<mod>/       una función pública por fichero
src/hal/         adaptación a plataforma (red, reloj, ficheros)
src/app/         main, hilos, MQTT, logging
tests/unit/      test_<mod>_<func>.c (Unity)
tests/integration/
config/          parámetros calibrables, desviaciones MISRA
third_party/     Unity, CMock (versionados y fijados)
```
