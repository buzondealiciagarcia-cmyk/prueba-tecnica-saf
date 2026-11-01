# Requisitos del sistema de detección de obstáculos estáticos en vía (radar frontal)

| Campo | Valor |
|---|---|
| Proyecto | SAF e2e |
| Código | SAF-REQ |
| Versión | 1.0 |
| Fecha | 2026-11-01 |
| Autor | E2E |
| Estado | Borrador |

Redacción según ISO/IEC/IEEE 29148:2018, con plantillas EARS: «El sistema deberá…» para requisitos siempre activos y «Si…, entonces el sistema deberá…» para la respuesta ante fallos.

Codificación: SYS-\<categoría\>-\<nn\> para requisitos del sistema y H-\<nn\> para hipótesis. Categorías: DET = detección y prestaciones; IN = entradas; PR = procesamiento; OUT = salida; T = tiempo; SAF = seguridad y modos de funcionamiento; GEN = normativa; IMP = implementación.

Source: R1–R4 son los requisitos del enunciado; «Derivado» indica un requisito o hipótesis necesario para el diseño que no aparece explícitamente en el enunciado. Los valores entre corchetes \[X\] están pendientes de definir.

<!--
Formato de cada requisito (STD-REQ). En el markdown, un valor pendiente se escribe
[PENDIENTE: valor propuesto] (o [PENDIENTE] si no hay propuesta) y se exporta como [valor] / [X].
Ningún valor pendiente puede quedar al cerrar la fase.

### SYS-IN-01 — Título corto
| Campo | Valor |
|---|---|
| Descripción | El sistema deberá … |
| Tipo | Funcional |
| Origen | R1 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | … (magnitud + unidad + condición) |
| Estado | Borrador |

Hipótesis: código H-nn en la sección Hipótesis, Tipo = Hipótesis; Prioridad, Método de
verificación y Criterio de aceptación son opcionales.
-->

## Normativa

## Entradas

### SYS-IN-01 — Recepción de nubes de puntos

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá procesar una nube de puntos del radar cada 50 ms. |
| Tipo | Funcional |
| Origen | R1 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Latencia de procesado ≤ 50 ms con 512 ecos |
| Estado | Aprobado |


## Procesamiento

### SYS-PR-01 — Probabilidad acotada

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener la probabilidad de cada celda en el intervalo [0, 1]. |
| Tipo | Funcional |
| Origen | R2 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 ≤ p ≤ 1 en 100 % de las celdas tras 1000 ciclos |
| Estado | Aprobado |


## Detección y prestaciones

## Salida

## Tiempo

## Seguridad y modos de funcionamiento

## Implementación

## Hipótesis

### H-01 — Vía recta

| Campo | Valor |
|---|---|
| Descripción | La vía es recta en el campo de visión del radar. |
| Tipo | Hipótesis |
| Origen | Derivado |
| Estado | Aprobado |


## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-01 | E2E | Creación a partir de la plantilla Requirements_2 |
