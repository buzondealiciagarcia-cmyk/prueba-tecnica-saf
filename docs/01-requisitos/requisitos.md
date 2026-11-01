# Requisitos del sistema de detección de obstáculos en vía, estáticos y en movimiento (radar frontal)

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-REQ |
| Versión | 0.2 |
| Fecha | 2026-11-01 |
| Autor | Alicia Garcia |
| Estado | Borrador |

Redacción según ISO/IEC/IEEE 29148:2018, con plantillas EARS: «El sistema deberá…» para requisitos siempre activos y «Si…, entonces el sistema deberá…» para la respuesta ante fallos.

Codificación: SYS-\<categoría\>-\<nn\> para requisitos del sistema y H-\<nn\> para hipótesis. Categorías: DET = detección y prestaciones; IN = entradas; PR = procesamiento; OUT = salida; T = tiempo; SAF = seguridad y modos de funcionamiento; GEN = normativa; IMP = implementación.

Source: en este proyecto no existen requisitos del enunciado R1–R4; el origen se cita como «Derivado de requisitos.txt (sección, línea)» sobre las notas del usuario (`entradas/requisitos.txt`). «Derivado» indica además un requisito o hipótesis necesario para el diseño que no aparece explícitamente en esas notas; «Derivado» indica un requisito o hipótesis necesario para el diseño que no aparece explícitamente en el enunciado. Los valores entre corchetes \[X\] están pendientes de definir.

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

### SYS-GEN-01 — Cumplimiento de normativa de seguridad funcional

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá cumplir los requisitos de seguridad funcional de la normativa [PENDIENTE: EN 50126, EN 50128 y EN 50129; nivel de integridad SIL por definir]. |
| Tipo | Restricción |
| Origen | Derivado de requisitos.txt (Requisitos, L19) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Informe de cumplimiento de [PENDIENTE: normas y nivel SIL] sin desviaciones abiertas en el 100 % de los requisitos SYS-SAF. |
| Estado | Borrador |


## Entradas

### SYS-IN-01 — Recepción de la nube de puntos del radar

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir del radar una nube de puntos con un periodo de 50 ms. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L6) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se reciben nubes con periodo de 50 ms [PENDIENTE: tolerancia] durante 1 h de ejecución. |
| Estado | Borrador |

### SYS-IN-02 — Recepción de la velocidad del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir la velocidad del tranvía con una resolución de [PENDIENTE] y una frecuencia de [PENDIENTE]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L27-L29) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se recibe la velocidad con resolución [PENDIENTE: X m/s] y frecuencia [PENDIENTE: X Hz]. |
| Estado | Borrador |

### SYS-IN-03 — Recepción de la velocidad de giro del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir la velocidad de giro (ω) del tranvía con una resolución de [PENDIENTE] y una frecuencia de [PENDIENTE]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L9, L27) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se recibe ω con resolución [PENDIENTE: X rad/s] y frecuencia [PENDIENTE: X Hz]. |
| Estado | Borrador |

### SYS-IN-04 — Recepción de la marca de tiempo de cada trama

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir una marca de tiempo (timestamp) asociada a cada trama del radar. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L41; Condiciones de fallo, L52) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las tramas procesadas en 1 h de ejecución incluye timestamp. |
| Estado | Borrador |

### SYS-IN-05 — Altura de montaje del radar como parámetro

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá disponer de la altura de montaje del radar como parámetro de entrada de [PENDIENTE: valor en m y origen del dato]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L18) |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | La altura del radar es un parámetro con valor [PENDIENTE: X m] documentado en configuración. |
| Estado | Borrador |


## Procesamiento

### SYS-PR-01 — Coordenadas respecto a la vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá expresar las coordenadas de las detecciones del radar tomando la vía como referencia. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L17) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una detección de posición conocida se expresa en coordenadas de vía con error ≤ [PENDIENTE: X m]. |
| Estado | Borrador |

### SYS-PR-02 — Compensación del movimiento del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá compensar el movimiento del tranvía en las detecciones del radar. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L30) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Un objeto fijo se mantiene en la misma posición de vía con desviación ≤ [PENDIENTE: X m] con el tranvía a velocidades de 0 a [PENDIENTE: v_max km/h]. |
| Estado | Borrador |

