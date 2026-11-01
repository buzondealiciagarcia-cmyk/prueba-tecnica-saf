# Requisitos del sistema de detección de obstáculos en vía, estáticos y en movimiento (radar frontal)

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-REQ |
| Versión | 0.10 |
| Fecha | 2026-11-01 |
| Autor | Alicia Garcia |
| Estado | Aprobado |

Redacción según ISO/IEC/IEEE 29148:2018, con plantillas EARS: «El sistema deberá…» para requisitos siempre activos y «Si…, entonces el sistema deberá…» para la respuesta ante fallos.

Codificación: SYS-\<categoría\>-\<nn\> para requisitos del sistema y H-\<nn\> para hipótesis. Categorías: DET = detección y prestaciones; IN = entradas; PR = procesamiento; OUT = salida; T = tiempo; SAF = seguridad y modos de funcionamiento; GEN = normativa; IMP = implementación.

Source: R1–R4 son los requisitos del enunciado aportado por el usuario (`entradas/enunciado.txt`). «Derivado de requisitos.txt (sección, línea)» cita las notas del usuario (`entradas/requisitos.txt`); «Derivado» indica un requisito o hipótesis necesario para el diseño que no aparece explícitamente en el enunciado. Los valores entre corchetes \[X\] están pendientes de definir.

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
| Descripción | El sistema deberá cumplir los requisitos de seguridad funcional de las normas EN 50126, EN 50128 y EN 50129 con un nivel de integridad objetivo SIL 2, aplicados como práctica de simulación sin reclamar certificación. |
| Tipo | Restricción |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L19); respuesta del usuario a Q-015 |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Informe de aplicación de EN 50126, EN 50128 y EN 50129 con SIL 2 objetivo, sin desviaciones abiertas en el 100 % de los requisitos SYS-SAF. |
| Estado | Aprobado |

### SYS-GEN-02 — Documentación del entorno operativo

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá tener documentado su entorno operativo nominal. |
| Tipo | Restricción |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L19); respuesta del usuario a Q-022 |
| Prioridad | Should |
| Método de verificación | Inspección |
| Criterio de aceptación | El entorno operativo nominal figura en el documento de diseño. |
| Estado | Aprobado |

## Entradas

### SYS-IN-01 — Recepción de la nube de puntos del radar

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir del radar una nube de puntos con un periodo de 50 ms. |
| Tipo | Funcional |
| Origen | Derivado de requisitos.txt (Requisitos, L6) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se reciben nubes de puntos con periodo de 50 ms (tolerancia fijada en diseño) durante la duración de ejecución definida en la fase de diseño. |
| Estado | Aprobado |

### SYS-IN-02 — Recepción de la velocidad del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir la velocidad del tranvía con el mismo periodo de 50 ms de la nube de puntos y sincronizada con ella. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L27-L29); respuesta del usuario a Q-016 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se recibe la velocidad con periodo de 50 ms sincronizada con cada nube de puntos, con la resolución fijada en diseño. |
| Estado | Aprobado |

### SYS-IN-03 — Recepción de la velocidad de giro del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir la velocidad de giro (ω) del tranvía con el mismo periodo de 50 ms de la nube de puntos y sincronizada con ella. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L9, L27); respuesta del usuario a Q-016 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Se recibe ω con periodo de 50 ms sincronizada con cada nube de puntos, con la resolución fijada en diseño. |
| Estado | Aprobado |

### SYS-IN-04 — Recepción de la marca de tiempo de cada trama

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá recibir una marca de tiempo (timestamp) asociada a cada trama del radar. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L41; Condiciones de fallo, L52) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las tramas procesadas durante la duración de ejecución definida en la fase de diseño incluye timestamp. |
| Estado | Borrador |

### SYS-IN-05 — Altura de montaje del radar como parámetro

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá disponer de la altura de montaje del radar como parámetro de configuración. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L18); respuesta del usuario a Q-009 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | La altura de montaje figura como parámetro en m en el fichero de configuración (STD-CFG), con el valor fijado en diseño. |
| Estado | Aprobado |

### SYS-IN-06 — Validación del timestamp del radar

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá validar el timestamp del radar contra el reloj del sistema. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L41); respuesta del usuario a Q-026 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Un timestamp del radar incoherente con el reloj del sistema se detecta en el 100 % de los casos de prueba; resolución y tolerancia fijadas en diseño. |
| Estado | Aprobado |

### SYS-IN-07 — Validación de cada trama

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá validar cada trama recibida del radar antes de procesarla. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L51-L54); respuesta del usuario a Q-023 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las tramas inválidas de prueba se detecta y no se procesa; criterio de validez y contador de fallos consecutivos fijados en diseño (FMEA). |
| Estado | Aprobado |

## Procesamiento

### SYS-PR-01 — Coordenadas respecto a la vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá expresar las coordenadas de las detecciones del radar tomando la vía como referencia. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L17) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una detección de posición conocida se expresa en coordenadas de vía con el error de localización de SYS-DET-06. |
| Estado | Aprobado |

