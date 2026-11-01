# Índice de estándares del proyecto

Todos los comandos, agentes y skills del harness leen este índice y aplican los estándares de su ámbito. Los identificadores de regla (`C-FUN-01`, `GIT-BR-04`…) se citan en hallazgos, informes de test y desviaciones.

| Código | Fichero | Ámbito |
|---|---|---|
| STD-PRO | [proceso.md](proceso.md) | Fases, puertas, reglas de comportamiento de Claude y sus agentes |
| STD-C | [c-codificacion.md](c-codificacion.md) | C11 + MISRA C:2012, nombres, funciones, errores, formato, Doxygen |
| STD-TST | [testing.md](testing.md) | Independencia implementación/test, diseño de casos, criterios de aceptación, log |
| STD-GIT | [git.md](git.md) | Ramas, commits, merges, tags, hooks |
| STD-UI | [frontend.md](frontend.md) | Paleta, tipografía, componentes, diagramas, estilo de Word/PowerPoint |
| STD-DOC | [documentacion.md](documentacion.md) | Estructura y reglas de documentos, diagramas y exportación |
| STD-REQ | [requisitos-redaccion.md](requisitos-redaccion.md) | Calidad y atributos de requisitos, ciclo de iteración |
| STD-MQTT | [mqtt.md](mqtt.md) | Topics, payload (JSON por ahora), QoS, robustez |
| STD-LOG | [logging.md](logging.md) | Formato key=value, niveles syslog, logging sin E/S en el ciclo |
| STD-NET | [ethernet.md](ethernet.md) | IP estática, VLAN, watchdog de enlace, PTP/NTP |
| STD-SH | [shell.md](shell.md) | Bash 3.2 estricto + shellcheck; Python de herramientas |
| STD-DEP | [despliegue.md](despliegue.md) | CMake presets, Docker Compose |
| STD-SAF | [seguridad-funcional.md](seguridad-funcional.md) | Estado seguro, FMEA ligero, trazabilidad |
| STD-CFG | [configuracion.md](configuracion.md) | Parámetros calibrables validados |
| STD-DAT | [datos-prueba.md](datos-prueba.md) | Grabaciones de radar y escenarios |

Decisiones aplazadas a la fase de diseño (no las decide el harness): representación numérica del grid (float32/double/punto fijo), árbol de topics y QoS, protocolo de payload definitivo, hardware objetivo, plan de IPs/VLAN.