### SYS-PR-03 — Filtrado del eco de los raíles

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá filtrar el eco de los raíles mediante una detección constante y una predicción del eco. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L32) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Con vía libre, ecos de raíl reportados como obstáculo ≤ [PENDIENTE: X] por hora. |
| Estado | Borrador |

### SYS-PR-04 — Agrupación de celdas en objetos

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá agrupar las celdas detectadas para formar objetos de mayor tamaño. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L35) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Un objeto de 1 m² cubierto por varias celdas se reporta como 1 objeto en el 100 % de los casos del conjunto de pruebas. |
| Estado | Borrador |

### SYS-PR-05 — Clasificación de objeto en movimiento o estático

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá clasificar cada objeto detectado como estático o en movimiento comparando su velocidad con un umbral de [PENDIENTE: umbral de velocidad]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L8); respuesta del usuario a Q-001 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Clasificación correcta en ≥ [PENDIENTE: X %] de los objetos de prueba con umbral de [PENDIENTE: X m/s]. |
| Estado | Borrador |

### SYS-PR-06 — Cálculo del ángulo de giro a partir de ω

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá calcular el ángulo de giro del tranvía a partir de la velocidad de giro (ω). |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L9-L10) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Error del ángulo calculado ≤ [PENDIENTE: X °] en curva de radio [PENDIENTE: X m]. |
| Estado | Borrador |

### SYS-PR-07 — Corrección por altura del radar 2D [PROPUESTA]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá corregir o acotar la desviación de la posición de un objeto debida a que el radar no mide altura, usando la altura del radar. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L18, L20-L22) |
| Prioridad | Should |
| Método de verificación | Análisis |
| Criterio de aceptación | Desviación residual de posición ≤ [PENDIENTE: X m] a 220 m para objetos de altura entre [PENDIENTE: X] m y [PENDIENTE: X] m. |
| Estado | Borrador |

### SYS-PR-08 — Mantenimiento del estado ocupado, libre o desconocido

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener actualizado el estado de la vía como ocupado, libre o desconocido. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L31) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El estado se actualiza en cada ciclo de 50 ms y toma uno de tres valores (ocupado, libre, desconocido) en el 100 % de los ciclos. |
| Estado | Borrador |

### SYS-PR-09 — Confirmación del obstáculo antes del aviso

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá confirmar un obstáculo en [PENDIENTE: N] ciclos consecutivos antes de emitir el aviso. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L42) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El aviso se emite en el ciclo N = [PENDIENTE: N ciclos] y no antes. |
| Estado | Borrador |

### SYS-PR-10 — Rechazo de datos con velocidad del tranvía negativa

| Campo | Valor |
|---|---|
| Descripción | Si la velocidad del tranvía es menor que 0, entonces el sistema deberá rechazar la detección [PENDIENTE: ¿se rechaza la detección, la trama o se entra en modo degradado?]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L29) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Con velocidad < 0 m/s se rechaza el 100 % de las tramas de prueba. |
| Estado | Borrador |

### SYS-PR-11 — Rechazo de detecciones a más de 220 m

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá rechazar las detecciones situadas a una distancia mayor de 220 m. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L29) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Ninguna detección a > 220 m aparece en la salida en el conjunto de pruebas. |
| Estado | Borrador |


## Detección y prestaciones

### SYS-DET-01 — Alcance de detección de 220 m

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá detectar obstáculos situados hasta 220 m del radar. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L4) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Detección de un objeto de 0,6 m² a 220 m con probabilidad ≥ [PENDIENTE: 95 %]. |
| Estado | Borrador |

### SYS-DET-02 — Tamaño mínimo del objeto detectable

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá detectar objetos de tamaño mayor o igual a 0,6 m². |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L5) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Detección de objetos de 0,6 m² [PENDIENTE: ¿área física o sección radar?] dentro del alcance. |
| Estado | Borrador |

### SYS-DET-03 — Probabilidad de detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá alcanzar una probabilidad de detección superior a [PENDIENTE: 95 %]. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L36) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Probabilidad de detección > [PENDIENTE: 95 %] sobre [PENDIENTE: N] casos de prueba en las condiciones de [PENDIENTE: entorno]. |
| Estado | Borrador |

### SYS-DET-04 — Tasa de falsas alarmas

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener la tasa de falsas alarmas por debajo de [PENDIENTE: X %]. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L38) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tasa de falsas alarmas < [PENDIENTE: X %] sobre [PENDIENTE: N] ciclos con vía libre. |
| Estado | Borrador |

