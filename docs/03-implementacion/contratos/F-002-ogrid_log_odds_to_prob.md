# Contrato F-002 · `ogrid_log_odds_to_prob`

| Campo | Valor |
|---|---|
| Función | `ogrid_log_odds_to_prob` |
| Módulo | `ogrid` |
| Cabecera | `include/ogrid/ogrid_log_odds_to_prob.h` |
| Fuente | `src/ogrid/ogrid_log_odds_to_prob.c` |
| Requisitos | SYS-PR-08 |
| Apartado de diseño | §6.2 (p = 1 / (1 + e^−l), logaritmo natural) |
| Estado | Aprobado |

## 1. Propósito

Convierte el log-odds de una celda en probabilidad de ocupación. Es la única conversión del módulo: el estado de celda (F-005) y la máscara de ocupadas (F-007) comparan esta probabilidad con el umbral 0,85. La probabilidad es interna y no sale del módulo.

## 2. Firma

```c
ogrid_status_t ogrid_log_odds_to_prob(ogrid_real_t log_odds_lo, ogrid_real_t *prob_out);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|
| `log_odds_lo` | in | `ogrid_real_t` | finito, entre `OGRID_LOG_ODDS_MIN` (−0,8) y `OGRID_LOG_ODDS_MAX` (3,4), ambos incluidos | log-odds (ln) | Log-odds de una celda. |
| `prob_out` | out | `ogrid_real_t *` | no nulo | probabilidad (0 a 1) | Probabilidad p = 1 / (1 + e^−l). |

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | `prob_out` no es `NULL` | devuelve `OGRID_ERR_PARAM` sin modificar salidas |
| PRE-2 | `log_odds_lo` es un número finito (no NaN ni infinito) dentro de [`OGRID_LOG_ODDS_MIN`, `OGRID_LOG_ODDS_MAX`] | devuelve `OGRID_ERR_RANGE` sin modificar salidas |

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | Si devuelve `OGRID_OK`, `*prob_out` = 1 / (1 + e^−`log_odds_lo`) con error absoluto ≤ 1·10⁻⁶ (float32). |
| POST-2 | Valores de referencia con la misma tolerancia: l = 0 → 0,5; l = 0,85 → 0,7006; l = 1,70 → 0,8455; l = 2,55 → 0,9276; l = 3,4 → 0,9677; l = −0,8 → 0,3100. |
| POST-3 | Si devuelve `OGRID_OK`, 0 < `*prob_out` < 1, y la función es estrictamente creciente: l1 < l2 ⇒ p1 < p2. |
| POST-4 | En l = 0 el resultado es exactamente 0,5 (tolerancia 1·10⁻⁶). |
| POST-5 | Si devuelve error, `*prob_out` no se modifica. |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `OGRID_OK` | Parámetros válidos | `*prob_out` escrito (POST-1 a POST-4) |
| `OGRID_ERR_PARAM` | `prob_out` es `NULL` | sin cambios (POST-5) |
| `OGRID_ERR_RANGE` | `log_odds_lo` es NaN, infinito o está fuera de [−0,8; 3,4] | sin cambios (POST-5) |

## 7. Rendimiento (`perf`)

Tiempo constante, una exponencial de `<math.h>` con el dominio comprobado antes (C-LIB-03). Sin bucles, sin memoria dinámica. WCET objetivo ≪ 1 µs en la ECU; se mide en integración.

## 8. Efectos laterales y dependencias

Funciones del plan que usa: ninguna. Estado que modifica: solo `*prob_out`. Sin variables globales.

## 9. Ejemplos de referencia

Sin ejemplos aportados por el usuario.
