# Documento de diseño

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-DIS |
| Versión | 0.1 |
| Fecha | 2026-11-01 |
| Autor | Alicia Garcia |
| Estado | Borrador |

> Este documento recoge **solo** el diseño definido por el usuario (STD-DOC DOC-GEN-01). Las propuestas de Claude se marcan `[PROPUESTA]` hasta su aceptación y los huecos `[PENDIENTE: …]`.

Fuentes (sin modificar, en `entradas/`): `Diseño de Hardware.docx` (HW), `Diseño de Software.docx` (SW), `Máquina de Estados.docx` (ME).

## 1. Contexto y objetivos del diseño

Sistema de detección de obstáculos en vía mediante radar frontal, con procesamiento en el borde (edge) a bordo del tranvía (SYS-IMP-03). Salida hacia el HMI del conductor (HW).

Justificación de la arquitectura edge (HW): buena latencia, disponibilidad, certificable y más segura frente a ataques que el procesamiento en la nube. Se descarta el procesamiento en la nube por latencia, fiabilidad y restricciones de tiempo de procesamiento y análisis, que pondrían en riesgo el cumplimiento de los estándares del proyecto (SYS-IMP-03, SYS-T-01, SYS-GEN-01).

Entregables de la fase (SYS-IMP-06, SYS-IMP-07, SYS-IMP-08, SYS-IMP-10, SYS-IMP-11): [PENDIENTE: el material aportado no incluye el plan de viabilidad del radar (R1), el pseudocódigo de un componente ni la justificación del lenguaje más allá de «usaremos C porque el diseño es una ECU edge»].

## 2. Requisitos de referencia

Versión de requisitos: `req-v1.0`. Cada apartado cita los requisitos que cubre.

## 3. Arquitectura hardware

![Figura. Arquitectura hardware](diagramas/arquitectura-hw.svg)

| Elemento | Definición del usuario | Requisitos |
|---|---|---|
| ECU edge | Recibe los datos, procesa y publica en el HMI. Modelo, CPU, memoria: [PENDIENTE: no definidos] | SYS-IMP-03, SYS-IMP-04 |
| Radar | Radar de ocupación dinámico 3D: distancia, azimut y altura. Proporciona timestamp de medida a la ECU. Montado a 1 m sobre el suelo. Modelo concreto pendiente de decisión del cliente (valoración funcional y presupuestaria) | SYS-IN-01, SYS-IN-04, SYS-IN-05, SYS-PR-07, H-05 |
| Sensor de velocidad | Mide velocidad lineal y de giro. Proporciona timestamp de medida a la ECU | SYS-IN-02, SYS-IN-03, SYS-IN-04 |
| HMI | Salida de avisos e información de la ECU; entrada de sign in y resets (ME) | SYS-OUT-06, SYS-OUT-11, SYS-SAF-04, SYS-SAF-16 |
| Radar frontal activo | El sign in del conductor en el HMI indica qué cabina está activa y por tanto qué radar frontal se activa (los tranvías suelen ser bidireccionales) (ME). Número de radares instalados: [PENDIENTE] | SYS-IN-01 |

Parámetros de partida (HW):

| Parámetro | Valor definido | Observación | Requisitos |
|---|---|---|---|
| Altura del radar | 1 m sobre el suelo | Parámetro de configuración | SYS-IN-05 |
| Error angular del radar | 1° | Valor provisional mientras el cliente no elija radar; después se usará la especificación del radar elegido | SYS-DET-07, H-04 |
| Error de localización | 0,005 m | Valor provisional hasta tener la especificación del radar; el error de altura crece con la distancia | SYS-DET-06, H-06 |
| Velocidad máxima del tranvía | 50 km/h (velocidad regulada en Vitoria-Gasteiz) | | H-07 |
| Alcance de detección | 220 m a lo largo de la vía (SW, paso 5) | | SYS-DET-01, SYS-PR-11 |

Evoluciones futuras sugeridas al cliente en la oferta (no son requisitos, HW):

