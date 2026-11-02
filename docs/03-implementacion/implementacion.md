# Documento de implementación

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-IMP |
| Versión | 0.1 |
| Fecha | 2026-11-02 |
| Autor | Alicia Garcia |
| Estado | Borrador |

## 1. Alcance implementado

Se implementa el núcleo del grid de ocupación en log-odds (diseno.md §6 y paso 10), por ser el componente central del sistema y el que SYS-IMP-10 toma como ejemplo. Requisitos cubiertos: SYS-PR-08, SYS-SAF-07, SYS-OUT-06, SYS-PR-04 (máscara de celdas ocupadas) y SYS-IMP-01 (sin memoria dinámica).

Quedan fuera de este alcance: agrupación en objetos, filtro de tamaño, confirmación de N ciclos (pasos 11 a 12), geometría y conversión a coordenadas de vía, y desplazamiento del grid con el avance del tranvía.

Decisiones de implementación (el diseño v1.0 no las fijaba), tomadas por el usuario el 2026-11-02:

- Representación numérica: `float` de 32 bits (IEEE-754 binary32) mediante `ogrid_real_t` (C-TYP-02). Ocupa 4 bytes por valor y la comprobación está en `ogrid_platform.h`.
- Grid fijo en coordenadas de vía (s, d): el módulo recibe índices de celda ya calculados.
- Los valores del modelo (votos, límites, umbral) son constantes de compilación por defecto en `ogrid_limits.h`, tomadas de diseno.md §10.

## 2. Arquitectura del módulo

![Figura. Estructura del módulo](diagramas/modulo.svg)

## 3. Funciones

<!-- Una ficha por función del plan: contrato resumido, algoritmo, diagrama si aporta, casos de test y resultado. -->

## 4. Verificación

### 4.1 Proceso (implementación y test independientes)

### 4.2 Resultados por función

### 4.3 Integración completa

### 4.4 Cumplimiento de estándares y desviaciones

## 5. Trazabilidad

Ver `trazabilidad.md` (generado con `h trace`).

## 6. Construcción, ejecución y despliegue

## 7. Interfaz de usuario (si aplica)

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-02 | Alicia Garcia | Creación |