### SYS-PR-02 — Compensación del movimiento del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá compensar el movimiento del tranvía en las detecciones del radar. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L30) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Un objeto fijo se mantiene en la misma posición de vía con desviación dentro del error de localización de SYS-DET-06, con el tranvía a velocidades entre 0 y la velocidad máxima de H-07. |
| Estado | Aprobado |

### SYS-PR-03 — Filtrado del eco de los raíles

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá filtrar el eco de los raíles mediante un filtro temporal basado en la persistencia del eco. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L32); respuesta del usuario a Q-010 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | En pruebas sin obstáculos en la vía, los ecos de raíl reportados como obstáculo no superan el máximo de falsas alarmas de SYS-DET-04; parámetros del filtro fijados en diseño. |
| Estado | Aprobado |

### SYS-PR-04 — Agrupación de celdas en objetos

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá agrupar las celdas detectadas en un objeto solo si son contiguas en la grid según la vecindad de 8 (lados y diagonales), y no agrupar celdas separadas por un hueco. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L35) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Dos grupos de celdas detectadas separados por al menos 1 celda vacía que rompe la vecindad de 8 producen 2 objetos, y un grupo de celdas contiguas (también en diagonal) produce 1 objeto, en el 100 % de los casos de prueba. |
| Estado | Aprobado |

### SYS-PR-05 — Clasificación de objeto en movimiento o estático

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá clasificar cada objeto detectado como estático o en movimiento comparando su velocidad respecto a la vía, con compensación del movimiento del tranvía, con un umbral fijado en diseño. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L8); respuesta del usuario a Q-001; respuesta del usuario a Q-029 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Clasificación correcta de los objetos de prueba estáticos y en movimiento con el umbral y la histéresis fijados en diseño (STD-CFG). |
| Estado | Aprobado |

### SYS-PR-06 — Cálculo del ángulo de giro a partir de ω

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá calcular el ángulo de giro del tranvía a partir de la velocidad de giro (ω). |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L9-L10) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Error del ángulo calculado dentro del error angular de SYS-DET-07 en curva de radio fijado en diseño. |
| Estado | Aprobado |

### SYS-PR-07 — Posición con la altura del radar 3D

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá tener en cuenta la altura de montaje del radar y la altura medida de cada eco al calcular la posición del objeto en coordenadas de vía. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L18); respuesta del usuario a Q-009 |
| Prioridad | Should |
| Método de verificación | Test |
| Criterio de aceptación | Objetos de prueba a alturas distintas se posicionan con el error de SYS-DET-06. |
| Estado | Aprobado |

### SYS-PR-08 — Probabilidad interna de ocupación y estado de la celda

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener para cada celda de la vía una probabilidad numérica de ocupación interna y derivar de ella únicamente el estado de la celda como ocupado, probabilidad baja de obstáculo o zona no estudiada, sin derivar nunca un estado libre. |
| Tipo | Funcional |
| Origen | R2, R3, Derivado de requisitos.txt (Requisitos, L31); respuesta del usuario a Q-037; respuesta del usuario a Q-021 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | La probabilidad de ocupación de cada celda se actualiza en cada ciclo de 50 ms; el estado derivado toma uno de tres valores (ocupado, probabilidad baja de obstáculo, zona no estudiada) y 0 estados «libre» en el 100 % de los ciclos. |
| Estado | Aprobado |

### SYS-PR-09 — Confirmación del obstáculo antes del aviso

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá confirmar un obstáculo en N ciclos consecutivos antes de emitir el aviso, con N fijado en diseño. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L42) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El aviso se emite en el ciclo N de confirmación (N calibrable, STD-CFG) y no antes. |
| Estado | Aprobado |

### SYS-PR-10 — Rechazo de datos con velocidad del tranvía negativa

| Campo | Valor |
|---|---|
| Descripción | Si la velocidad del tranvía es menor que 0, entonces el sistema deberá rechazar la detección. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L29); respuesta del usuario a Q-017 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Con velocidad < 0 m/s se rechaza el 100 % de las detecciones de prueba. |
| Estado | Aprobado |

### SYS-PR-11 — Rechazo de detecciones a más de 220 m

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá rechazar las detecciones situadas a más de 220 m del radar medidos a lo largo de la vía. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L29); respuesta del usuario a Q-028; respuesta del usuario a Q-017 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Ninguna detección a > 220 m a lo largo de la vía aparece en la salida y el modo no cambia en el conjunto de pruebas. |
| Estado | Aprobado |

## Detección y prestaciones

### SYS-DET-01 — Alcance de detección de 220 m a lo largo de la vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá detectar obstáculos situados hasta 220 m del radar medidos a lo largo de la vía. |
| Tipo | Funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Requisitos, L4); respuesta del usuario a Q-028 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Detección de un objeto de 0,6 m × 0,6 m × 0,6 m situado a 220 m a lo largo de la vía. |
| Estado | Aprobado |

### SYS-DET-02 — Tamaño mínimo del objeto que genera alerta

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá alertar de un objeto detectado sobre la vía cuyas tres dimensiones sean mayores o iguales a 0,6 m × 0,6 m × 0,6 m. |
| Tipo | Funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Requisitos, L5); respuesta del usuario a Q-008, Q-011 y Q-040 (sustituye «≥ 0,6 m²» de requisitos.txt) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Alerta ante un objeto de 0,6 m × 0,6 m × 0,6 m dentro del ancho de la vía y del alcance de 220 m. |
| Estado | Aprobado |