- Acceso de la ECU al sistema de freno para detener el tranvía en emergencia.
- Acceso de la ECU a la nube para actualizaciones de la flota desde un centro de control, telemetría adicional y descarga de mapas (el envío de logs y trazabilidad ya es requisito, SYS-IMP-04).
- Segundo radar, para reducir el error de altura a gran distancia y aportar redundancia.

## 4. Arquitectura software

![Figura. Arquitectura software](diagramas/arquitectura-sw.svg)

Lenguaje: C, por ser el diseño una ECU edge (SW). El software cumple los requisitos elaborados y los estándares fijados en el harness (SYS-IMP-01, SYS-IMP-02, SYS-IMP-11). Estructura de módulos, tareas y hebras: [PENDIENTE: no definidos].

Funcionalidades en el orden del ciclo (SW):

| Paso | Definición del usuario | Requisitos |
|---|---|---|
| 1 | Recibe en paralelo la nube del radar y la velocidad lineal y de giro | SYS-IN-01, SYS-IN-02, SYS-IN-03 |
| 2 | Valida los datos según estándares y requisitos; los timestamps se validan contra el reloj de la ECU, que es la referencia (H-017); sincronización con el reloj del radar y tolerancia [PENDIENTE] | SYS-IN-06, SYS-IN-07 |
| 3 | Sincroniza en la ECU, con los timestamps de medida, las velocidades al instante de la «fotografía» de la nube, interpolando si hace falta (H-020); actualiza la posición del tranvía y estima la curva (puede ser recta) | SYS-PR-02, SYS-PR-06, H-02, H-03 |
| 4 | Convierte la nube de (r, azimut, elevación) a (x, y, z) considerando la altura del radar | SYS-PR-07, SYS-IN-05 |
| 5 | Descarta elementos dinámicos y elementos con señal muy débil. Los objetos en movimiento sobre la vía no se reportan (H-007) | SYS-PR-05 |
| 6 | Pasa de coordenadas (x, y) a coordenadas de vía (s, d) y descarta lo situado a más de 220 m a lo largo de la vía, s > 220 m (H-014) | SYS-PR-01, SYS-PR-11 |
| 7 | Descarta lo que está fuera de la vía | SYS-DET-05 |
| 8 | Descarta lo que está por encima del techo (evita falsos positivos por puentes) | SYS-DET-04 |
| 9 | Descarta raíles: detecciones muy cercanas a las coordenadas de los raíles | SYS-PR-03 |
| 10 | Actualiza el grid: celda con detección estática suma +0,85; celda atravesada por la luz recibe un voto «libre»; celda no observada recibe «desconocido» | SYS-PR-08 |
| 11 | Extrae las celdas con p > 0,8, agrupa las contiguas y obtiene posición, tamaño y altura de cada objeto | SYS-PR-04, SYS-OUT-01, SYS-OUT-02 |
| 11b | Descarta los objetos con alguna dimensión menor de 0,6 m (H-015) | SYS-DET-02, SYS-DET-08 |
| 12 | Compara los objetos con los del ciclo anterior y los reporta si llevan más de 2 ciclos seguidos, es decir N = 3 (H-005) | SYS-PR-09, SYS-OUT-10 |
| 13 | Calcula confianza e incertidumbre de cada objeto según distancia, ciclos confirmados e incertidumbre de la curva | SYS-OUT-03, SYS-OUT-08 |
| 14 | Gestiona el modo según la máquina de estados | SYS-SAF-01 |
| 15 | Crea la salida según el modo | SYS-OUT-06, SYS-OUT-11, SYS-SAF-05, SYS-SAF-06 |

Huecos del material en este apartado:

- [PENDIENTE: umbral de «señal muy débil» (paso 5)].
- [PENDIENTE: altura del techo del tranvía (paso 8)].
- [PENDIENTE: distancia y tolerancia para «muy cerca de raíles» y parámetros del filtro de persistencia (paso 9, SYS-PR-03)].
- [PENDIENTE: la regla de agrupación en el paso 11 no indica la vecindad; SYS-PR-04 exige 8 vecinos].
- [PENDIENTE: fórmulas de confianza e incertidumbre (paso 13)].
- [PENDIENTE: dónde se evalúa SYS-PR-10 (velocidad negativa)].

