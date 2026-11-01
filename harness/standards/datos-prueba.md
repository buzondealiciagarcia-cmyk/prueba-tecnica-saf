# STD-DAT · Datos de prueba y grabaciones de radar

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Nubes de puntos de entrada (sintéticas o grabadas), salidas de referencia del grid y escenarios de test |

| ID | Nivel | Regla |
|---|---|---|
| DAT-01 | OBL | Formato de grabación: JSON Lines (`.jsonl`), una nube por línea con el **mismo esquema que el payload MQTT** (STD-MQTT §2). Así el `replay` publica las líneas tal cual. |
| DAT-02 | OBL | Cada grabación tiene un fichero hermano `<nombre>.meta.json`: origen (sintético/real), generador y semilla, montaje del radar (posición y orientación respecto a la vía, en m y rad), duración, nº de nubes, escenario descrito y requisitos que ejerce. |
| DAT-03 | OBL | Escenarios mínimos a cubrir: vía libre; obstáculo estático en vía a varias distancias; obstáculo fuera de vía (no debe marcarse en vía); objeto móvil que cruza (no debe consolidarse como estático); ecos espurios/ruido; pérdida de nubes (huecos de `seq`); nube vacía; nube con el máximo de ecos. |
| DAT-04 | OBL | Los datos sintéticos se generan con `tools/gen_scenario.py` determinista (semilla fija) y se pueden regenerar; no se versionan ficheros > 5 MB (se versiona el generador y la semilla). |
| DAT-05 | OBL | Salidas de referencia (*golden*) de grid en `tests/data/golden/` con tolerancia documentada; solo el `ingeniero-tests` las crea, a partir de cálculos analíticos o del criterio del requisito, nunca ejecutando la implementación. |
| DAT-06 | OBL | Datos reales: anonimizados y con licencia/procedencia documentada en el `.meta.json`. |
