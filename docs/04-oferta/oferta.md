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

<!-- seccion: alcance | estado: aprobado -->
## 5. Alcance del proyecto y de los resultados

### 5.1 Incluido

- **Fase A.** Plan de viabilidad del uso del radar. Diseño hardware y software del sistema, con el diagrama del pipeline de un ciclo y la gestión de fallos. Código de un componente (el grid de ocupación) y justificación del lenguaje C. Este borrador de oferta. Como demostrador: el módulo del grid implementado y verificado (siete funciones, 209 casos de test, tiempos medidos en el equipo de desarrollo) y su visor.
- **Fase B.** Implementación completa del sistema: recepción y validación de los datos del radar y del tranvía, geometría de vía, agrupación en objetos, confirmación, confianza e incertidumbre, máquina de estados y gestión de fallos, salida al HMI y envío de estado, diagnóstico y logs a la nube por MQTT. Verificación independiente, trazabilidad y documentación. Las pruebas son en banco, con entradas simuladas o grabadas.

### 5.2 Excluido

- **Hardware** (radar, ECU y sensor de velocidad). Lo suministra el cliente y se acuerda con Ikerlan. En el siguiente borrador de la oferta el hardware deberá quedar incluido.
- **Instalación y montaje en el tranvía.** Corren por cuenta del cliente y del propietario del tranvía.
- **Pruebas en tranvía real** con el cliente, para validar las falsas alarmas y la precisión del radar elegido. Se acuerdan aparte.
- **Certificación formal.** Las normas EN 50126, EN 50128 y EN 50129 con un nivel de integridad objetivo SIL 2 se aplican como práctica de simulación, sin reclamar certificación.
- **Segundo radar.** Se recomienda como mejora opcional, fuera del alcance base, porque reduce el error de medida y aporta redundancia, lo que mejora la seguridad de un sistema con seguridad funcional. La propuesta de Ikerlan pide un único radar, y esta opción no tiene precio por ahora.
- **Evoluciones futuras** sugeridas al cliente: acceso de la ECU al sistema de freno, actualizaciones de flota y descarga de mapas desde la nube, y persistencia del último estado del sistema entre arranques.

### 5.3 Entregables

- **Fase A:** plan de viabilidad, documento de diseño, código del grid y justificación del lenguaje C, documentos de requisitos e implementación, y este borrador de oferta.
- **Fase B:** el software del sistema completo, su documentación y sus informes de verificación.

### 5.4 Garantía

Garantía de 12 meses para la corrección de defectos.

<!-- seccion: requisitos | estado: aprobado -->
## 6. Requisitos

Los requisitos del sistema están cerrados y aprobados (versión 1.2): 64 requisitos, de los que 62 son obligatorios (Must) y 2 deseables (Should), 48 funcionales, 5 no funcionales y 11 restricciones, además de 8 hipótesis de partida. El detalle completo está en el documento de requisitos.

| Categoría | Nº | Qué cubren |
|---|---:|---|
| Detección y prestaciones | 7 | Alcance de 220 m, tamaño mínimo de 0,6 m, exclusión de objetos fuera de la vía, falsas alarmas, error de localización y error angular |
| Entradas | 7 | Nube de puntos del radar, velocidad y giro del tranvía, marcas de tiempo, altura de montaje, validación de trama y de reloj |
| Procesamiento | 11 | Coordenadas de vía, compensación del movimiento, filtrado de raíles, agrupación de celdas, objetos estáticos y móviles, probabilidad de ocupación y confirmación en N ciclos |
| Salida | 10 | Lista de objetos con posición, tamaño, incertidumbre, confianza, identificador y marca de tiempo; aviso y registro de cambio de modo |
| Tiempo | 1 | Entrega en menos de 128 ms |
| Seguridad y modos | 16 | Modos normal, degradado y fallo, resets, recuperación, no declarar nunca la ausencia de obstáculo |
| Normativa | 2 | Seguridad funcional (SIL 2 como práctica) y documentación del entorno operativo |
| Implementación | 10 | Sin memoria dinámica, comportamiento determinista, procesamiento a bordo, nube solo para trazabilidad y los entregables de la práctica |

Los cuatro requisitos de la propuesta de Ikerlan se cubren así: la detección (requisito 1) con los de detección y procesamiento; el análisis de radar, velocidad y giro (2) con los de entradas y procesamiento; la entrega en menos de 128 ms con posición, tamaño y confianza (3) con los de salida y tiempo; y el paso a modo degradado o error (4) con los de seguridad y modos.

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