## 5. Comunicaciones

| Enlace | Protocolo definido | Requisitos |
|---|---|---|
| Radar → ECU | Marcado por el fabricante del radar; lo más común Automotive Ethernet, también CAN FD en algunos casos | SYS-IN-01, SYS-IN-04 |
| Sensor de velocidad → ECU | [PENDIENTE: no definido] | SYS-IN-02, SYS-IN-03 |
| ECU ↔ HMI | El que ya use el tranvía para comunicarse entre sí; lo más común Ethernet ferroviario (ECN, IEC 61375) con capa de seguridad EN 50159 | SYS-OUT-06, SYS-OUT-11 |
| ECU → nube | MQTT con el cifrado necesario. Obligatorios (H-004): mensaje MQTT de estado y diagnóstico (QoS 1, retained) y log CRIT en cada paso a DEGRADADO o FALLO. Propuesta al cliente: telemetría adicional, actualizaciones de flota y mapas. No preocupa la latencia ni es crítico perder mensajes de telemetría | SYS-IMP-04, SYS-OUT-07, SYS-OUT-09 |

Fallos de MQTT o de la nube (H-018): no se registran como fallo de la máquina de estados ni son críticos para la seguridad funcional, porque el sistema funciona completo sin la nube (SYS-IMP-04). Perder la conexión con el broker no cambia el modo. Esto se aparta de MQ-QOS-03 (pérdida del broker → evento de fallo y estado seguro) por decisión del usuario; desviación a registrar con su análisis de impacto (SAF-07): la nube no interviene en el procesamiento ni en la salida al HMI.

### 5.1 Red y direccionamiento (STD-NET)

| Nodo | Función | VLAN | IP/máscara | Puertos | Protocolo | Dirección del tráfico |
|---|---|---|---|---|---|---|
| [PENDIENTE] | [PENDIENTE: el material no define direccionamiento, VLAN ni puertos] | | | | | |

### 5.2 Topics MQTT (STD-MQTT)

| Topic | Publicador | Suscriptores | QoS | Retained | Periodo | Payload (esquema) | Requisitos |
|---|---|---|---|---|---|---|---|
| [PENDIENTE: topics, QoS y payload no definidos] | ECU | | | | | | SYS-OUT-07 |

![Figura. Secuencia de un ciclo de 50 ms](diagramas/secuencia-ciclo.svg)

## 6. Datos y algoritmo del grid de ocupación

### 6.1 Representación del grid

[PENDIENTE: tamaño de celda, dimensiones y orientación del grid; el material solo indica coordenadas de vía (s, d) y alcance de 220 m] (SYS-PR-08, SYS-DET-01).

### 6.2 Modelo inverso del sensor

Celda con detección estática: +0,85. Celda atravesada por la luz: voto «libre». Celda no observada: «desconocido» (SW paso 10). El voto +0,85 se suma en escala log-odds (H-013): partiendo de p = 0,5, una detección da p ≈ 0,70 y dos seguidas p ≈ 0,85, por lo que el umbral p > 0,8 (log-odds > 1,39) no se alcanza en un solo ciclo. Estados de celda derivados (H-012): ocupado si p > 0,8; zona no estudiada si la celda nunca se ha observado o se ha reiniciado; probabilidad baja de obstáculo si ha sido observada y p ≤ 0,8; nunca «libre». [PENDIENTE: valor del voto «libre» y límites de saturación] (SYS-PR-08, SYS-SAF-07).

### 6.3 Actualización y dinámica (decaimiento, estáticos vs. móviles)

Los elementos dinámicos se descartan antes de actualizar el grid (SW paso 5, SYS-PR-05). Al volver de FALLO el grid se reinicia a «desconocido» porque las mediciones anteriores no son fiables (ME). [PENDIENTE: decaimiento temporal y criterio de velocidad para clasificar un eco como dinámico].

