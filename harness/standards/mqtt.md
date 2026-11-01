# STD-MQTT · Comunicación MQTT

| Campo | Valor |
|---|---|
| Versión | 1.0.0 |
| Broker / cliente | Eclipse Mosquitto 2.x · libmosquitto (C) · paho-mqtt (herramientas Python) |
| Protocolo | MQTT v5 (v3.1.1 admitido solo si el diseño lo justifica) |
| Pendiente de diseño | Árbol de topics, QoS por topic y protocolo de payload definitivo |

## 1. Topics

| ID | Nivel | Regla |
|---|---|---|
| MQ-TOP-01 | OBL | El árbol de topics se define en la fase de diseño, en `docs/02-diseno/diseno.md` (apartado Comunicaciones) como tabla: `Topic | Publicador | Suscriptores | QoS | Retained | Periodo | Payload (esquema) | Requisitos`. |
| MQ-TOP-02 | OBL | Topics en minúsculas, segmentos `snake_case`, sin espacios ni `/` inicial ni final. Comodines solo en suscripciones de herramientas, nunca en el núcleo. |
| MQ-TOP-03 | OBL | Todo cliente publica estado `online/offline` con Last Will (LWT) retained. |
| MQ-TOP-04 | OBL | Los topics son constantes centralizadas en `include/app/topics.h` (C) y `tools/topics.py` (Python) generadas desde la tabla del diseño; prohibido escribir literales de topic en otro sitio. |

## 2. Payload

| ID | Nivel | Regla |
|---|---|---|
| MQ-PAY-01 | OBL | **De momento JSON** UTF-8. El formato puede cambiar en diseño según las características del sistema (ancho de banda, 20 Hz): el código aísla (de)serialización en `src/app/codec_*.c` para poder sustituirla sin tocar el núcleo. |
| MQ-PAY-02 | OBL | Todo mensaje lleva cabecera común: `{"v": <versión de esquema>, "seq": <uint32>, "ts_ms": <uint64 epoch ms>, "src": "<nodo>", ...}`. |
| MQ-PAY-03 | OBL | Cada payload tiene esquema JSON en `docs/02-diseno/esquemas/<mensaje>.schema.json`; los tests de integración validan contra él. |
| MQ-PAY-04 | OBL | Unidades SI en los nombres de campo (`range_m`, `azimuth_rad`, `rcs_dbsm`, `vr_mps`). Coma flotante en JSON con precisión fija documentada. |
| MQ-PAY-05 | OBL | El parser rechaza (y registra) mensajes con versión desconocida, campos obligatorios ausentes, tipos incorrectos o tamaño > límite configurado; nunca usa datos parcialmente válidos. |

## 3. Calidad de servicio y robustez

| ID | Nivel | Regla |
|---|---|---|
| MQ-QOS-01 | OBL | Datos periódicos de sensor: QoS 0 (el dato viejo no vale); comandos y configuración: QoS 1 con idempotencia por `seq`; estado: QoS 1 retained. Excepciones justificadas en diseño. |
| MQ-QOS-02 | OBL | Detección de huecos y desorden por `seq`; contadores expuestos en el topic de diagnóstico. |
| MQ-QOS-03 | OBL | Pérdida de conexión con el broker o de N ciclos consecutivos de datos (N en configuración) → evento de fallo y estado seguro según STD-SAF. |
| MQ-QOS-04 | OBL | Reconexión con backoff exponencial acotado; las publicaciones nunca bloquean el hilo de proceso (cola de tamaño fijo, política de descarte documentada). |

## 4. Seguridad

Broker sin acceso anónimo fuera de la simulación local; usuarios por nodo con ACL por topic (`deploy/mosquitto/acl`). TLS opcional según diseño.
