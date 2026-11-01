# STD-SAF · Seguridad funcional

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Referencias | EN 50126 (RAMS), EN 50128 (software ferroviario), EN 50129, IEC 61508 como marco general |
| Alcance | Práctica de simulación: se aplican las prácticas, no se reclama certificación |

## 1. Principios

| ID | Nivel | Regla |
|---|---|---|
| SAF-01 | OBL | **Estado seguro definido en diseño.** Para un detector de obstáculos, ante duda o fallo el sistema no informa "vía libre": publica estado degradado/fallo y la celda o el sistema pasa a *desconocido*. Nunca se fabrica una salida "libre" sin evidencia. |
| SAF-02 | OBL | Toda condición de fallo detectable (sensor mudo, datos inválidos, desbordamiento, reloj, saturación de CPU) tiene: detección, reacción, notificación (log `CRIT` + topic de diagnóstico) y recuperación documentadas en la tabla de fallos del diseño. |
| SAF-03 | OBL | Análisis de modos de fallo ligero (FMEA) del diseño en `docs/02-diseno/diseno.md`: `Modo de fallo | Causa | Efecto | Detección | Mitigación | Requisito`. El revisor de diseño comprueba que cada fallo del sensor y de la red aparece. |
| SAF-04 | OBL | Programación defensiva (STD-C §5–6): validación de entradas, invariantes, sin memoria dinámica, sin recursión, tiempos acotados. |
| SAF-05 | OBL | Trazabilidad completa requisito ↔ diseño ↔ código ↔ test (`h trace`); un requisito de seguridad sin test asociado bloquea el cierre de implementación. |
| SAF-06 | OBL | Independencia de verificación: quien implementa no escribe los tests (STD-TST §1). |
| SAF-07 | OBL | Toda desviación de estándar (MISRA, complejidad) se registra con análisis de impacto en seguridad. |
| SAF-08 | REC | Supuestos de seguridad (*safety assumptions*) explícitos en el diseño: lo que el sistema asume del entorno (velocidad máxima del tranvía, campo de visión del radar, tasa de falsos ecos). |
