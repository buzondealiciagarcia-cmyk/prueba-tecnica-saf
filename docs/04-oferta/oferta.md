# Oferta técnica y económica

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-OFE |
| Versión | 0.1 |
| Fecha | 2026-11-03 |
| Autor | Alicia Garcia |
| Cliente | Ikerlan |
| Destinatario | Equipo técnico de Ikerlan |
| Idioma y tono | Español, técnico y formal |
| Validez de la oferta | 60 días |
| Moneda e impuestos | EUR, impuestos aparte |
| Demostrador | La implementación realizada (módulo del grid, verificación y visor) se incluye como demostrador de la Fase A, sin coste aparte |
| Estado | Borrador |

<!-- seccion: objeto | estado: aprobado -->
## 1. Objeto de la oferta

Esta oferta propone a Ikerlan la ejecución de un sistema de detección de obstáculos estáticos en la vía de un tranvía, basado en un radar frontal 3D y en procesamiento a bordo del tranvía (ECU edge). El sistema entrega al HMI del conductor la lista de los objetos estáticos detectados dentro de la vía y a menos de 220 m, con su posición, tamaño, incertidumbre y confianza, y acompaña cada publicación con el modo del sistema y un indicador de vigilancia, de modo que una lista vacía nunca se interprete como vía libre. El entorno de referencia es un tranvía bidireccional con velocidad máxima de 50 km/h.

La ejecución se ofrece en dos fases:

- **Fase A.** Plan de viabilidad del uso del radar, diseño hardware y software del sistema, con el diagrama del pipeline de un ciclo y la gestión de los fallos del sensor, y (pseudo)código de un componente del sistema, acompañado de un demostrador: el módulo del grid de ocupación ya implementado y verificado, con su visor.
- **Fase B.** Implementación completa del sistema.

La oferta la presenta Alicia Garcia, a título personal.

<!-- seccion: introduccion | estado: aprobado -->
## 2. Introducción

La propuesta inicial de Ikerlan, del equipo SAF de la unidad DIG («percepción confiable, del sensor a la decisión»), plantea diseñar un algoritmo basado en un grid de ocupación dinámico para identificar obstáculos estáticos sobre la vía de un tranvía, usando un radar frontal. El radar entrega cada 50 ms una nube de puntos con sus ecos, y el resultado debe ser un grid con la probabilidad de ocupación de cada celda, basada exclusivamente en esos ecos. El sistema debe detectar objetos estáticos de más de 0,60 m de lado situados en la vía a menos de 220 m, analizar los datos del radar junto con la velocidad y el giro del tranvía, entregar los objetos detectados (posición, tamaño y confianza) en menos de 128 ms, y pasar a modo degradado o declarar un error ante fallos del sensor.

Un tranvía que circula a 50 km/h recorre unos 14 m cada segundo, y los 220 m de alcance exigido los cubre en unos 16 s. La detección debe ser fiable, determinista y de tiempo acotado, y nunca debe dar a entender que la vía está libre cuando no se ha podido comprobar: es un sistema que requiere seguridad funcional.

La solución propuesta procesa los datos del radar frontal 3D en una unidad a bordo del tranvía (procesamiento en el borde) y no en la nube. Se descarta la nube por latencia, fiabilidad y restricciones de tiempo, que pondrían en riesgo el cumplimiento de los estándares del proyecto; el procesamiento a bordo ofrece buena latencia y disponibilidad, es certificable y es más seguro frente a ataques.

El trabajo se plantea por fases, con requisitos cerrados y aprobados, un diseño revisado contra ellos y una implementación en la que el código y sus pruebas los escriben y revisan de forma independiente. Los primeros resultados confirman que el planteamiento es viable: el plan de viabilidad concluye que un radar 3D con la precisión angular exigida permite cubrir los 220 m, y el módulo central del sistema, el grid de ocupación, ya está implementado y verificado.

<!-- seccion: datos | estado: aprobado -->
## 3. Datos utilizados

**Datos de entrada del sistema en operación**

- **Nube de puntos del radar**, cada 50 ms, con los ecos (distancia, azimut y altura) y una marca de tiempo de medida por trama.
- **Velocidad lineal y velocidad de giro del tranvía**, también cada 50 ms y con marca de tiempo de medida, para sincronizarlas con cada nube de puntos.
- **Parámetros de configuración**, como la altura de montaje del radar (1 m). Los que dependen del radar, del tranvía o de la red se fijan con sus hojas de datos antes de implementar.
- **Órdenes del conductor** desde el HMI: sign in y resets.

