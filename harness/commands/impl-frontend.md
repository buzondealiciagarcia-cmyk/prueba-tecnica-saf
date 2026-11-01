---
description: Fase 3 · Valora e implementa un front-end para entender o interactuar con la parte implementada
argument-hint: "[qué debe mostrar]"
---
# Front-end

`H="@@HARNESS@@/bin/h"` · Petición: `$ARGUMENTS`

Estándar: STD-UI (`@@HARNESS@@/standards/frontend.md`). Kit: `@@HARNESS@@/templates/frontend/`.

1. `$H gate require impl`. Valora con el usuario si aporta valor: visualizar el grid y la nube de puntos, ver parámetros calibrables, reproducir escenarios de STD-DAT, mostrar métricas (latencia, ecos, celdas ocupadas). Si no aporta, no se hace.
2. Define con el usuario vistas y datos a mostrar; el formato de los mensajes sale del diseño (STD-MQTT). Nada de controles que actúen sobre la seguridad (UI-TEC-03).
3. `$H git start ui/<desc>` y `$H scaffold frontend` (copia `frontend/` y `tools/ui_bridge.py`).
4. Adapta `app.js` al formato real de mensajes y a las vistas acordadas usando solo componentes del kit y variables CSS. Prueba con `python3 tools/ui_bridge.py --replay <grabación>` y revisa el resultado visualmente (captura con el navegador si está disponible); comprueba tema claro y oscuro.
5. El front-end no es código crítico: no pasa por el ciclo de agentes aislados, pero sí por STD-UI y por la revisión del usuario. Commitea `feat(ui): ...` y, tras la aprobación explícita del usuario, integra con `$H git finish`.