### 6.4 Flujo por ciclo

Ver §4 (pasos 1 a 15) y la figura de arquitectura software. Umbral de extracción de objetos: p > 0,8. Confirmación: el objeto se reporta si lleva más de 2 ciclos seguidos, N = 3 (H-005, SYS-PR-09), independiente de las 3 mediciones de SYS-SAF-12. Salida: celdas ocupadas, probabilidad baja de obstáculo o zona no estudiada; nunca «libre» (SYS-SAF-07, SYS-OUT-06).

## 7. Modos y estados del sistema

![Figura. Modos del sistema](diagramas/estados.svg)

El último estado activo debe ser conocido al encender el tranvía de nuevo, para que el fallo no pueda resetearse de forma no segura (ME). [PENDIENTE: mecanismo de persistencia, H-009].

| Estado | Comportamiento definido (ME) | Requisitos |
|---|---|---|
| INIT | Entrega «no hay vigilancia» mientras comprueba niveles básicos de funcionamiento y espera un input del conductor. Estado inicial tras el arranque | SYS-SAF-06 |
| NORMAL | Funcionamiento normal: cumple los requisitos | SYS-SAF-15 |
| DEGRADADO | Sigue funcionando y da alertas y datos con confianza reducida | SYS-SAF-05 |
| FALLO | No entrega ninguna celda como libre: «no hay vigilancia» | SYS-SAF-06, SYS-SAF-07 |

| Transición | Condiciones definidas (ME) | Requisitos |
|---|---|---|
| INIT → NORMAL | Autodiagnóstico del radar; datos válidos durante 1 ciclo; cálculo de predicciones durante 1 ciclo; sign in del conductor en el HMI (que también identifica la cabina activa). En el futuro la señal de INIT podría darla el arranque del tranvía | SYS-SAF-01 |
| INIT → FALLO | Falla cualquier condición de INIT; INIT tarda más de 3 min; hay un fallo memorizado (implementación futura) | SYS-SAF-01 |
| NORMAL → DEGRADADO | No se detecta nada, ni siquiera los raíles, en 3 mediciones seguidas (H-002); desalineación del radar | SYS-SAF-12, SYS-SAF-13 |
| DEGRADADO → NORMAL | Se exigen a la vez (H-003): reset nivel 1 desde el HMI; condiciones que llevaron a DEGRADADO limpiadas; funcionamiento normal ininterrumpido, p. ej. mediciones correctas durante 4 ciclos seguidos del radar, es decir 200 ms (este valor podrá cambiar tras las fases de test y verificación) | SYS-SAF-04 |
| DEGRADADO → FALLO | Estado degradado continuado en el tiempo; cualquier condición de fallo estando en degradado, p. ej. aviso de autofallo del radar | SYS-SAF-14 |
| FALLO → DEGRADADO | No es posible | SYS-SAF-16 |
| NORMAL → FALLO | No se recibe ninguna trama durante el timeout de trama (H-008, valor [PENDIENTE]; el material proponía 128 ms); el timestamp de los datos no avanza o retrocede; un dato no está disponible; fallo autodiagnosticado por el radar | SYS-SAF-08, SYS-SAF-09, SYS-SAF-10, SYS-SAF-11 |
| FALLO → NORMAL | Reset nivel 2 desde el HMI: solo una persona autorizada, con doble autenticación; solo se acepta si las condiciones están limpias. El grid se reinicia a «desconocido» | SYS-SAF-16 |

Huecos y puntos a contrastar con los requisitos (se tratan en `/diseno-revisar`):

