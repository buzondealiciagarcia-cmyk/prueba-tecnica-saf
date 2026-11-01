# Especificación de requisitos

| Campo | Valor |
|---|---|
| Proyecto | SAF |
| Código | SAF-REQ |
| Versión | 1.0 |
| Fecha | 2026-11-01 |
| Autor | Álvaro |
| Estado | Cerrado |

## 3. Requisitos funcionales

### REQ-FUN-001 — Recepción de nubes de puntos

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá procesar una nube de puntos del radar cada 50 ms. |
| Tipo | Funcional |
| Prioridad | Must |
| Origen | Enunciado de la práctica |
| Método de verificación | Test |
| Criterio de aceptación | Latencia de procesado ≤ 50 ms con 512 ecos |
| Estado | Aprobado |

### REQ-FUN-002 — Probabilidad acotada

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener la probabilidad de cada celda en el intervalo [0, 1]. |
| Tipo | Funcional |
| Prioridad | Must |
| Origen | Enunciado de la práctica |
| Método de verificación | Test |
| Criterio de aceptación | 0 ≤ p ≤ 1 en 100 % de las celdas tras 1000 ciclos |
| Estado | Aprobado |
