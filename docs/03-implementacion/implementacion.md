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

Se implementa el núcleo del grid de ocupación en log-odds (diseno.md §6 y paso 10), por ser el componente central del sistema y el que SYS-IMP-10 toma como ejemplo. Requisitos cubiertos: SYS-PR-08, SYS-SAF-07, SYS-PR-04 (máscara de celdas ocupadas) y SYS-IMP-01 (sin memoria dinámica).

El estado por celdas no se publica al HMI: el análisis de la rejilla es interno y al HMI solo llega la lista de objetos (decisión del usuario, 2026-11-02). SYS-OUT-06 queda pendiente de reformular al reabrir requisitos.

Quedan fuera de este alcance: agrupación en objetos, filtro de tamaño, confirmación de N ciclos (pasos 11 a 12), geometría y conversión a coordenadas de vía, y desplazamiento del grid con el avance del tranvía.

Decisiones de implementación (el diseño v1.0 no las fijaba), tomadas por el usuario el 2026-11-02:

- Representación numérica: `float` de 32 bits (IEEE-754 binary32) mediante `ogrid_real_t` (C-TYP-02). Ocupa 4 bytes por valor y la comprobación está en `ogrid_platform.h`.
- Grid fijo en coordenadas de vía (s, d): el módulo recibe índices de celda ya calculados.
- Los valores del modelo (votos, límites, umbral) son constantes de compilación por defecto en `ogrid_limits.h`, tomadas de diseno.md §10.

## 2. Arquitectura del módulo

![Figura. Estructura del módulo](diagramas/modulo.svg)

Las flechas «llama a» van de la función que usa a la función que le da servicio: por ejemplo, F-005 llama a F-002 para obtener la probabilidad. Las flechas «lee» y «escribe» indican qué hace cada función con el grid.

### 2.1 Cómo encajan las funciones

El módulo `ogrid` guarda una rejilla de 880 × 4 celdas (la vía de 220 m dividida en cuadrados de 0,25 m). Cada celda tiene dos datos: su **log-odds**, un número que mide cuánta evidencia hay de que exista un obstáculo, y si alguna vez ha sido **observada**. Las siete funciones se reparten en tres papeles:

- **Escriben el grid.** `ogrid_reset_grid` (F-001) lo deja todo en «zona no estudiada» al arrancar o al volver de FALLO. Para cada haz del radar, `ogrid_update_ray` (F-006) recorre las celdas que cruza y llama a `ogrid_update_cell` (F-004) con el voto que toca. F-004 suma el voto al log-odds y pide a `ogrid_clamp_log_odds` (F-003) que lo mantenga entre −0,8 y 3,4.
- **Leen el grid.** `ogrid_get_cell_state` (F-005) decide en qué estado está una celda. Para ello necesita la probabilidad, que calcula `ogrid_log_odds_to_prob` (F-002) con p = 1 / (1 + e^−l).
- **Entrega el resultado.** `ogrid_extract_occupied` (F-007) devuelve una máscara con las celdas ocupadas, que será la entrada de la agrupación en objetos. Pregunta a F-005, de modo que la máscara y el estado de celda nunca se contradicen. Al HMI no llega el estado por celdas, sino solo la lista de objetos que saldrá de esa agrupación.

**Orden de dependencias.** Cada función solo usa funciones anteriores del plan: F-004 usa F-003; F-005 usa F-002; F-006 usa F-004; F-007 usa F-005. Por eso se implementan en este orden, de las más básicas a las compuestas.

**Ejemplo de un obstáculo.** Una celda empieza con log-odds 0 y no observada, es decir, «zona no estudiada». Un haz con eco en ella suma +0,85, y ya cuenta como observada. El log-odds pasa de 0 a 0,85, p = 0,70 y el estado es «probabilidad baja de obstáculo». Con el segundo eco llega a 1,70 (p = 0,8455), todavía por debajo del umbral 0,85. Con el tercero sube a 2,55 (p = 0,93) y la celda pasa a **ocupado**. Un eco suelto no basta. Si en cambio la onda pasa por la celda sin eco, resta 0,40, y si la celda queda detrás de un eco no cambia. El mínimo de −0,8 asegura que, aunque la celda haya perdido mucha evidencia, tres ecos seguidos la vuelvan a marcar como ocupada.

**Qué no ocurre nunca.** No existe el estado «libre»: que el radar no vea nada en una celda no demuestra que esté despejada (SYS-SAF-07). La probabilidad numérica y el estado por celdas son internos y no salen del módulo.

**Errores y seguridad.** Todas las funciones devuelven un código (`OGRID_OK`, `OGRID_ERR_PARAM`, `OGRID_ERR_RANGE` u `OGRID_ERR_STATE`) y no tocan nada si el parámetro es inválido. Si el grid contiene un valor corrupto, las funciones de lectura lo tratan como «zona no estudiada» y devuelven `OGRID_ERR_STATE`, para que quien las llama pase a un estado seguro.

**Qué queda fuera.** Este módulo no decide qué celdas cruza cada haz (eso es geometría), no agrupa celdas en objetos, no los confirma tras N ciclos y no desplaza el grid con el avance del tranvía.

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
