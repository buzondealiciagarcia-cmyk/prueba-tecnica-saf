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

<!-- seccion: descripcion | estado: aprobado -->
## 7. Descripción del proyecto

### 7.1 Solución técnica

- **Hardware.** Radar frontal 3D (distancia, azimut y altura) a 1 m sobre el suelo, sensor de velocidad lineal y de giro, ECU edge a bordo y HMI del conductor. El estado, el diagnóstico y los logs van a la nube por MQTT cifrado, sin que la nube intervenga en el procesamiento ni en la salida al HMI.
- **Software.** Un pipeline de 15 pasos por cada nube de puntos de 50 ms, en cuatro bloques: entrada y validación; descartes (objetos dinámicos, señal débil, fuera de vía, por encima del techo y raíles); grid de ocupación y objetos; y cálculo de confianza y salida.
- **Grid de ocupación.** 880 × 4 celdas de 0,25 m. Cada celda guarda un log-odds: un eco suma 0,85, una onda que pasa sin eco resta 0,40 y una celda oculta tras un eco no cambia. Con tres ecos seguidos la celda pasa a ocupado. Nunca existe el estado «libre».
- **Modos y fallos.** Modos init, normal, degradado y fallo, con una tabla de fallos (detección, mitigación, notificación y recuperación) y resets de dos niveles.
- **Salida.** La lista de objetos estáticos con posición, tamaño, incertidumbre y confianza, junto con el modo del sistema y un indicador de vigilancia.

### 7.2 Metodología

- **Fases con puertas.** Requisitos, diseño, implementación y oferta, cada una cerrada con su versión y su etiqueta, de modo que cada fase parte de la anterior ya aprobada. La autora aprueba los requisitos, el diseño y el contrato de cada función.
- **Revisión del diseño.** El diseño se contrastó con los requisitos en varias rondas: 62 hallazgos, todos resueltos.
- **Código y pruebas independientes.** El código y las pruebas de cada función los generan agentes de inteligencia artificial distintos: uno implementa la función y otro escribe sus tests, sin ver el trabajo del otro, a partir del mismo contrato aprobado. La verificación es automática y se ejecuta sobre cada función: cobertura del 100 % de líneas y de ramas, cumplimiento de MISRA C:2012, análisis estático, formato, complejidad acotada y ejecución con detectores de errores de memoria y de comportamiento indefinido.
- **Trazabilidad.** Cada requisito se traza hasta el apartado del diseño, la función y los casos de test que lo verifican.
- **Documentos de cada fase.** Cada fase tiene su documento en Markdown, versionado en git, y su exportación a Word (`.docx`):

  | Fase | Markdown (en git) | Word |
  |---|---|---|
  | Requisitos | `docs/01-requisitos/requisitos.md` y `dudas.md` | `docs/export/requisitos.docx` |
  | Diseño | `docs/02-diseno/diseno.md`, `viabilidad-radar.md` y `justificacion-c.md`, con los diagramas en `docs/02-diseno/diagramas/` | `docs/export/diseno.docx` |
  | Implementación | `docs/03-implementacion/implementacion.md`, con `trazabilidad.md`, `contratos/`, `casos/` y `wcet-medidas.md` | `docs/export/implementacion.docx` |
  | Oferta | `docs/04-oferta/oferta.md` | `docs/export/oferta.docx` |

  Los ficheros Word se generan a partir de los Markdown y no se guardan en git: la carpeta `docs/export/` queda fuera del control de versiones. Quien descargue el proyecto de git los obtiene instalando el harness con `./harness/install.sh --target .` y ejecutando `.claude/harness/bin/h export <fase>`, con `<fase>` igual a `requisitos`, `diseno`, `implementacion` u `oferta`.
- **Control de versiones con git.** Todo el trabajo se ha mantenido bajo control de versiones: una rama por fase y por función, mensajes de commit con formato Conventional Commits, en inglés y con la referencia al requisito que cubren, fusiones sin avance rápido (`--no-ff`) y etiquetas por fase (`req-v1.2`, `design-v1.2`, `impl-v1.2`) y por versión del módulo (`v1.1.0`). Hooks de git validan el formato de los mensajes, protegen las ramas principales y comprueban que la última verificación de una función esté en verde antes de subirla.
- **Harness de ingeniería con IA.** Para habilitar el proyecto se creó un harness (arnés) para Claude Code, que está en la carpeta `harness/` del repositorio con su `README.md` y su instalador. Reúne 15 estándares (requisitos, código C, documentación, seguridad funcional, MQTT, red, configuración, pruebas, git y otros), comandos para cada paso, siete agentes especializados, skills, hooks y scripts. Impone el flujo por fases con puertas: no se avanza sin cerrar la fase anterior, y solo la autora puede cerrar o reabrir una fase. Garantiza la independencia entre código y pruebas con áreas de trabajo separadas, ejecuta de forma determinista las verificaciones automáticas y genera los Word, los diagramas y la matriz de trazabilidad. El estado de las fases queda en `.harness/state.json`.

