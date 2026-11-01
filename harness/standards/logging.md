# STD-LOG · Logging

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Formato | Una línea de texto estructurada `key=value` por evento |
| Niveles | Syslog (RFC 5424) |

## 1. Formato

```
2026-11-01T10:00:00.123Z ERROR ogrid cyc=1234 seq=88 code=OGRID_ERR_RANGE msg="cell index out of grid" x=412 y=17
```

| Campo | Regla |
|---|---|
| timestamp | ISO 8601 UTC con milisegundos |
| nivel | `EMERG ALERT CRIT ERROR WARN NOTICE INFO DEBUG` (ancho fijo, mayúsculas) |
| componente | prefijo del módulo (`ogrid`, `hal`, `app`, `mqtt`, `ui`) |
| campos | `key=value` en `snake_case`; valores con espacios entre comillas dobles; `msg` en inglés; siempre `cyc` (ciclo) en el núcleo |

## 2. Reglas

| ID | Nivel | Regla |
|---|---|---|
| LOG-01 | OBL | El núcleo no hace E/S: escribe eventos en un buffer circular de tamaño fijo (`log_ring_t`) que vacía el hilo de `app/` fuera del ciclo de 50 ms. Si el buffer se llena se descartan los de menor nivel y se cuenta `log_dropped`. |
| LOG-02 | OBL | Uso de niveles: `CRIT` = transición a estado seguro; `ERROR` = fallo que degrada una salida; `WARN` = condición anómala recuperable; `NOTICE` = cambio de modo/configuración; `INFO` = arranque/parada/resumen periódico (≤ 1 Hz); `DEBUG` = solo en builds de desarrollo. |
| LOG-03 | OBL | Prohibido loguear en `INFO` o superior dentro del bucle por cada ciclo. Los resúmenes periódicos agregan (máx., media, contadores). |
| LOG-04 | OBL | Cada código de error (`*_status_t`) tiene un mensaje único y estable; se documentan en `docs/03-implementacion/implementacion.md`. |
| LOG-05 | OBL | Destino: `stderr` en desarrollo, fichero rotado en `deploy` (`/var/log/saf/*.log`, 10 MB × 5) y topic MQTT de diagnóstico para `WARN` o superior. |
| LOG-06 | OBL | Scripts y herramientas usan el mismo formato (función `log` de `tools/lib/log.sh` y `logging` de Python con el formatter del proyecto). |