### SYS-DET-04 — Número máximo de falsas alarmas

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá mantener el número de falsas alarmas por debajo del máximo definido en la fase de diseño (parámetro calibrable, STD-CFG). |
| Tipo | No funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Tranvía, L38); respuesta del usuario a Q-012 y Q-041 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Falsas alarmas por debajo del máximo (parámetro calibrable, STD-CFG) en pruebas sin obstáculos en la vía. |
| Estado | Aprobado |

### SYS-DET-05 — Exclusión de objetos fuera de la vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá no reportar objetos situados fuera del ancho de la vía. |
| Tipo | Funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Tranvía, L37); respuesta del usuario a Q-008 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 objetos reportados fuera del ancho de la vía (H-01) en el 100 % de los casos de prueba. |
| Estado | Aprobado |

### SYS-DET-06 — Error de localización

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá localizar cada objeto con un error menor que el valor fijado en diseño. |
| Tipo | No funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Tranvía, L40) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Error de localización menor que el valor de diseño en todo el rango hasta 220 m a lo largo de la vía. |
| Estado | Aprobado |

### SYS-DET-07 — Error angular de la desviación del radar

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá calcular la desviación angular del radar respecto al eje del tranvía con un error máximo definido en la fase de diseño (parámetro calibrable, STD-CFG). |
| Tipo | No funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Requisitos, L25-L26); respuesta del usuario a Q-019 |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | Error angular dentro del máximo definido en diseño. |
| Estado | Aprobado |


### SYS-DET-08 — Ignorar objetos menores que el tamaño mínimo

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá ignorar las detecciones de objetos con alguna dimensión menor de 0,6 m. |
| Tipo | Funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Requisitos, L5); respuesta del usuario a Q-040 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 alertas por un objeto de 0,5 m × 0,5 m × 0,5 m dentro del ancho de la vía en el 100 % de los casos de prueba. |
| Estado | Aprobado |

## Salida

### SYS-OUT-01 — Posición del objeto en coordenadas de vía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la posición del objeto en coordenadas de vía (longitudinal y lateral) en m. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L39); respuesta del usuario a Q-025 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye posición longitudinal y lateral en m; formato detallado fijado en diseño. |
| Estado | Aprobado |

### SYS-OUT-02 — Tamaño del objeto

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, el tamaño del objeto. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L39) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye las dimensiones del objeto (largo, ancho y alto) en m. |
| Estado | Aprobado |

### SYS-OUT-03 — Incertidumbre de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la incertidumbre de la posición. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L39) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye incertidumbre; criterio de incertidumbre fijado en diseño. |
| Estado | Aprobado |

### SYS-OUT-04 — Marca de tiempo de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, una marca de tiempo (timestamp). |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L41); respuesta del usuario a Q-026 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones incluye timestamp del radar validado con el reloj del sistema; resolución fijada en diseño. |
| Estado | Aprobado |

### SYS-OUT-05 — Alcance estimado al objeto

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, el alcance estimado al objeto. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L41) |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones incluye alcance con el error de localización de SYS-DET-06. |
| Estado | Aprobado |

### SYS-OUT-06 — Publicación del estado de la vía por celdas

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá publicar el estado de la vía por celdas como ocupado, probabilidad baja de obstáculo o zona no estudiada en cada ciclo, sin publicar la probabilidad numérica de ocupación. |
| Tipo | Funcional |
| Origen | R2, R3, Derivado de requisitos.txt (Requisitos, L31); respuesta del usuario a Q-037 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una publicación por cada ciclo de 50 ms con uno de los tres valores para cada celda y 0 publicaciones de la probabilidad numérica. |
| Estado | Aprobado |

### SYS-OUT-07 — Aviso MQTT de modo degradado o fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema entra en modo degradado o en modo fallo, entonces el sistema deberá publicar un mensaje MQTT de estado y diagnóstico con el modo actual. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L15); respuesta del usuario a Q-005 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Un mensaje MQTT con el modo actual por cada cambio a degradado o a fallo. |
| Estado | Aprobado |

### SYS-OUT-08 — Confianza de la detección

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, la confianza de la detección. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L14, L47); respuesta del usuario a Q-002 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones de cada ciclo de 50 ms incluye confianza; escala fijada en diseño. |
| Estado | Aprobado |

### SYS-OUT-09 — Registro CRIT del cambio a modo degradado o fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema entra en modo degradado o en modo fallo, entonces el sistema deberá registrar un log de nivel CRIT. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L15); respuesta del usuario a Q-005 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una entrada de log CRIT por cada cambio a degradado o a fallo. |
| Estado | Aprobado |

### SYS-OUT-10 — Identificador del objeto

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar, por cada detección, un identificador del objeto. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L39); respuesta del usuario a Q-025 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | El 100 % de las detecciones incluye un identificador de objeto. |
| Estado | Aprobado |

