# Registro de desviaciones MISRA C:2012

Toda desviación necesita registro aquí antes de suprimirse (C-LNG-03, SAF-07).

| ID | Regla | Ámbito (fichero:línea o proyecto) | Justificación | Análisis de riesgo | Aprobada por | Fecha |
|---|---|---|---|---|---|---|
| DEV-001 | 15.5 (un único punto de salida) | Proyecto | Las cláusulas de guarda al inicio de la función mejoran la legibilidad y evitan anidamiento profundo (C-FUN-03) | Bajo: el retorno anticipado solo ocurre antes de cualquier efecto lateral | Alicia Garcia | 2026-11-02 |
| DEV-002 | 2.3, 2.4, 2.5 (tipos, etiquetas y macros sin usar) | `include/ogrid/*.h` | Las cabeceras declaran la API y las constantes del módulo para sus consumidores; el análisis por unidad de traducción no ve usados los que esa unidad no necesita | Bajo: son solo declaraciones, no generan código ni cambian el comportamiento | Alicia Garcia | 2026-11-02 |
| DEV-003 | 8.7 (símbolo con enlace externo usado en una sola unidad) | Proyecto, funciones públicas de `src/ogrid/` | Una función pública por fichero (C-NAM-06): cada una forma la API y se usa desde otras unidades o desde los tests | Bajo: no cambia el comportamiento; el enlace externo es el pretendido | Alicia Garcia | 2026-11-02 |
