---
name: matriz-ponderada
description: Construye matrices de decisión ponderadas para comparar el diseño seleccionado con alternativas (pesos, puntuaciones, sensibilidad, cumplimiento de requisitos). Úsala en /diseno-comparar.
---
# Matriz ponderada

CLI: `H="@@HARNESS@@/bin/h"` · Fuente: `docs/02-diseno/comparativa.json` · Salida: bloque generado en `comparativa.md` + `diagramas/comparativa-totales.mmd`.

```json
{
  "escala": {"min": 1, "max": 5, "descripcion": "1 = muy desfavorable · 5 = muy favorable"},
  "criterios": [{"id": "C1", "nombre": "Coste HW", "peso": 25, "descripcion": "…"}],
  "alternativas": [{"id": "BASE", "nombre": "Diseño seleccionado", "seleccionada": true},
                   {"id": "ALT-01", "nombre": "…"}],
  "puntuaciones": {"BASE": {"C1": 4}, "ALT-01": {"C1": 2}},
  "justificaciones": {"ALT-01": {"C1": "Requiere una cámara adicional"}},
  "cumplimiento": {"requisitos": ["SYS-DET-01"], "valores": {"ALT-01": {"SYS-DET-01": "parcial"}}},
  "sensibilidad": {"variacion_pct": 20}
}
```
Reglas: pesos enteros que suman 100 · exactamente una alternativa `seleccionada` · todas las puntuaciones en escala · cumplimiento `si | parcial | no | na` · criterios orientados a "más es mejor" (p.ej. "Coste HW" se puntúa alto si es barato; dilo en la descripción).

`$H matrix validate` · `$H matrix render` (tablas de criterios, puntuaciones, ponderada con total y % normalizado, clasificación, sensibilidad ±X % por criterio, cumplimiento y justificaciones, gráfico de barras).

Proceso con el usuario: él decide criterios y pesos; tú propones puntuaciones **con justificación trazable** a los documentos y él las confirma. Si la sensibilidad cambia la ganadora, coméntalo en conclusiones. La ganadora numérica no tiene por qué ser la seleccionada (las alternativas pueden no cumplir requisitos o exigir más recursos): explícalo.
