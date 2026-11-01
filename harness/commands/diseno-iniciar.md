---
description: Fase 2 · Incorpora el diseño HW/SW y de comunicaciones que aporta el usuario, sin inventar nada
argument-hint: "<ficheros o descripción del diseño>"
---
# Iniciar diseño

`H="@@HARNESS@@/bin/h"` · Diseño aportado: `$ARGUMENTS`

Estándares: STD-DOC §4, STD-SAF, STD-NET, STD-MQTT, STD-CFG, STD-UI §7.

1. `$H gate require design`, `$H git start-phase design`, `$H scaffold design`.
2. Guarda el material original en `docs/02-diseno/entradas/` sin modificar.
3. Vuelca en `docs/02-diseno/diseno.md` **solo lo que el usuario ha definido**, en los apartados de la plantilla (arquitectura HW, SW, comunicaciones, datos/algoritmo, modos, fallos, rendimiento, configuración). Cada afirmación lleva el requisito que cubre. Lo no definido queda `[PENDIENTE: …]`; nada se completa por tu cuenta.
4. Representa lo definido con diagramas (skill **diagramas**): bloques HW, bloques SW, secuencia del ciclo de 50 ms y estados, si el material lo permite. Solo elementos que existan en el diseño del usuario. `$H diagram render`.
5. Rellena la tabla de trazabilidad requisito → apartado y lista los requisitos `Must/Should` sin cobertura como `[PENDIENTE]`.
6. Commitea `docs(design): import user design` y propone continuar con `/diseno-revisar`.
