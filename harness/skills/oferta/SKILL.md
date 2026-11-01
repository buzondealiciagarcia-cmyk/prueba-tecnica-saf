---
name: oferta
description: Cómo construir la oferta al cliente (9 secciones iteradas con el usuario, presupuesto y plazo generados, exportación a Word) y su presentación PowerPoint con la paleta del proyecto.
---
# Oferta y presentación

CLI: `H="@@HARNESS@@/bin/h"`

## Secciones (`docs/04-oferta/oferta.md`, marcador `<!-- seccion: X | estado: … -->`)
| Clave | Sección | Fuentes principales | Lo que hay que preguntar |
|---|---|---|---|
| `objeto` | Objeto de la oferta | requisitos §1 | qué se ofrece exactamente, a quién |
| `introduccion` | Introducción | contexto del dosier, diseño §1 | tono, antecedentes del cliente |
| `datos` | Datos utilizados | diseño (sensor, formato), STD-DAT, datasets de test | qué datos aporta el cliente, confidencialidad |
| `objetivos` | Objetivos | requisitos Must | priorización comercial |
| `alcance` | Alcance del proyecto y de los resultados | alcance implementado, alternativas, entregables | incluido / excluido / entregables / garantía |
| `requisitos` | Requisitos | requisitos cerrados (resumen por tipo) | nivel de detalle para el cliente |
| `descripcion` | Descripción del proyecto | diseño, implementación, metodología (fases, verificación independiente) | qué se cuenta de la metodología |
| `presupuesto` | Presupuesto | `presupuesto.json` → `$H offer budget` | partidas, horas, tarifas, otros, contingencia, impuestos, condiciones de pago |
| `plazo` | Plazo | `presupuesto.json → plazo` → Gantt | fecha de inicio, tareas, dependencias, hitos |

Estados: `borrador` → `en revision` → `aprobado` (`$H offer section <clave> <estado>`; `aprobado` solo con confirmación explícita). `$H offer status` muestra el avance.

## Presentación (`docs/04-oferta/presentacion.json` → `$H pptx build`)
```json
{"titulo": "…", "subtitulo": "…", "autor": "…", "cliente": "…", "fecha": "…", "pie": "…",
 "diapositivas": [
  {"tipo": "titulo"},
  {"tipo": "seccion", "numero": "01", "titulo": "…", "texto": "…"},
  {"tipo": "vinetas", "titulo": "…", "vinetas": ["…"], "notas": "…"},
  {"tipo": "dos_columnas", "titulo": "…", "izquierda_titulo": "…", "izquierda": [], "derecha_titulo": "…", "derecha": []},
  {"tipo": "imagen", "titulo": "…", "imagen": "docs/02-diseno/diagramas/x.mmd", "pie": "…"},
  {"tipo": "tabla", "titulo": "…", "cabecera": [], "filas": [[]]},
  {"tipo": "kpis", "titulo": "…", "kpis": [{"valor": "50 ms", "etiqueta": "…"}]},
  {"tipo": "cierre", "titulo": "Gracias", "texto": "…"}]}
```
16:9, fondo blanco, título con barra de acento, ≤ 6 viñetas, pie con proyecto y número. Guion recomendado: título → problema → objetivos → solución (arquitectura) → por qué esta solución (comparativa) → verificación (KPIs) → alcance → plan (Gantt) → presupuesto → cierre.
