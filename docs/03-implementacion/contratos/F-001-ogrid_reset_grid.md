# Contrato F-001 · `ogrid_reset_grid`

| Campo | Valor |
|---|---|
| Función | `ogrid_reset_grid` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_reset_grid.h` |
| Fuente | `src/ogrid/ogrid_reset_grid.c` |
| Requisitos | SYS-PR-08, SYS-SAF-07, SYS-IMP-01 |
| Apartado de diseño | §6.1, §6.2 (estados de celda), §6.3 (reinicio al volver de FALLO) |
| Estado | Aprobado |

## 1. Propósito

Deja todas las celdas del grid en «zona no estudiada» con log-odds 0. Se usa en el arranque y al volver de FALLO, porque las mediciones anteriores dejan de ser fiables (diseno.md §6.3). Tras el reinicio ninguna celda puede ser «ocupado»: no hay evidencia.

## 2. Firma

```c
ogrid_status_t ogrid_reset_grid(ogrid_t *grid_out);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `grid_out` | out | `ogrid_t *` | no nulo; apunta a un `ogrid_t` completo (cualquier contenido previo, incluso sin inicializar) | — | Grid que se reinicia por completo. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `grid_out` no es `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar salidas |

El contenido previo de `*grid_out` no se lee, por lo que no hay otra precondición.

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Si devuelve `OGRID_OK`, las `OGRID_CELL_COUNT` (3 520) celdas tienen `log_odds_lo` igual a `OGRID_LOG_ODDS_INIT` (0,0), exacto, sin tolerancia. |
| POST-2 | Si devuelve `OGRID_OK`, las `OGRID_CELL_COUNT` celdas tienen `observed` igual a `OGRID_OBSERVED_NO` (0). |
| POST-3 | Si devuelve `OGRID_OK`, `ogrid_get_cell_state` (F-005) dará `OGRID_CELL_UNSURVEYED` para toda celda; ninguna celda queda ocupada ni con probabilidad baja. |
| POST-4 | Llamarla dos veces seguidas deja el mismo resultado (idempotente). |
| POST-5 | Si devuelve error, no se modifica ningún dato. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | `grid_out` no es nulo | grid completo reiniciado (POST-1 a POST-4) |
| `OGRID_ERR_PARAM` | `grid_out` es `NULL` | sin cambios (POST-5) |

## 7. Rendimiento (`perf`)

Complejidad O(`OGRID_CELL_COUNT`) = 3 520 celdas, con bucle de cota estática. Sin memoria dinámica ni recursión. WCET objetivo: muy inferior a 1 ms en la ECU, ya que solo se llama en el arranque y en la vuelta de FALLO, nunca en el bucle de 50 ms. Se medirá en la integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: ninguna. Estado que modifica: solo `*grid_out`. Sin variables globales ni estáticas (C-FUN-08).

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
