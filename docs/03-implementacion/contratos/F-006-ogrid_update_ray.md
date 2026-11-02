# Contrato F-006 · `ogrid_update_ray`

| Campo | Valor |
|---|---|
| Función | `ogrid_update_ray` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_update_ray.h` |
| Fuente | `src/ogrid/ogrid_update_ray.c` |
| Requisitos | SYS-PR-08, SYS-SAF-07 |
| Apartado de diseño | §6.2 y paso 10 (por cada haz del radar) |
| Estado | Aprobado |

## 1. Propósito

Aplica el efecto de un haz a las celdas que recorre, de la más cercana a la más lejana: las celdas anteriores al eco reciben «paso sin eco», la celda del eco recibe «eco» y las posteriores quedan ocultas (sin cambio). Si el haz no tiene eco, todas reciben «paso sin eco». Esta función solo aplica votos; qué celdas atraviesa cada haz lo calcula la geometría (fuera de este alcance).

## 2. Firma

```c
ogrid_status_t ogrid_update_ray(ogrid_t *grid_io, const uint32_t *cells_in, uint32_t cell_count,
                                bool has_echo, uint32_t echo_pos);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `grid_io` | in,out | `ogrid_t *` | no nulo | — | Grid que se actualiza. |
| `cells_in` | in | `const uint32_t *` | no nulo si `cell_count` > 0; cada valor entre 0 y `OGRID_CELL_COUNT` − 1 | índices de celda | Celdas que recorre el haz, ordenadas de la más cercana a la más lejana, sin repetidos. |
| `cell_count` | in | `uint32_t` | 0 a `OGRID_CELL_COUNT` | celdas | Número de celdas del haz. 0 es válido y no hace nada. |
| `has_echo` | in | `bool` | true o false | — | El haz devolvió eco dentro de esas celdas. |
| `echo_pos` | in | `uint32_t` | si `has_echo`: 0 a `cell_count` − 1; si no, se ignora | posición en `cells_in` | Posición de la celda del eco en `cells_in`. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `grid_io` no es `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar nada |
| PRE-2 | `cells_in` no es `NULL` cuando `cell_count` > 0 | devuelve `OGRID_ERR_PARAM` sin modificar nada |
| PRE-3 | `cell_count` ≤ `OGRID_CELL_COUNT` | devuelve `OGRID_ERR_RANGE` sin modificar nada |
| PRE-4 | Todos los índices de `cells_in[0..cell_count−1]` son < `OGRID_CELL_COUNT` | devuelve `OGRID_ERR_RANGE` sin modificar nada |
| PRE-5 | Si `has_echo`, `echo_pos` < `cell_count` | devuelve `OGRID_ERR_RANGE` sin modificar nada |

Aclaraciones:

- **Orden de comprobación:** primero los parámetros (`grid_io` nulo, o `cells_in` nulo con `cell_count` > 0, `OGRID_ERR_PARAM`), después el rango (`cell_count`, todos los índices de `cells_in` aunque estén detrás del eco, y `echo_pos`, `OGRID_ERR_RANGE`) y por último el estado de las celdas (`OGRID_ERR_STATE`). Si falla más de una, se devuelve el error de la primera.
- **`cell_count` = 0:** con `has_echo` falso devuelve `OGRID_OK` sin hacer nada; con `has_echo` verdadero devuelve `OGRID_ERR_RANGE`, porque no puede haber un eco entre cero celdas.
- **Celdas ocultas:** las celdas detrás del eco no se leen ni se modifican, así que una celda corrupta situada detrás del eco no produce `OGRID_ERR_STATE`.
- **Tras `OGRID_ERR_STATE`:** las celdas anteriores a la corrupta pueden estar ya actualizadas; la celda corrupta y las posteriores no cambian. Es lo único que se garantiza.
- **Celda corrupta:** a efectos de `OGRID_ERR_STATE` es la que define F-004 (PRE-4): log-odds almacenado NaN, infinito o fuera de [−0,8; 3,4]. Un valor de `observed` distinto de 0 y 1 no produce `OGRID_ERR_STATE` en esta función: los votos de eco y de paso lo reescriben con `OGRID_OBSERVED_YES`.
- **Voto de paso y de eco:** ambos dejan `observed` = `OGRID_OBSERVED_YES` (lo hace F-004).

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Sin eco (`has_echo` false): cada celda de `cells_in` recibe el voto «paso sin eco» (−0,40, con límite −0,8). |
| POST-2 | Con eco: las celdas `cells_in[0..echo_pos−1]` reciben «paso sin eco»; la celda `cells_in[echo_pos]` recibe «eco» (+0,85, con límite 3,4); las celdas `cells_in[echo_pos+1..]` no cambian (ocultas). |
| POST-3 | Las celdas que no aparecen en `cells_in` no cambian. |
| POST-4 | Todo log-odds resultante está en [−0,8; 3,4]. |
| POST-5 | Ejemplo: celdas [10, 11, 12, 13], eco en la posición 2, grid recién reiniciado → celda 10 = −0,40, celda 11 = −0,40, celda 12 = +0,85 y `observed`, celda 13 sin cambio ni `observed`. |
| POST-6 | Con `cell_count` = 0 y `has_echo` falso devuelve `OGRID_OK` y el grid no cambia. |
| POST-7 | Atomicidad en errores de parámetro: se validan todos los parámetros e índices antes de modificar nada; si devuelve `OGRID_ERR_PARAM` u `OGRID_ERR_RANGE`, el grid no se modifica. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos | grid actualizado (POST-1 a POST-6) |
| `OGRID_ERR_PARAM` | `grid_io` es `NULL`, o `cells_in` es `NULL` con `cell_count` > 0 | grid sin cambios (POST-7) |
| `OGRID_ERR_RANGE` | `cell_count` > `OGRID_CELL_COUNT`, algún índice fuera de rango o `echo_pos` ≥ `cell_count` | grid sin cambios (POST-7) |
| `OGRID_ERR_STATE` | alguna celda del haz tiene log-odds corrupto (propagado desde F-004) | las celdas anteriores a la corrupta pueden estar ya actualizadas (ver nota de la sección 8) |

## 7. Rendimiento (`perf`)

O(`cell_count`) ≤ 3 520 llamadas a F-004. Bucle con cota estática, sin memoria dinámica. WCET objetivo: decenas de µs por haz en la ECU; se mide en integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: F-004 `ogrid_update_cell`. Estado que modifica: celdas de `*grid_io` listadas en `cells_in`. **Nota:** si una celda del haz está corrupta (`OGRID_ERR_STATE`), las celdas anteriores ya pueden haberse actualizado y la función se detiene en la corrupta; quien llama debe llevar el sistema a estado seguro (C-ERR-02), de modo que no se sigue usando ese grid hasta reiniciarlo con F-001.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
