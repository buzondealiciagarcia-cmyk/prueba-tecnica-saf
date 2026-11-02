# Contrato F-003 · `ogrid_clamp_log_odds`

| Campo | Valor |
|---|---|
| Función | `ogrid_clamp_log_odds` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_clamp_log_odds.h` |
| Fuente | `src/ogrid/ogrid_clamp_log_odds.c` |
| Requisitos | SYS-PR-08, SYS-SAF-07 |
| Apartado de diseño | §6.2 (límites del log-odds: máximo 3,4 y mínimo −0,8) |
| Estado | Aprobado |

## 1. Propósito

Limita un valor de log-odds al intervalo [−0,8; 3,4]. El máximo evita que una celda saturada tarde en dejar de ser «ocupado» (5 ciclos); el mínimo garantiza que tres ecos seguidos confirmen un obstáculo, incluso si antes había pasos sin eco.

## 2. Firma

```c
ogrid_status_t ogrid_clamp_log_odds(ogrid_real_t value_lo, ogrid_real_t *clamped_lo_out);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `value_lo` | in | `ogrid_real_t` | finito (no NaN ni infinito); cualquier valor finito es admisible | log-odds (ln) | Valor a limitar. |
| `clamped_lo_out` | out | `ogrid_real_t *` | no nulo | log-odds (ln) | Valor limitado. |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `clamped_lo_out` no es `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar salidas |
| PRE-2 | `value_lo` es un número finito | devuelve `OGRID_ERR_RANGE` sin modificar salidas |

Orden de comprobación: PRE-1 antes que PRE-2; si `clamped_lo_out` es `NULL` y `value_lo` es NaN o infinito, devuelve `OGRID_ERR_PARAM`.

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Si devuelve `OGRID_OK` y `value_lo` está en [`OGRID_LOG_ODDS_MIN`, `OGRID_LOG_ODDS_MAX`], `*clamped_lo_out` = `value_lo` exacto, sin tolerancia (el cero con signo se compara por valor: +0,0 y −0,0 son iguales). |
| POST-2 | Si `value_lo` < `OGRID_LOG_ODDS_MIN`, `*clamped_lo_out` = `OGRID_LOG_ODDS_MIN` (−0,8) exacto. |
| POST-3 | Si `value_lo` > `OGRID_LOG_ODDS_MAX`, `*clamped_lo_out` = `OGRID_LOG_ODDS_MAX` (3,4) exacto. |
| POST-4 | Idempotente: limitar un valor ya limitado devuelve el mismo valor. |
| POST-5 | Si devuelve error, `*clamped_lo_out` no se modifica. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos | `*clamped_lo_out` escrito (POST-1 a POST-4) |
| `OGRID_ERR_PARAM` | `clamped_lo_out` es `NULL` | sin cambios (POST-5) |
| `OGRID_ERR_RANGE` | `value_lo` es NaN o infinito | sin cambios (POST-5) |

## 7. Rendimiento (`perf`)

Tiempo constante, dos comparaciones. Sin memoria dinámica. WCET objetivo ≪ 1 µs; se mide en integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: ninguna. Estado que modifica: solo `*clamped_lo_out`.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