- [PENDIENTE: tiempo máximo en DEGRADADO antes de pasar a FALLO (SYS-SAF-14) y umbral de desalineación (SYS-SAF-13)].
- [PENDIENTE: el material no recoge las transiciones por exceso de falsas alarmas (SYS-SAF-18) ni por velocidad negativa (SYS-SAF-19), ni el máximo de falsas alarmas (SYS-DET-04)].
- Los requisitos definen tres modos (SYS-SAF-01) y el material añade INIT como cuarto estado.
- Resuelto (H-002): «sin detección ni raíles durante 3 mediciones» lleva a DEGRADADO, como SYS-SAF-12.
- Resuelto (H-011): se elimina la condición «con raíles → DEGRADADO».
- Resuelto (H-010): un reset de nivel equivocado se rechaza, se mantiene el modo y se registra un log.
- Resuelto (H-009): la persistencia del último estado queda [PENDIENTE] y no cuenta como mitigación vigente.
- Resuelto (H-003): el reset de degradado a normal exige reset nivel 1, condiciones limpias y 4 ciclos correctos a la vez.
- Abierto (H-001): el usuario decide que SYS-SAF-01 pida tres modos como mínimo en lugar de exactamente tres; requiere reabrir requisitos (`/fase-reabrir req`).

## 8. Gestión de fallos y seguridad funcional (STD-SAF)

| Modo de fallo | Causa | Efecto | Detección | Mitigación | Requisito |
|---|---|---|---|---|---|
| Ausencia de tramas | Radar o enlace caído | Sin datos | Ninguna trama durante el timeout de trama | Paso a FALLO | SYS-SAF-08 |
| Timestamp incoherente | Reloj o datos corruptos | Datos no fiables | Timestamp que no avanza o retrocede | Paso a FALLO | SYS-SAF-09, SYS-IN-06 |
| Dato de entrada no disponible | Fallo del sensor de velocidad u otro | Datos incompletos | Dato ausente | Paso a FALLO | SYS-SAF-10 |
| Fallo del radar | Autodiagnóstico del radar | Detección no fiable | Notificación del radar | Paso a FALLO | SYS-SAF-11 |
| Sin ecos ni raíles | [PENDIENTE: causa] | Confianza reducida | 3 mediciones seguidas sin ninguna detección | Paso a DEGRADADO | SYS-SAF-12 |
| Desalineación del radar | Montaje o golpe | Error de posición | Desalineación detectada | Paso a DEGRADADO | SYS-SAF-13 |
| Degradado prolongado | Causa no resuelta | Detección con confianza reducida | Tiempo continuado en DEGRADADO | Paso a FALLO | SYS-SAF-14 |
| Fallo de INIT | Autodiagnóstico o datos no válidos | Sin vigilancia | Condición de INIT falla o INIT supera 3 min | Paso a FALLO | SYS-SAF-01 |
| Reset indebido de fallo | Reset no autorizado o de nivel equivocado | Reanudar con fallo activo | Autenticación, condiciones y nivel del reset | Reset nivel 2 con doble autenticación, solo con condiciones limpias; un reset del nivel equivocado se rechaza (H-010). Persistencia del último estado al reiniciar: [PENDIENTE: mecanismo, H-009]; hasta definirlo no cuenta como mitigación vigente | SYS-SAF-16 |

Fuera de esta tabla por decisión del usuario (H-018): fallos de MQTT, del broker y de la nube, que no afectan al modo ni a la seguridad funcional (§5). Filas pendientes de añadir: datos corruptos de trama (SYS-IN-07), huecos de seq, saturación de CPU y desbordamiento; y columna «Notificación» (log CRIT + topic de diagnóstico).

Tabla por completar en revisión: [PENDIENTE: causas y análisis de fallo propios de STD-SAF no aportados].

## 9. Rendimiento y presupuesto temporal

- Periodo del radar y de las velocidades: 50 ms (SYS-IN-01, SYS-IN-02, SYS-IN-03).
- Tiempo máximo de entrega de objetos: 128 ms (SYS-T-01), distinto del timeout de trama. El timeout de ausencia de tramas es un parámetro distinto, [PENDIENTE] en múltiplos de 50 ms (H-008, SYS-SAF-08).
- [PENDIENTE: presupuesto temporal por paso del ciclo, hardware de la ECU y margen de CPU; no aportados].

## 10. Configuración (STD-CFG)