### 7.3 Resultados de la Fase A

Viabilidad concluida, diseño cerrado y el módulo del grid implementado y verificado.

**Puntos clave de la fase implementada**

- **Grid de ocupación implementado.** Siete funciones en C del módulo `ogrid`: reinicio, actualización por haz y por celda, límites del valor, probabilidad de ocupación, estado de la celda y máscara de celdas ocupadas. La memoria es fija (880 × 4 celdas), sin memoria dinámica, y el resultado depende solo de los ecos del radar.
- **Probabilidad de ocupación por celda.** Cada celda guarda un log-odds a partir del cual se calcula su probabilidad. Una celda pasa a ocupada con tres ecos seguidos y no con uno aislado, y una celda oculta tras un eco no cambia.
- **Nunca «libre».** El estado de una celda es ocupado, probabilidad baja de obstáculo o zona no estudiada. Una celda con datos corruptos se informa como error y se trata como zona no estudiada.
- **Verificación.** 209 casos de test (194 unitarios y 15 de integración), todos en verde, con cobertura del 100 % de líneas y de ramas, MISRA C:2012 con tres desviaciones registradas y aprobadas, análisis estático y detectores de errores de memoria.
- **Tiempos.** Medidos en el equipo de desarrollo, no en la ECU: un haz de 880 celdas cuesta unos 2 µs (máximo observado 2,7 µs) y la máscara de las 3 520 celdas unos 9 µs (máximo 10,6 µs), muy por debajo del presupuesto de 78 ms de proceso del diseño.
- **Visor.** Seis escenarios que reproducen el comportamiento del grid ciclo a ciclo con el código real.

**Cómo lo probado cumple los requisitos de Ikerlan**

| Requisito de Ikerlan | Qué se ha probado | Resultado | Qué falta (Fase B) |
|---|---|---|---|
| Algoritmo de grid de ocupación dinámico con la probabilidad de cada celda, basada exclusivamente en los ecos del radar | Actualización por haz y por celda; probabilidad de ocupación de cada celda; confirmación con tres ecos | Cumplido para el grid | Alimentarlo con el radar real |
| 1. Detectar objetos estáticos de más de 0,60 m en la vía a menos de 220 m con un único radar frontal | El grid cubre 220 m de vía en celdas de 0,25 m; marca como ocupada una celda tras tres ecos y no con un eco aislado; extrae la máscara de celdas ocupadas | Cumplido solo en la parte del grid | Agrupar celdas en objetos, filtrar por tamaño, confirmar en N ciclos y geometría de vía |
| 2. Analizar los datos del radar junto con la velocidad y el giro del tranvía | Diseñado, no probado | No probado | Recepción, validación y sincronización de radar, velocidad y giro |
| 3. Entregar los objetos (posición, tamaño y confianza) en menos de 128 ms | Los tiempos del grid medidos en el equipo de desarrollo | Parcial: el coste del grid es de microsegundos | Medir el ciclo completo y la entrega al HMI en la ECU |
| 4. Pasar a modo degradado o declarar un error ante fallos del sensor | El grid detecta sus datos corruptos y no declara nunca una celda libre | Parcial: piezas de base | Máquina de estados y gestión de fallos |

Para las partes no implementadas (la Fase B), el planteamiento está en el documento de diseño (`docs/02-diseno/diseno.md`, y su exportación `docs/export/diseno.docx`): arquitectura hardware y software, pipeline de 15 pasos por ciclo, comunicaciones, modos y fallos, rendimiento y configuración.

Una explicación más detallada de los resultados está en el documento de implementación (`docs/03-implementacion/implementacion.md`, y su exportación `docs/export/implementacion.docx`): fichas de cada función, resultados de verificación, tiempos de ejecución y trazabilidad.

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