### SYS-DET-05 — Exclusión de objetos fuera de la vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá no reportar objetos situados fuera de la vía. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L37) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Objetos a más de [PENDIENTE: X m] del eje de vía no se reportan en el 100 % de los casos de prueba. |
| Estado | Borrador |

### SYS-DET-06 — Error de localización

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá localizar cada objeto con un error menor que [PENDIENTE: valor]. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L40) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Error de localización < [PENDIENTE: X m] en todo el rango hasta 220 m. |
| Estado | Borrador |

### SYS-DET-07 — Error angular de la desviación del radar

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá calcular la desviación angular del radar respecto al eje del tranvía con un error máximo de 1°. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L25-L26) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Error angular ≤ 1° [PENDIENTE: confirmar que es requisito y no solo hipótesis]. |
| Estado | Borrador |


## Salida

### SYS-OUT-01 — Posición del objeto en coordenadas del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la posición del objeto en coordenadas del tranvía. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L39) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye posición en coordenadas del tranvía [PENDIENTE: formato y unidad]. |
| Estado | Borrador |

### SYS-OUT-02 — Tamaño del objeto

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, el tamaño del objeto. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L39) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye tamaño en [PENDIENTE: m²]. |
| Estado | Borrador |

### SYS-OUT-03 — Incertidumbre de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la incertidumbre de la posición. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L39) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye incertidumbre en [PENDIENTE: m, 1σ u otro criterio]. |
| Estado | Borrador |

### SYS-OUT-04 — Marca de tiempo de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, una marca de tiempo (timestamp). |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L41) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones incluye timestamp de resolución ≤ [PENDIENTE: 1 ms]. |
| Estado | Borrador |

### SYS-OUT-05 — Alcance estimado al objeto

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, el alcance estimado al objeto. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L41) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones incluye alcance con error ≤ [PENDIENTE: X m]. |
| Estado | Borrador |

### SYS-OUT-06 — Publicación del estado ocupado, libre o desconocido

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá publicar el estado ocupado, libre o desconocido en cada ciclo. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L31) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una publicación por cada ciclo de 50 ms [PENDIENTE: interfaz y formato de salida]. |
| Estado | Borrador |

### SYS-OUT-07 — Aviso MQTT de modo degradado o fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema entra en modo degradado o en modo fallo, entonces el sistema deberá publicar un mensaje MQTT de estado y diagnóstico con el modo actual. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L15); respuesta del usuario a Q-005 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Mensaje publicado en ≤ [PENDIENTE: X ms] tras el cambio de modo en el topic [PENDIENTE: topic según STD-MQTT]. |
| Estado | Borrador |

### SYS-OUT-08 — Confianza de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la confianza de la detección. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L14, L47); respuesta del usuario a Q-002 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye confianza en [PENDIENTE: escala]. |
| Estado | Borrador |

### SYS-OUT-09 — Registro CRIT del cambio a modo degradado o fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema entra en modo degradado o en modo fallo, entonces el sistema deberá registrar un log de nivel CRIT. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L15); respuesta del usuario a Q-005 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una entrada de log CRIT por cada cambio de modo, en ≤ [PENDIENTE: X ms] tras el cambio. |
| Estado | Borrador |


## Tiempo

### SYS-T-01 — Tiempo máximo de entrega de objetos detectados

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar los objetos detectados en la vía (tamaño, posición y confianza) en un tiempo máximo de 128 ms [PENDIENTE: instante de inicio del plazo]. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L14); respuesta del usuario a Q-002 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tiempo de entrega ≤ 128 ms en el percentil [PENDIENTE: 99] con [PENDIENTE: N] puntos por nube. |
| Estado | Borrador |

## Seguridad y modos de funcionamiento

### SYS-SAF-01 — Modos de funcionamiento

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá disponer de tres modos de funcionamiento: normal, degradado y fallo. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L45) |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | El sistema se encuentra en exactamente 1 de los 3 modos en el 100 % de los ciclos. |
| Estado | Borrador |

### SYS-SAF-04 — Reset de degradado a normal desde el HMI

| Campo | Valor |
|---|---|
| Descripción | Si se recibe desde el HMI el reset de degradado estando en modo degradado, entonces el sistema deberá pasar a modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L46); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Transición a normal en ≤ [PENDIENTE: X ms] tras el reset de degradado recibido; el reset de fallo no produce la transición. |
| Estado | Borrador |