| Parámetro | Valor definido | Requisito |
|---|---|---|
| Altura del radar | 1 m | SYS-IN-05 |
| Error angular | 1° (provisional) | SYS-DET-07 |
| Error de localización | 0,005 m (provisional) | SYS-DET-06 |
| Alcance | 220 m | SYS-DET-01, SYS-PR-11 |
| Velocidad máxima del tranvía | 50 km/h | H-07 |
| Voto de celda estática | +0,85 en log-odds | SYS-PR-08 |
| Umbral de extracción de objetos | p > 0,8 | SYS-PR-04 |
| Ciclos de confirmación N | 3 (más de 2 seguidos) | SYS-PR-09 |
| Mediciones sin detección ni raíles para degradado | 3 mediciones | SYS-SAF-12 |
| Timeout de trama para fallo | [PENDIENTE: múltiplo de 50 ms] | SYS-SAF-08 |
| Tiempo máximo de entrega de objetos | 128 ms | SYS-T-01 |
| Ciclos de recuperación a normal | 4 (podrá cambiar tras test) | SYS-SAF-04 |
| Tiempo máximo de INIT | 3 min | SYS-SAF-01 |
| Tiempo máximo en DEGRADADO | [PENDIENTE] | SYS-SAF-14 |
| Umbral de desalineación | [PENDIENTE] | SYS-SAF-13 |
| Máximo de falsas alarmas | [PENDIENTE] | SYS-DET-04 |

## 11. Trazabilidad requisito → apartado

| Requisito | Apartado(s) del diseño |
|---|---|
| SYS-GEN-01 | 1 |
| SYS-IN-01, SYS-IN-02, SYS-IN-03, SYS-IN-04 | 3, 4, 5 |
| SYS-IN-05 | 3, 10 |
| SYS-IN-06, SYS-IN-07 | 4, 8 |
| SYS-PR-01, SYS-PR-02, SYS-PR-06, SYS-PR-07 | 4 |
| SYS-PR-03, SYS-PR-04, SYS-PR-05, SYS-PR-08, SYS-PR-09 | 4, 6 |
| SYS-DET-02, SYS-DET-08 | 4 (paso 11b) |
| SYS-PR-11, SYS-DET-01 | 3, 4, 10 |
| SYS-DET-05, SYS-DET-06, SYS-DET-07 | 3, 4, 10 |
| SYS-OUT-01, SYS-OUT-02, SYS-OUT-03, SYS-OUT-08, SYS-OUT-10, SYS-OUT-11 | 4 |
| SYS-OUT-06 | 4, 6, 7 |
| SYS-OUT-07, SYS-OUT-09 | 5 (solo enlace a la nube; contenido MQTT sin definir) |
| SYS-T-01 | 1, 9 |
| SYS-SAF-01, SYS-SAF-04, SYS-SAF-05, SYS-SAF-06, SYS-SAF-07, SYS-SAF-08, SYS-SAF-09, SYS-SAF-10, SYS-SAF-11, SYS-SAF-12, SYS-SAF-13, SYS-SAF-14, SYS-SAF-15, SYS-SAF-16 | 7, 8 |
| SYS-IMP-01, SYS-IMP-02, SYS-IMP-11 | 4 |
| SYS-IMP-03, SYS-IMP-04 | 1, 3, 5 |

Requisitos Must sin cobertura en el material aportado:

- [PENDIENTE: SYS-DET-04 (falsas alarmas), SYS-SAF-18, SYS-SAF-19, SYS-PR-10 (velocidad negativa)].
- [PENDIENTE: SYS-OUT-04, SYS-OUT-05 (timestamp y alcance estimado a la salida)].
- [PENDIENTE: SYS-GEN-02 (entorno operativo)].
- [PENDIENTE: SYS-IMP-06, SYS-IMP-07, SYS-IMP-08, SYS-IMP-10, SYS-IMP-12 (entregables; SYS-IMP-08 se cubre en parte con la figura de §4)].

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-01 | Alicia Garcia | Creación a partir del diseño aportado por el usuario |