### SYS-OUT-11 — Publicación de la lista de objetos detectados

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá publicar en cada ciclo la lista de objetos detectados, con los datos de SYS-OUT-01 a SYS-OUT-05, SYS-OUT-08 y SYS-OUT-10 por cada objeto. |
| Tipo | Funcional |
| Origen | R2, Derivado de respuesta del usuario a Q-045 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Una publicación por cada ciclo de 50 ms con la lista de objetos detectados, y el 100 % de los objetos incluye todos los datos de SYS-OUT-01 a SYS-OUT-05, SYS-OUT-08 y SYS-OUT-10. |
| Estado | Aprobado |

## Tiempo

### SYS-T-01 — Tiempo máximo de entrega de objetos detectados

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá entregar los objetos detectados en la vía (tamaño, posición y confianza) en un tiempo máximo de 128 ms desde la recepción de la nube de puntos del radar. |
| Tipo | No funcional |
| Origen | R1, R2, Derivado de requisitos.txt (Requisitos, L14); respuesta del usuario a Q-002; respuesta del usuario a Q-030 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tiempo de entrega ≤ 128 ms desde la recepción de la nube de puntos; percentil, carga y solape de ciclos fijados en diseño. |
| Estado | Aprobado |

## Seguridad y modos de funcionamiento

### SYS-SAF-01 — Modos de funcionamiento

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá disponer de tres modos de funcionamiento: normal, degradado y fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L45) |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | El sistema se encuentra en exactamente 1 de los 3 modos en el 100 % de los ciclos. |
| Estado | Aprobado |

### SYS-SAF-04 — Recuperación de degradado a normal mediante reset

| Campo | Valor |
|---|---|
| Descripción | Si se recibe el reset de degradado estando en modo degradado, entonces el sistema deberá pasar a modo normal. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L46); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tras el reset de degradado el modo es normal; el reset de fallo no produce esa transición. |
| Estado | Aprobado |

### SYS-SAF-05 — Confianza reducida en modo degradado

| Campo | Valor |
|---|---|
| Descripción | Si el sistema está en modo degradado, entonces el sistema deberá seguir emitiendo alertas con confianza reducida, sobre todo en los casos en que no se detecta obstáculo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L47); respuesta del usuario a Q-003 y Q-037 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | En modo degradado, el 100 % de las salidas de cada ciclo de 50 ms lleva confianza reducida; escala fijada en diseño. |
| Estado | Aprobado |

### SYS-SAF-06 — No dar vía libre en modo fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema está en modo fallo, entonces el sistema deberá no entregar nada como vía libre y comunicar que no hay vigilancia. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L48); respuesta del usuario a Q-037 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | En modo fallo, 0 salidas de vía libre o de ausencia de obstáculo por ciclo de 50 ms y el 100 % de los ciclos comunica «sin vigilancia». |
| Estado | Aprobado |

### SYS-SAF-07 — Nunca declarar ausencia de obstáculo

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá informar de la vía solo como ocupada, con probabilidad baja de obstáculo o como zona no estudiada, y no declarar nunca la ausencia de obstáculo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L48); respuesta del usuario a Q-037 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 salidas de «no hay obstáculo» o «vía libre» por ciclo de 50 ms en el 100 % de los casos de prueba, incluidos los de inyección de fallo. |
| Estado | Aprobado |

### SYS-SAF-08 — Fallo por ausencia de tramas

| Campo | Valor |
|---|---|
| Descripción | Si el sistema no recibe ninguna trama del radar durante el tiempo máximo definido en la fase de diseño (parámetro calibrable, STD-CFG), entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L51); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo cuando transcurre sin trama el tiempo máximo definido en diseño. |
| Estado | Aprobado |

### SYS-SAF-09 — Fallo por timestamp incoherente

| Campo | Valor |
|---|---|
| Descripción | Si el timestamp de los datos recogidos no avanza o retrocede, entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L52); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo tras inyectar un timestamp repetido o decreciente. |
| Estado | Aprobado |

### SYS-SAF-10 — Fallo por dato de entrada no disponible

| Campo | Valor |
|---|---|
| Descripción | Si uno de los datos de entrada, como la velocidad, no está disponible, entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L53); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo tras la indisponibilidad de cada dato de entrada. |
| Estado | Aprobado |

### SYS-SAF-11 — Fallo por autodiagnóstico del radar

| Campo | Valor |
|---|---|
| Descripción | Si el autodiagnóstico del radar notifica un fallo, entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L54); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo tras la notificación del autodiagnóstico. |
| Estado | Aprobado |

### SYS-SAF-12 — Degradado por ausencia de eco de raíles

| Campo | Valor |
|---|---|
| Descripción | Si la nube de puntos recibida no contiene ninguna detección, ni siquiera el eco de los raíles, durante el número de mediciones consecutivas definido en la fase de diseño (parámetro calibrable, STD-CFG), entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L55); respuesta del usuario a Q-003 y Q-004 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a degradado tras el número de mediciones consecutivas sin ningún eco definido en diseño y no antes. |
| Estado | Aprobado |

### SYS-SAF-13 — Degradado por desalineación del radar

| Campo | Valor |
|---|---|
| Descripción | Si se detecta una desalineación del radar mayor que el umbral definido en la fase de diseño (parámetro calibrable, STD-CFG), entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Condiciones de fallo, L56); respuesta del usuario a Q-003; respuesta del usuario a Q-019 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a degradado al inyectar una desalineación mayor que el umbral definido en diseño; método de detección fijado en diseño. |
| Estado | Aprobado |

