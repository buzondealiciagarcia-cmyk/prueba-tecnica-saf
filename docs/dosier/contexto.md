# Contexto del proyecto

**Origen.** El proyecto parte de la «Prueba práctica SAF: detección de obstáculos con radar» del equipo SAF de la unidad DIG de Ikerlan. La práctica pide diseñar un algoritmo basado en un grid de ocupación dinámico para identificar obstáculos estáticos sobre la vía de un tranvía con un radar frontal, y se desglosa en cuatro ejercicios: un plan de viabilidad del uso del radar, el diseño del sistema, la implementación del código de un componente y una oferta al cliente.

**Problema.** Un tranvía a 50 km/h recorre unos 14 m cada segundo, y los 220 m de alcance exigidos se cubren en unos 16 s. El sistema debe detectar objetos estáticos de 0,6 m o más situados en la vía a menos de 220 m, con un único radar frontal que entrega una nube de puntos cada 50 ms; debe entregarlos al conductor en menos de 128 ms, pasar a modo degradado o declarar un error ante fallos del sensor y no dar nunca la vía por libre sin haberlo comprobado. Es un sistema que requiere seguridad funcional.

**Enfoque.** El trabajo se ha hecho en cuatro fases con puertas: requisitos, diseño, implementación y oferta. Cada fase se cierra con su versión y su etiqueta en git, y no se avanza sin cerrar la anterior. Este dosier reúne las tres primeras; la oferta, con su presentación, se entrega aparte. Para habilitar el proyecto se creó un harness de ingeniería para Claude Code, que está en la carpeta `harness/` del repositorio e impone ese flujo. El código y los tests de cada función los generan agentes de inteligencia artificial distintos que no se ven entre sí, a partir de un contrato aprobado por la autora.

**Alcance de las fases 1 a 3.**

- **Requisitos (versión 1.2).** 64 requisitos y 8 hipótesis, cerrados y aprobados.
- **Diseño (versión 1.2).** Diseño hardware y software del sistema completo, con el pipeline de un ciclo y la gestión de fallos. Como documentos de apoyo se incluyen el plan de viabilidad del radar y la justificación del lenguaje C. No se estudiaron diseños alternativos, por lo que no hay comparativa.
- **Implementación (versión 1.2).** Solo se ha implementado el módulo del grid de ocupación: siete funciones en C, 209 casos de test, tiempos de ejecución medidos en el equipo de desarrollo y un visor que reproduce escenarios con el código real. El resto del sistema está diseñado y no implementado; la oferta lo plantea como Fase B.

**Cómo leer el dosier.** La Parte I recoge qué debe hacer el sistema (requisitos). La Parte II, cómo se resuelve (diseño, viabilidad y justificación del lenguaje). La Parte III, qué se ha construido y cómo se ha verificado (implementación y trazabilidad). Las tablas de la guía de lectura indican la versión de cada documento y el estado de las fases. Cada documento también existe por separado en Markdown, en `docs/`, y se puede exportar a Word con el harness.
