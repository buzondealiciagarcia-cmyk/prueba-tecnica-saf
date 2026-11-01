# Documento de diseño

| Campo | Valor |
|---|---|
| Proyecto | @PROJECT@ |
| Código | @CODE@-DIS |
| Versión | 0.1 |
| Fecha | @DATE@ |
| Autor | @AUTHOR@ |
| Estado | Borrador |

> Este documento recoge **solo** el diseño definido por el usuario (STD-DOC DOC-GEN-01). Las propuestas de Claude se marcan `[PROPUESTA]` hasta su aceptación y los huecos `[PENDIENTE: …]`.

## 1. Contexto y objetivos del diseño

## 2. Requisitos de referencia

Versión de requisitos: `req-v1.0`. Cada apartado cita los requisitos que cubre.

## 3. Arquitectura hardware

![Figura. Arquitectura hardware](diagramas/arquitectura-hw.svg)

## 4. Arquitectura software

![Figura. Arquitectura software](diagramas/arquitectura-sw.svg)

## 5. Comunicaciones

### 5.1 Red y direccionamiento (STD-NET)

| Nodo | Función | VLAN | IP/máscara | Puertos | Protocolo | Dirección del tráfico |
|---|---|---|---|---|---|---|

### 5.2 Topics MQTT (STD-MQTT)

| Topic | Publicador | Suscriptores | QoS | Retained | Periodo | Payload (esquema) | Requisitos |
|---|---|---|---|---|---|---|---|

![Figura. Secuencia de un ciclo de 50 ms](diagramas/secuencia-ciclo.svg)

## 6. Datos y algoritmo del grid de ocupación

### 6.1 Representación del grid

### 6.2 Modelo inverso del sensor

### 6.3 Actualización y dinámica (decaimiento, estáticos vs. móviles)

### 6.4 Flujo por ciclo

## 7. Modos y estados del sistema

![Figura. Modos del sistema](diagramas/estados.svg)

## 8. Gestión de fallos y seguridad funcional (STD-SAF)

| Modo de fallo | Causa | Efecto | Detección | Mitigación | Requisito |
|---|---|---|---|---|---|

## 9. Rendimiento y presupuesto temporal

## 10. Configuración (STD-CFG)

## 11. Trazabilidad requisito → apartado

| Requisito | Apartado(s) del diseño |
|---|---|

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | @DATE@ | @AUTHOR@ | Creación a partir del diseño aportado por el usuario |