### SYS-SAF-05 — Confianza reducida en modo degradado

| Campo | Valor |
|---|---|
| Descripción | Si el sistema está en modo degradado, entonces el sistema deberá seguir emitiendo alertas con confianza reducida, incluida la salida de ausencia de obstáculo. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L47); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | En modo degradado, el 100 % de las salidas de cada ciclo de 50 ms lleva confianza reducida [PENDIENTE: valor o escala de confianza]. |
| Estado | Borrador |

### SYS-SAF-06 — No declarar vía libre en modo fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema está en modo fallo, entonces el sistema deberá indicar que no hay vigilancia y no declarar la vía libre. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L48) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | En modo fallo, 0 salidas por ciclo de 50 ms «vía libre» y el 100 % de los ciclos indica «sin vigilancia». |
| Estado | Borrador |

### SYS-SAF-07 — Estado desconocido ante falta de evidencia [PROPUESTA]

| Campo | Valor |
|---|---|
| Descripción | Si no hay evidencia para determinar el estado de la vía, entonces el sistema deberá declarar el estado desconocido y no libre. |
| Tipo | Funcional |
| Origen | Derivado de seguridad-funcional.md (SAF-01) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 salidas «libre» sin evidencia por ciclo de 50 ms en el 100 % de los casos de inyección de fallo. |
| Estado | Borrador |

### SYS-SAF-08 — Fallo por ausencia de tramas

| Campo | Valor |
|---|---|
| Descripción | Si el sistema no recibe ninguna trama del radar en 128 ms, entonces el sistema deberá pasar a modo fallo desde el modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L51); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Detección de la ausencia de tramas y paso a fallo en ≤ 128 ms. |
| Estado | Borrador |

### SYS-SAF-09 — Fallo por timestamp incoherente

| Campo | Valor |
|---|---|
| Descripción | Si el timestamp de los datos recogidos no avanza o retrocede, entonces el sistema deberá pasar a modo fallo desde el modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L52); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo en ≤ 1 ciclo de 50 ms ante timestamp repetido o decreciente. |
| Estado | Borrador |

### SYS-SAF-10 — Fallo por dato de entrada no disponible

| Campo | Valor |
|---|---|
| Descripción | Si uno de los datos de entrada, como la velocidad, no está disponible, entonces el sistema deberá pasar a modo fallo desde el modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L53); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo en ≤ 1 ciclo de 50 ms por cada dato de entrada indisponible. |
| Estado | Borrador |

### SYS-SAF-11 — Fallo por autodiagnóstico del radar

| Campo | Valor |
|---|---|
| Descripción | Si el autodiagnóstico del radar notifica un fallo, entonces el sistema deberá pasar a modo fallo desde el modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L54); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo en ≤ 1 ciclo de 50 ms tras la notificación. |
| Estado | Borrador |

### SYS-SAF-12 — Degradado por ausencia de eco de raíles

| Campo | Valor |
|---|---|
| Descripción | Si el sistema no detecta nada, ni siquiera el eco de los raíles, en 3 mediciones consecutivas estando en modo normal, entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L55); respuesta del usuario a Q-003 y Q-004 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a degradado en la 3.ª medición consecutiva (150 ms con periodo de 50 ms) sin ningún eco, y no antes. |
| Estado | Borrador |

### SYS-SAF-13 — Degradado por desalineación del radar

| Campo | Valor |
|---|---|
| Descripción | Si se detecta desalineación del radar estando en modo normal, entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Condiciones de fallo, L56); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a degradado en ≤ [PENDIENTE: X s] con desalineación > [PENDIENTE: 1 °]. |
| Estado | Borrador |

### SYS-SAF-14 — Degradado continuado a fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema permanece en modo degradado de forma continuada durante más de [PENDIENTE: X s], entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L49); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo tras [PENDIENTE: X s] continuos en modo degradado. |
| Estado | Borrador |

### SYS-SAF-15 — Funcionamiento en modo normal

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá cumplir en modo normal todos los requisitos de este documento. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L44-L45); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 incumplimientos de los requisitos en 1 h de ejecución en modo normal. |
| Estado | Borrador |

### SYS-SAF-16 — Reset de fallo a normal desde el HMI

