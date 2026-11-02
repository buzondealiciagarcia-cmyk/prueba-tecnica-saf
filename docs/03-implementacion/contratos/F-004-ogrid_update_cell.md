# Contrato F-004 · `ogrid_update_cell`

| Campo | Valor |
|---|---|
| Función | `ogrid_update_cell` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_update_cell.h` |
| Fuente | `src/ogrid/ogrid_update_cell.c` |
| Requisitos | SYS-PR-08, SYS-SAF-07 |
| Apartado de diseño | §6.2 (modelo inverso del sensor, regla general H-024/H-031) y paso 10 |
| Estado | Aprobado |

## 1. Propósito

Aplica a una celda el voto de un haz del radar: eco suma +0,85, onda que pasa sin eco suma −0,40, celda oculta detrás de un eco no cambia. El resultado se limita a [−0,8; 3,4]. Es la regla que hace que un eco aislado no confirme un obstáculo y que tres seguidos sí.

## 2. Firma

```c
ogrid_status_t ogrid_update_cell(ogrid_t *grid_io, uint32_t cell_idx, ogrid_vote_kind_t vote);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `grid_io` | in,out | `ogrid_t *` | no nulo | — | Grid cuya celda se actualiza. |
| `cell_idx` | in | `uint32_t` | 0 a `OGRID_CELL_COUNT` − 1 (3 519); índice = s_index × `OGRID_WIDTH_CELLS` + d_index | índice de celda | Celda que recibe el voto. |
| `vote` | in | `ogrid_vote_kind_t` | `OGRID_VOTE_KIND_ECHO`, `OGRID_VOTE_KIND_PASS` o `OGRID_VOTE_KIND_OCCLUDED` | — | Lo que el haz dice de la celda. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `grid_io` no es `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar nada |
| PRE-2 | `vote` es uno de los tres valores válidos | devuelve `OGRID_ERR_PARAM` sin modificar nada |
| PRE-3 | `cell_idx` < `OGRID_CELL_COUNT` | devuelve `OGRID_ERR_RANGE` sin modificar nada |
| PRE-4 | El `log_odds_lo` actual de la celda es finito y está en [−0,8; 3,4] | devuelve `OGRID_ERR_STATE` sin modificar nada |

Orden de comprobación: PRE-1, PRE-2, PRE-3 y PRE-4, en ese orden; si fallan varias a la vez, se devuelve el error de la primera que falla. PRE-4 se comprueba con cualquier voto, también con `OGRID_VOTE_KIND_OCCLUDED`: una celda corrupta devuelve `OGRID_ERR_STATE` aunque ese voto no la cambie.

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | `OGRID_VOTE_KIND_ECHO`: nuevo log-odds = limitar(anterior + `OGRID_VOTE_ECHO`) y `observed` = `OGRID_OBSERVED_YES` (1). Desde 0, tres ecos seguidos dan 0,85 / 1,70 / 2,55 con tolerancia 1·10⁻⁶. |
| POST-2 | `OGRID_VOTE_KIND_PASS`: nuevo log-odds = limitar(anterior + `OGRID_VOTE_PASS`) y `observed` = `OGRID_OBSERVED_YES` (1). Desde 0 da −0,40; desde −0,8 se queda en −0,8. |
| POST-3 | `OGRID_VOTE_KIND_OCCLUDED`: ni `log_odds_lo` ni `observed` cambian (la celda no ha sido observada por el haz). |
| POST-4 | Con cualquier voto, el log-odds resultante está siempre en [−0,8; 3,4] y las demás celdas del grid no cambian. |
| POST-5 | Desde el mínimo (−0,8), tres ecos seguidos dejan la celda con p > 0,85 (−0,8 + 3 × 0,85 = 1,75 > 1,7346). |
| POST-6 | Si devuelve error, el grid no se modifica. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos y celda coherente | celda actualizada (POST-1 a POST-4) |
| `OGRID_ERR_PARAM` | `grid_io` es `NULL` o `vote` no es válido | grid sin cambios (POST-6) |
| `OGRID_ERR_RANGE` | `cell_idx` ≥ `OGRID_CELL_COUNT` | grid sin cambios (POST-6) |
| `OGRID_ERR_STATE` | el log-odds almacenado es NaN, infinito o está fuera de [−0,8; 3,4] (celda corrupta) | grid sin cambios (POST-6); quien llama debe llevar el sistema a estado seguro (C-ERR-02) |

## 7. Rendimiento (`perf`)

Tiempo constante. Sin bucles ni memoria dinámica. WCET objetivo ≪ 1 µs; se mide en integración. Se llama hasta unas decenas de miles de veces por ciclo de 50 ms, desde F-006.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: F-003 `ogrid_clamp_log_odds` (límites). Estado que modifica: una celda de `*grid_io`.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
