# Contrato F-005 · `ogrid_get_cell_state`

| Campo | Valor |
|---|---|
| Función | `ogrid_get_cell_state` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_get_cell_state.h` |
| Fuente | `src/ogrid/ogrid_get_cell_state.c` |
| Requisitos | SYS-PR-08, SYS-SAF-07 |
| Apartado de diseño | §6.2 (estados de celda derivados, H-012) |
| Estado | Aprobado |

## 1. Propósito

Deriva el estado de una celda a partir de su log-odds: ocupado, probabilidad baja de obstáculo o zona no estudiada. No existe el estado «libre»: ausencia de eco no demuestra que no haya obstáculo.

## 2. Firma

```c
ogrid_status_t ogrid_get_cell_state(const ogrid_t *grid, uint32_t cell_idx, ogrid_cell_state_t *state_out);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `grid` | in | `const ogrid_t *` | no nulo | — | Grid que se consulta. |
| `cell_idx` | in | `uint32_t` | 0 a `OGRID_CELL_COUNT` − 1 | índice de celda | Celda consultada. |
| `state_out` | out | `ogrid_cell_state_t *` | no nulo | — | Estado derivado de la celda. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `grid` y `state_out` no son `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar salidas |
| PRE-2 | `cell_idx` < `OGRID_CELL_COUNT` | devuelve `OGRID_ERR_RANGE` sin modificar salidas |

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Si `observed` es `OGRID_OBSERVED_NO`, `*state_out` = `OGRID_CELL_UNSURVEYED` (zona no estudiada), con cualquier valor de log-odds. |
| POST-2 | Si `observed` es `OGRID_OBSERVED_YES` y p > `OGRID_OCCUPIED_PROB_THRESHOLD` (0,85), `*state_out` = `OGRID_CELL_OCCUPIED`; p se calcula con F-002. Con tolerancia: l = 1,70 (p = 0,8455) no es ocupado y l = 2,55 (p = 0,9276) sí. |
| POST-3 | Si `observed` es `OGRID_OBSERVED_YES` y p ≤ 0,85, `*state_out` = `OGRID_CELL_LOW_PROB` (probabilidad baja de obstáculo). |
| POST-4 | La salida nunca es un valor distinto de los tres estados de `ogrid_cell_state_t`: no existe «libre» (SYS-SAF-07). |
| POST-5 | Si `observed` no es ni `OGRID_OBSERVED_NO` ni `OGRID_OBSERVED_YES` (celda corrupta), o si el log-odds almacenado de una celda observada es NaN, infinito o está fuera de [−0,8; 3,4], devuelve `OGRID_ERR_STATE` y `*state_out` = `OGRID_CELL_UNSURVEYED` (decisión de seguridad: nunca «libre» ni «probabilidad baja» ante datos corruptos). |
| POST-6 | Si devuelve `OGRID_ERR_PARAM` u `OGRID_ERR_RANGE`, `*state_out` no se modifica. No modifica el grid. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos y celda coherente | `*state_out` escrito (POST-1 a POST-4) |
| `OGRID_ERR_PARAM` | `grid` o `state_out` es `NULL` | sin cambios (POST-6) |
| `OGRID_ERR_RANGE` | `cell_idx` ≥ `OGRID_CELL_COUNT` | sin cambios (POST-6) |
| `OGRID_ERR_STATE` | `observed` con un valor distinto de 0 y 1, o celda observada con log-odds corrupto | `*state_out` = `OGRID_CELL_UNSURVEYED` (POST-5) |

## 7. Rendimiento (`perf`)

Tiempo constante: una llamada a F-002 y una comparación. Sin bucles. WCET objetivo ≪ 1 µs; se mide en integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: F-002 `ogrid_log_odds_to_prob`. Estado que modifica: solo `*state_out`.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