| Campo | Valor |
|---|---|
| Descripción | Si se recibe desde el HMI el reset de fallo, protegido con [PENDIENTE: nivel de seguridad del reset], estando en modo fallo, entonces el sistema deberá pasar a modo normal. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L44); respuesta del usuario a Q-003 y respuesta a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Transición a normal en ≤ [PENDIENTE: X ms] solo con el reset de fallo válido; con el reset de degradado permanece en fallo. |
| Estado | Borrador |

### SYS-SAF-17 — Funcionamiento normal tras el reset de degradado

| Campo | Valor |
|---|---|
| Descripción | Si se ejecuta el reset de degradado, entonces el sistema deberá funcionar en condiciones normales durante [PENDIENTE: 2 min]. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Modos, L46); respuesta del usuario a Q-003 y respuesta a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tras el reset, 0 transiciones a degradado o fallo durante [PENDIENTE: 2 min] con entradas nominales. |
| Estado | Borrador |


## Implementación

### SYS-IMP-01 — Sin memoria dinámica

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá evitar el uso de memoria dinámica tras la inicialización [PENDIENTE: confirmar «tras la inicialización» o «nunca»]. |
| Tipo | Restricción |
| Origen | Derivado de requisitos.txt (Tranvía, L43) |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | 0 llamadas a funciones de asignación dinámica de memoria en el código (análisis estático). |
| Estado | Borrador |

### SYS-IMP-02 — Comportamiento determinista

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá tener un comportamiento determinista: mismas entradas producen mismas salidas en un tiempo acotado. |
| Tipo | No funcional |
| Origen | Derivado de requisitos.txt (Tranvía, L43) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | 100 % de salidas idénticas en 1000 repeticiones de la misma secuencia de entradas y tiempo de ciclo máximo ≤ 128 ms. |
| Estado | Borrador |


### SYS-IMP-03 — Procesamiento a bordo del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá ejecutar el procesamiento a bordo del tranvía. |
| Tipo | Restricción |
| Origen | Derivado de requisitos.txt (Requisitos, L13); respuesta del usuario a Q-006 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | El 100 % del procesamiento se ejecuta en el equipo de a bordo. |
| Estado | Borrador |

## Hipótesis

### H-01 — Ancho de vía de 1 m

| Campo | Valor |
|---|---|
| Descripción | El ancho de vía es de 1 m. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L7) |
| Estado | Borrador |

### H-02 — Giro constante

| Campo | Valor |
|---|---|
| Descripción | El tranvía mantiene un giro constante durante [PENDIENTE: intervalo de análisis, ver Q-018]. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L23) |
| Estado | Borrador |

### H-03 — Sincronización de velocidad y giro

| Campo | Valor |
|---|---|
| Descripción | La velocidad y el giro llegan sincronizados con la nube de puntos [PENDIENTE: tolerancia]. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L24) |
| Estado | Borrador |

### H-04 — Radar montado y calibrado con el eje del tranvía

| Campo | Valor |
|---|---|
| Descripción | El radar está montado y calibrado con el eje del tranvía. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L25) |
| Estado | Borrador |

### H-05 — Radar de medida 2D

| Campo | Valor |
|---|---|
| Descripción | El radar mide en 2D y no mide altura. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L20-L21) |
| Estado | Borrador |

### H-06 — Error creciente con la distancia

| Campo | Valor |
|---|---|
| Descripción | El error de medida del radar crece con la distancia. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L22) |
| Estado | Borrador |

### H-07 — Velocidad máxima del tranvía [PROPUESTA]

| Campo | Valor |
|---|---|
| Descripción | La velocidad máxima del tranvía es [PENDIENTE: v_max]. |
| Tipo | Hipótesis |
| Origen | Derivado de requisitos.txt (Requisitos, L28) |
| Estado | Borrador |


## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-01 | Alicia Garcia | Creación a partir de la plantilla Requirements_2 |
| 0.2 | 2026-11-01 | Alicia Garcia | Ronda 1 y 2 de dudas: modos y transiciones (SYS-SAF-04, 05, 09, 12 a 17), HMI, SYS-OUT-07 a 09, SYS-T-01, SYS-IMP-03, H-01 y H-02 confirmadas. Códigos SYS-SAF-02 y SYS-SAF-03 retirados (absorbidos por SYS-SAF-12, 13 y 14), no se reutilizan |