### SYS-SAF-14 — Degradado continuado a fallo

| Campo | Valor |
|---|---|
| Descripción | Si el sistema permanece en modo degradado de forma continuada durante más del tiempo máximo definido en la fase de diseño (parámetro calibrable, STD-CFG), entonces el sistema deberá pasar a modo fallo. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L49); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a fallo cuando el tiempo continuado en degradado supera el límite parametrizado (STD-CFG). |
| Estado | Aprobado |

### SYS-SAF-15 — Funcionamiento en modo normal

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá cumplir en modo normal todos los requisitos de este documento. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L44-L45); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | 0 incumplimientos de los requisitos durante la duración de ejecución definida en la fase de diseño en modo normal. |
| Estado | Borrador |

### SYS-SAF-16 — Recuperación de fallo a normal mediante reset

| Campo | Valor |
|---|---|
| Descripción | Si se recibe el reset de fallo estando en modo fallo, entonces el sistema deberá pasar a modo normal. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Modos, L44); respuesta del usuario a Q-003 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Tras el reset de fallo el modo es normal; el reset de degradado no produce esa transición. |
| Estado | Aprobado |

### SYS-SAF-18 — Degradado por exceso de falsas alarmas

| Campo | Valor |
|---|---|
| Descripción | Si el número de falsas alarmas supera el máximo de SYS-DET-04, entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Tranvía, L38); respuesta del usuario a Q-041 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a modo degradado, y no directamente a fallo, al superar el máximo de falsas alarmas. |
| Estado | Aprobado |

### SYS-SAF-19 — Degradado por velocidad del tranvía negativa

| Campo | Valor |
|---|---|
| Descripción | Si la velocidad del tranvía es menor que 0 (marcha atrás), entonces el sistema deberá pasar a modo degradado. |
| Tipo | Funcional |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L29); respuesta del usuario a Q-017 |
| Prioridad | Must |
| Método de verificación | Test |
| Criterio de aceptación | Paso a modo degradado con velocidad < 0 m/s. |
| Estado | Aprobado |

## Implementación

### SYS-IMP-01 — Sin memoria dinámica

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá evitar el uso de memoria dinámica en todo el software, sin excepciones. |
| Tipo | Restricción |
| Origen | R2, R3, Derivado de requisitos.txt (Tranvía, L43); respuesta del usuario a Q-024 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | 0 llamadas a funciones de asignación dinámica de memoria en el código (análisis estático). |
| Estado | Aprobado |

### SYS-IMP-02 — Comportamiento determinista

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá tener un comportamiento determinista: mismas entradas producen mismas salidas en un tiempo acotado. |
| Tipo | No funcional |
| Origen | R2, R3, Derivado de requisitos.txt (Tranvía, L43) |
| Prioridad | Must |
| Método de verificación | Análisis |
| Criterio de aceptación | 100 % de salidas idénticas en 1000 repeticiones de la misma secuencia de entradas y tiempo de ciclo máximo ≤ 128 ms. |
| Estado | Aprobado |


### SYS-IMP-03 — Procesamiento a bordo del tranvía

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá ejecutar el procesamiento a bordo del tranvía. |
| Tipo | Restricción |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L13); respuesta del usuario a Q-006 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | El 100 % del procesamiento se ejecuta en el equipo de a bordo. |
| Estado | Aprobado |

### SYS-IMP-04 — Nube (cloud) solo para trazabilidad y logs

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá usar la nube (cloud) únicamente para enviar datos de trazabilidad y logs. |
| Tipo | Restricción |
| Origen | Derivado de respuesta del usuario (ronda 5, nube en este proyecto) |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Ningún procesamiento del sistema depende de datos recibidos de la nube; las únicas interacciones con la nube son envíos de trazabilidad y logs. |
| Estado | Aprobado |

### SYS-IMP-06 — Plan de viabilidad del radar [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado de un plan de viabilidad, entregado en la fase de diseño como documento aparte, del uso del radar para detectar obstáculos en la vía, con las capacidades y limitaciones del sensor para el caso de uso de requisitos.txt. |
| Tipo | Restricción |
| Origen | R1 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Existe 1 plan de viabilidad, como documento aparte entregado en la fase de diseño, que recoge las capacidades y las limitaciones del radar para el caso de uso de requisitos.txt. |
| Estado | Aprobado |

### SYS-IMP-07 — Diseño hardware y software del sistema [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado de un diseño del sistema que cubra el hardware y el software. |
| Tipo | Restricción |
| Origen | R2 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Existe 1 documento de diseño con la arquitectura de hardware y la de software. |
| Estado | Aprobado |

### SYS-IMP-08 — Diagrama pipeline de un ciclo [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado de un diagrama pipeline de un ciclo, desde la entrada del radar hasta la lista de objetos. |
| Tipo | Restricción |
| Origen | R2 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | El diseño incluye 1 diagrama pipeline que cubre un ciclo completo desde la entrada del radar hasta la lista de objetos. |
| Estado | Aprobado |

