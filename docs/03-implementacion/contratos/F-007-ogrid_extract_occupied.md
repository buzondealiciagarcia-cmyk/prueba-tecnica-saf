# Contrato F-007 · `ogrid_extract_occupied`

| Campo | Valor |
|---|---|
| Función | `ogrid_extract_occupied` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_extract_occupied.h` |
| Fuente | `src/ogrid/ogrid_extract_occupied.c` |
| Requisitos | SYS-PR-04, SYS-PR-08 |
| Apartado de diseño | §6.4 y paso 11 (extracción de celdas con p > 0,85, H-026) |
| Estado | Aprobado |

## 1. Propósito

Marca en una máscara las celdas que cuentan como ocupadas (p > 0,85). Es la entrada de la agrupación en objetos (SYS-PR-04), que queda fuera de este alcance. Usa la misma regla que el estado de celda, así que ambas salidas nunca se contradicen.

## 2. Firma

```c
ogrid_status_t ogrid_extract_occupied(const ogrid_t *grid, bool *mask_out);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `grid` | in | `const ogrid_t *` | no nulo | — | Grid que se analiza. |
| `mask_out` | out | `bool *` | no nulo; apunta a un array de `OGRID_CELL_COUNT` (3 520) bool | — | Máscara: true en las celdas ocupadas. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `grid` y `mask_out` no son `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar salidas |

Orden de comprobación: PRE-1 y después el estado de cada celda. Si `grid` o `mask_out` es `NULL` se devuelve `OGRID_ERR_PARAM` sin recorrer nada. Con varias celdas corruptas se devuelve un único `OGRID_ERR_STATE` al terminar de recorrer todas. No debe haber ramas que ningún caso de prueba del contrato pueda alcanzar: los índices que se pasan a F-005 son siempre válidos.

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Si devuelve `OGRID_OK`, `mask_out[i]` es true si y solo si `ogrid_get_cell_state(grid, i)` daría `OGRID_CELL_OCCUPIED`, para las 3 520 celdas. |
| POST-2 | Una celda no observada nunca es true, aunque su log-odds fuera alto. |
| POST-3 | Una celda con p = 0,8455 (l = 1,70) es false y una con p = 0,9276 (l = 2,55) es true. |
| POST-4 | Si alguna celda está corrupta según F-005 (`observed` distinto de 0 y 1, o celda observada con log-odds NaN, infinito o fuera de [−0,8; 3,4]), `mask_out[i]` = false para esa celda, el resto de la máscara se calcula igual y devuelve `OGRID_ERR_STATE`. |
| POST-5 | No modifica el grid. Si devuelve `OGRID_ERR_PARAM`, `mask_out` no se modifica. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos y celdas coherentes | máscara completa (POST-1 a POST-3) |
| `OGRID_ERR_PARAM` | `grid` o `mask_out` es `NULL` | sin cambios (POST-5) |
| `OGRID_ERR_STATE` | alguna celda corrupta según F-005 | máscara completa, con false en las celdas corruptas (POST-4) |

## 7. Rendimiento (`perf`)

O(`OGRID_CELL_COUNT`) = 3 520 celdas, bucle con cota estática, una llamada a F-005 por celda. Sin memoria dinámica. WCET objetivo: bajo 1 ms en la ECU (parte del ciclo de 50 ms); se mide en integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: F-005 `ogrid_get_cell_state` (con F-002 por debajo). Estado que modifica: solo `mask_out`.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
