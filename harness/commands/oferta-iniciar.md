---
description: Fase 4 · Prepara la oferta al cliente a partir de lo generado en las fases 1-3
---
# Iniciar la oferta

`H="@@HARNESS@@/bin/h"`

Skill: **oferta**.

1. `$H gate require offer`, `$H git start-phase offer`, `$H scaffold offer`.
2. Reúne el material disponible: requisitos, diseño, comparativa, implementación, trazabilidad, `log-tests.md`, métricas de los informes. Haz un inventario de qué puede alimentar cada sección de la oferta y qué **no se sabe** (cliente, condiciones, tarifas, horas, plazos, validez, forma de pago…).
3. Pregunta al usuario los datos generales con `AskUserQuestion`: cliente y destinatario, idioma y tono, validez de la oferta, moneda e impuestos, si se incluye la implementación realizada como demostrador. Actualiza la tabla de control de `oferta.md` y `harness.config.json → project.client`.
4. Commitea `docs(offer): start offer` y propone el orden de trabajo por secciones con `/oferta-seccion <sección>`: `objeto`, `introduccion`, `datos`, `objetivos`, `alcance`, `requisitos`, `descripcion`, `presupuesto`, `plazo`.