### SYS-IMP-10 — Pseudocódigo de un componente [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado del (pseudo) código de un componente del sistema, por ejemplo la actualización de la probabilidad interna de ocupación de una celda del grid, sin derivar nunca un estado libre. |
| Tipo | Restricción |
| Origen | R3 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Existe el (pseudo) código de 1 componente del sistema, trazado a los requisitos que cubre. |
| Estado | Aprobado |

### SYS-IMP-11 — Justificación del lenguaje C [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado de la justificación de la elección de C (C11 con MISRA, determinismo y sin memoria dinámica) frente a lenguajes alternativos. |
| Tipo | Restricción |
| Origen | R3 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Existe 1 justificación escrita de C (C11 con MISRA, determinismo y sin memoria dinámica) frente a al menos 1 lenguaje alternativo. |
| Estado | Aprobado |

### SYS-IMP-12 — Borrador de oferta al cliente [ENTREGABLE]

| Campo | Valor |
|---|---|
| Descripción | El sistema deberá estar acompañado de un borrador de oferta al cliente con el alcance, el plazo y el presupuesto, en dos fases: Fase A (viabilidad, diseño y pseudocódigo) y Fase B (implementación completa del sistema). |
| Tipo | Restricción |
| Origen | R4 |
| Prioridad | Must |
| Método de verificación | Inspección |
| Criterio de aceptación | Existe 1 borrador de oferta con alcance, plazo y presupuesto de las 2 fases (A y B); plazo, presupuesto, tarifas y margen los aporta el usuario, sin cifras inventadas. |
| Estado | Aprobado |

## Hipótesis

### H-01 — Ancho de vía de 1 m

| Campo | Valor |
|---|---|
| Descripción | El ancho de vía es de 1 m. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L7) |
| Estado | Aprobado |

### H-02 — Giro constante

| Campo | Valor |
|---|---|
| Descripción | El tranvía mantiene un giro constante durante el horizonte de análisis fijado en diseño. |
| Tipo | Hipótesis |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L23) |
| Estado | Aprobado |

### H-03 — Sincronización de velocidad y giro

| Campo | Valor |
|---|---|
| Descripción | La velocidad y el giro llegan con el mismo periodo de 50 ms, sincronizados con la nube de puntos. |
| Tipo | Hipótesis |
| Origen | R2, Derivado de requisitos.txt (Requisitos, L24) |
| Estado | Aprobado |

### H-04 — Radar montado y calibrado con el eje del tranvía

| Campo | Valor |
|---|---|
| Descripción | El radar está montado y calibrado con el eje del tranvía. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L25) |
| Estado | Aprobado |

### H-05 — Radar 3D

| Campo | Valor |
|---|---|
| Descripción | El radar es 3D y mide distancia, azimut y altura. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L20-L21); respuesta del usuario a Q-009 (sustituye la nota «2D, no detecta la altura» de requisitos.txt) |
| Estado | Aprobado |

### H-06 — Error creciente con la distancia

| Campo | Valor |
|---|---|
| Descripción | El error de medida del radar en azimut y altura crece con la distancia. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L22) |
| Estado | Aprobado |

### H-07 — Velocidad máxima del tranvía

| Campo | Valor |
|---|---|
| Descripción | La velocidad máxima del tranvía se fija en diseño. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L28) |
| Estado | Aprobado |

### H-08 — Campo de visión del sistema

| Campo | Valor |
|---|---|
| Descripción | El campo de visión del sistema es todo lo que alcanza a detectar el radar y de lo que luego entrega una nube de puntos. |
| Tipo | Hipótesis |
| Origen | R1, Derivado de requisitos.txt (Requisitos, L4); respuesta del usuario a Q-008 |
| Estado | Aprobado |

## Trazabilidad con el enunciado

El campo Origen combina R1–R4 (puntos del enunciado, en `entradas/enunciado.txt`) con «Derivado» (notas de `entradas/requisitos.txt` o decisiones del usuario). Esta tabla se deriva de ese campo. SYS-IMP-06, 07, 08, 10, 11 y 12 son los entregables que el enunciado pide y no están cubiertos por el resto de requisitos.

