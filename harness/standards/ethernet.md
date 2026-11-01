# STD-NET · Red Ethernet

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Ámbito | Red embarcada entre radar, unidad de proceso, broker y estación de visualización (y su réplica en la simulación Docker) |

## 1. Direccionamiento y segmentación

| ID | Nivel | Regla |
|---|---|---|
| NET-IP-01 | OBL | Direccionamiento IPv4 **estático**; sin DHCP en la red de seguridad. |
| NET-IP-02 | OBL | Plan de direccionamiento en `docs/02-diseno/diseno.md` como tabla: `Nodo | Función | VLAN | IP/máscara | Puertos | Protocolo | Dirección del tráfico`. La simulación (`deploy/docker-compose.yml`) replica las mismas IPs y redes. |
| NET-IP-03 | OBL | Separación por VLAN: sensores (datos crudos), control/seguridad, mantenimiento/visualización. El tráfico de visualización no puede llegar a la VLAN de sensores. |
| NET-IP-04 | OBL | Puertos documentados y mínimos; MQTT 1883 solo en simulación local, 8883 (TLS) si el diseño lo exige. |

## 2. Supervisión de enlace y tiempos

| ID | Nivel | Regla |
|---|---|---|
| NET-TMO-01 | OBL | Watchdog de datos: si no llega una nube válida en `N × 50 ms` (N en configuración, p.ej. 3) → fallo de sensor y estado seguro (STD-SAF). |
| NET-TMO-02 | OBL | Detección de pérdida de enlace físico (estado del interfaz) y lógico (heartbeat/LWT) diferenciadas en el diagnóstico. |
| NET-TMO-03 | OBL | Todo socket con timeouts explícitos; ninguna llamada de red bloquea sin límite. |
| NET-TMO-04 | OBL | Presupuesto de latencia extremo a extremo (radar → grid publicado) documentado en diseño y medido en integración. |

## 3. Sincronización de tiempo

| ID | Nivel | Regla |
|---|---|---|
| NET-TIM-01 | OBL | Fuente de tiempo común: PTP (IEEE 1588) si el hardware lo soporta, si no NTP; precisión requerida fijada en diseño. |
| NET-TIM-02 | OBL | Los timestamps de los datos son los del sensor (tiempo de medida), no los de recepción; ambos se registran para medir latencia. |
| NET-TIM-03 | OBL | Desfase de reloj fuera de tolerancia → aviso y degradación definida en diseño. |
| NET-TIM-04 | OBL | En simulación, el reloj se inyecta (modo replay usa el tiempo de la grabación). |