**Datos que produce el sistema.** La lista de objetos estáticos detectados en cada ciclo, con posición, tamaño, incertidumbre, confianza, identificador y marca de tiempo, junto con el modo del sistema y el indicador de vigilancia.

**Datos de desarrollo y verificación.** Hasta ahora se han usado entradas calculadas a mano a partir de los requisitos y del diseño. No se dispone de grabaciones reales del radar ni del tranvía.

**Datos que aportaría el cliente.** Por ahora Ikerlan no aporta ningún dato. Más adelante se acordaría con Ikerlan el hardware específico (radar, sensor de velocidad y red del tranvía), y de ese acuerdo saldrían los datos que se necesiten, como las hojas de datos y, si procede, grabaciones reales.

**Confidencialidad.** Los datos del cliente se tratan como confidenciales: se usan solo para este proyecto, los datos reales se anonimizan y se documenta su procedencia, y no se difunden ni se publican.

<!-- seccion: objetivos | estado: aprobado -->
## 4. Objetivos

**Objetivo general.** Detectar obstáculos estáticos sobre la vía de un tranvía con un radar frontal 3D y entregarlos al conductor con la fiabilidad que exige un sistema con seguridad funcional.

**Objetivos del sistema**, por orden de prioridad:

1. **Detección.** Detectar los objetos estáticos con las tres dimensiones de 0,6 m o más, situados en la vía a menos de 220 m, usando un único radar frontal.
2. **Tiempo.** Entregar los objetos detectados (posición, tamaño, incertidumbre y confianza) en menos de 128 ms desde la recepción de cada nube de puntos, de forma determinista y con tiempo acotado.
3. **Seguridad funcional.** No comunicar nunca la ausencia de obstáculo; pasar a modo degradado o declarar un error ante fallos del sensor, y no dar vía libre en modo fallo.
4. **Entradas.** Analizar los datos del radar junto con la velocidad y el giro del tranvía, compensando el movimiento.
5. **Calidad del software.** Código en C sin memoria dinámica, verificable y trazable hasta los requisitos.

**Qué cubre esta práctica (Fase A) y qué queda para la Fase B.** La Fase A demuestra la viabilidad y deja diseñados los cinco objetivos; implementa solo el módulo del grid de ocupación, que es el componente central de la detección.

| Objetivo | En esta práctica (Fase A) | Queda para la Fase B |
|---|---|---|
| 1. Detección | Viabilidad con un radar 3D demostrada; diseño del grid y de la extracción de objetos; **implementado**: grid de ocupación y máscara de celdas ocupadas | Agrupación en objetos, filtro de tamaño, confirmación en N ciclos y geometría de vía |
| 2. Tiempo | Presupuesto de tiempo diseñado (128 ms de entrega, 78 ms de proceso); tiempos del grid medidos en el equipo de desarrollo | Medida del ciclo completo en la ECU |
| 3. Seguridad funcional | Modos y gestión de fallos diseñados; **implementado**: el grid nunca declara una celda libre | Máquina de estados, gestión de fallos y salida al HMI |
| 4. Entradas | Entradas y su validación diseñadas | Recepción, validación y sincronización del radar y del tranvía |
| 5. Calidad del software | **Aplicada** al módulo implementado: C sin memoria dinámica, verificación independiente y trazabilidad | Aplicación al resto del sistema |

<!-- seccion: alcance | estado: borrador -->
## 5. Alcance del proyecto y de los resultados

### 5.1 Incluido

### 5.2 Excluido

### 5.3 Entregables

<!-- seccion: requisitos | estado: borrador -->
## 6. Requisitos

<!-- seccion: descripcion | estado: borrador -->
## 7. Descripción del proyecto

<!-- seccion: presupuesto | estado: borrador -->
## 8. Presupuesto

<!-- h:budget:begin -->
<!-- h:budget:end -->

### 8.1 Condiciones económicas

<!-- seccion: plazo | estado: borrador -->
## 9. Plazo del proyecto

<!-- h:plazo:begin -->
<!-- h:plazo:end -->

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-03 | Alicia Garcia | Creación |