| Enunciado | Requisitos de entregable | Requisitos de comportamiento y restricción relacionados |
|---|---|---|
| R1 | SYS-IMP-06 | SYS-IN-01, SYS-GEN-02, SYS-DET-01, SYS-DET-02, SYS-DET-04, SYS-DET-05, SYS-DET-06, SYS-DET-07, SYS-DET-08, SYS-T-01, H-01, H-04, H-05, H-06, H-07, H-08 |
| R2 | SYS-IMP-07, SYS-IMP-08 | SYS-IN-01, SYS-GEN-01, SYS-IN-02, SYS-IN-03, SYS-IN-04, SYS-IN-05, SYS-IN-06, SYS-IN-07, SYS-PR-01, SYS-PR-02, SYS-PR-03, SYS-PR-04, SYS-PR-05, SYS-PR-06, SYS-PR-07, SYS-PR-08, SYS-PR-09, SYS-PR-10, SYS-PR-11, SYS-DET-01, SYS-DET-02, SYS-DET-04, SYS-DET-05, SYS-DET-06, SYS-DET-07, SYS-DET-08, SYS-OUT-01, SYS-OUT-02, SYS-OUT-03, SYS-OUT-04, SYS-OUT-05, SYS-OUT-06, SYS-OUT-07, SYS-OUT-08, SYS-OUT-09, SYS-OUT-10, SYS-OUT-11, SYS-T-01, SYS-SAF-01, SYS-SAF-04, SYS-SAF-05, SYS-SAF-06, SYS-SAF-07, SYS-SAF-08, SYS-SAF-09, SYS-SAF-10, SYS-SAF-11, SYS-SAF-12, SYS-SAF-13, SYS-SAF-14, SYS-SAF-15, SYS-SAF-16, SYS-SAF-18, SYS-SAF-19, SYS-IMP-01, SYS-IMP-02, SYS-IMP-03, H-02, H-03 |
| R3 | SYS-IMP-10, SYS-IMP-11 | SYS-PR-08, SYS-OUT-06, SYS-IMP-01, SYS-IMP-02 |
| R4 | SYS-IMP-12 |  |

## Decisiones aplazadas a diseño

Por decisión del usuario, estos temas se deciden en `docs/02-diseno/diseno.md` y no son obligaciones de este documento. Los valores numéricos son parámetros calibrables en la configuración (STD-CFG: fichero versionado con esquema, rango y requisito; validación al arrancar).

| Duda | Requisito afectado | Qué hay que decidir en diseño |
|---|---|---|
| Q-031 | SYS-SAF-14 | Tiempo máximo de degradado continuado antes de pasar a fallo (X s). Parámetro calibrable. |
| Q-032 | SYS-SAF-04 (retirado SYS-SAF-17) | Valor y semántica del periodo de funcionamiento normal tras el reset de degradado (e.g. 2 min). Parámetro calibrable. |
| Q-033 | SYS-SAF-04, SYS-SAF-16 | Nivel de seguridad de los resets desde el HMI (autenticación, doble confirmación). |
| Q-034 | SYS-SAF-04, SYS-SAF-16, SYS-OUT-07 | Interfaz con el HMI: protocolo, mensajes y confirmación de los dos resets distintos. |
| Q-051 | SYS-SAF-08 | Tiempo máximo sin trama antes de pasar a fallo. Parámetro calibrable. |
| Q-035 | SYS-SAF-12 | Semántica y valor de «3 mediciones consecutivas» y su relación con las N confirmaciones de SYS-PR-09. Parámetro calibrable. |
| Q-036 | SYS-SAF-08 a SYS-SAF-14 | Transiciones finas: condiciones de fallo estando en degradado, reset con la causa aún presente, otras vías de transición. |
| Q-038 | SYS-PR-08, SYS-OUT-06, SYS-SAF-05 | Nombres, codificación y escala de confianza; nombres de los tres valores del estado de la vía y escala de probabilidad. |
| Q-009 | SYS-IN-05 | Altura de montaje del radar (parámetro de configuración, STD-CFG). |
| Q-012 | SYS-DET-03 (retirado) | Probabilidad de detección: valor, casos de prueba y entorno de medida. |
| Q-041 | SYS-DET-04, SYS-SAF-18 | Número máximo de falsas alarmas, su unidad y ventana. Parámetro calibrable. |
| Q-010 | SYS-PR-03 | Parámetros del filtro temporal del eco de raíles. Parámetros calibrables. |
| Q-013 | SYS-PR-09 | Valor de N confirmaciones. Parámetro calibrable. |
| Q-014 | SYS-DET-06, SYS-OUT-03 | Valor del error de localización y criterio de incertidumbre. |
| Q-016 | SYS-IN-02, SYS-IN-03, H-07 | Resolución de velocidad y ω y velocidad máxima del tranvía. |
| Q-018 | H-02 | Horizonte del giro constante. |
| Q-019, Q-052 | SYS-SAF-13, SYS-DET-07 | Umbral de desalineación del radar, error angular máximo de SYS-DET-07 (ambos parámetros calibrables) y método de detección. |
| Q-021 | SYS-PR-08, SYS-OUT-06 | Tamaño y resolución de las celdas. Parámetro calibrable. |
| Q-022 | SYS-GEN-02 | Entorno operativo nominal y sus hipótesis. |
| Q-023 | SYS-IN-07 | Criterio de validez de trama y contador de fallos consecutivos (FMEA). |
| Q-025 | SYS-OUT-01 | Formato detallado de coordenadas y objeto. |
| Q-026 | SYS-OUT-04, SYS-IN-06 | Resolución y tolerancia del timestamp. |
| Q-029 | SYS-PR-05 | Umbral de velocidad e histéresis. Parámetro calibrable. |
| Q-030 | SYS-T-01 | Percentil, carga y solape de ciclos del plazo de 128 ms. |
| Q-051 | SYS-SAF-08 | Tiempo máximo sin trama antes de pasar a fallo. Parámetro calibrable. |
| Q-035 | SYS-SAF-12 | Número de mediciones consecutivas sin ningún eco antes de pasar a degradado. Parámetro calibrable. |
| Q-031 | SYS-SAF-14 | Tiempo máximo de degradado continuado antes de pasar a fallo. Parámetro calibrable. |
| Q-005 | SYS-OUT-07, SYS-OUT-09 | Topic MQTT de estado y diagnóstico y formato del log CRIT (STD-MQTT, STD-LOG). |
| — | SYS-IN-01, SYS-IN-04, SYS-SAF-15 | Duración de la ejecución de prueba de los criterios de aceptación (antes fijada en 1 h), también en SYS-SAF-15. |

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-01 | Alicia Garcia | Creación a partir de la plantilla Requirements_2 |
| 0.2 | 2026-11-01 | Alicia Garcia | Ronda 1 y 2 de dudas: modos y transiciones (SYS-SAF-04, 05, 09, 12 a 17), HMI, SYS-OUT-07 a 09, SYS-T-01, SYS-IMP-03, H-01 y H-02 confirmadas. Códigos SYS-SAF-02 y SYS-SAF-03 retirados (absorbidos por SYS-SAF-12, 13 y 14), no se reutilizan |
| 0.3 | 2026-11-01 | Alicia Garcia | Ronda 3: la salida nunca declara ausencia de obstáculo (SYS-PR-08, SYS-OUT-06, SYS-SAF-05, 06, 07); detalle de la máquina de estados y de los resets aplazado a diseño (SYS-SAF-04, 08 a 14, 16, SYS-OUT-07, 09); SYS-SAF-17 retirado, código no reutilizado; sección «Decisiones aplazadas a diseño» |
| 0.4 | 2026-11-01 | Alicia Garcia | Ronda 4: objeto sobre la vía y campo de visión (Q-008, H-08); radar 3D (H-05, SYS-PR-07, SYS-IN-05; sustituye el radar 2D de requisitos.txt); tamaño mínimo 0,6 × 0,6 × 0,6 m (SYS-DET-02; sustituye «≥ 0,6 m²» de requisitos.txt L5); 220 m a lo largo de la vía (SYS-DET-01, SYS-PR-11); SYS-DET-03 retirado (probabilidad de detección a diseño), código no reutilizado; SYS-DET-04 reformulado; SYS-GEN-01 con EN 50126/50128/50129, SIL 2, sin certificar (Q-015) |
| 0.5 | 2026-11-01 | Alicia Garcia | Ronda 5: valores y parámetros aplazados a diseño sin marcadores pendientes; filtro temporal del eco de raíles (SYS-PR-03); estado por celdas (SYS-PR-08, SYS-OUT-06); marcha atrás: rechazo y degradado (SYS-PR-10, SYS-SAF-19); 220 m sin cambio de modo (SYS-PR-11); validación de timestamp y de trama (SYS-IN-06, SYS-IN-07); identificador de objeto (SYS-OUT-10); entorno operativo (SYS-GEN-02); sin memoria dinámica en todo el software (SYS-IMP-01); nube (cloud) solo para trazabilidad y logs, y funcionamiento sin ella (SYS-IMP-04, SYS-IMP-05); posición en coordenadas de vía (SYS-OUT-01); tamaño mínimo 0,6 × 0,6 × 0,6 m, SYS-DET-02, 08 |
| 0.6 | 2026-11-01 | Alicia Garcia | Ronda 6: se incorpora el enunciado R1–R4 (`entradas/enunciado.txt`); línea Source restaurada; entregables SYS-IMP-06 a SYS-IMP-12 con Origen R1, R2, R3 y R4; sección «Trazabilidad con el enunciado» |
| 0.7 | 2026-11-01 | Alicia Garcia | Ronda 7: probabilidad numérica de ocupación interna y tres estados derivados (SYS-PR-08, SYS-OUT-06, SYS-IMP-10); dos salidas, lista de objetos (SYS-OUT-11 nuevo) y estado por celdas; SYS-IMP-11 justifica C; SYS-IMP-06 como documento aparte en la fase de diseño; SYS-IMP-12 en Fase A y Fase B |
| 0.8 | 2026-11-01 | Alicia Garcia | Ronda 8: contigüidad de celdas (SYS-PR-04); SYS-IMP-05 retirado («no es relevante») y SYS-IMP-09 retirado (cubierto por SYS-SAF-01, 04 a 16, 18, 19, SYS-IN-06, 07 y SYS-OUT-07, 09), códigos no reutilizados; Origen de todos los requisitos con R1–R4 y «Derivado»; parámetros de la máquina de estados definidos en diseño (SYS-SAF-12, 13, 14, SYS-DET-04); tabla de trazabilidad regenerada |
| 0.9 | 2026-11-01 | Alicia Garcia | Ronda 9: vecindad de 8 en SYS-PR-04; sin cifra en SYS-SAF-08 (tiempo sin trama) ni en SYS-DET-07 y SYS-SAF-13 (1°), a diseño; SYS-PR-06 remite al error angular de SYS-DET-07; entregables SYS-IMP-06, 07, 08, 10, 11 y 12 se mantienen con Origen R1 a R4 |
| 0.10 | 2026-11-01 | Alicia Garcia | Duración de ejecución de los criterios de SYS-IN-01, SYS-IN-04 y SYS-SAF-15 definida en la fase de diseño (antes 1 h); los tres vuelven a Borrador. |
