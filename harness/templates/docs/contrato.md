# Contrato @FID@ · `@FUNC@`

| Campo | Valor |
|---|---|
| Función | `@FUNC@` |
| Módulo | `@MOD@` |
| Cabecera | `include/@MOD@/@MOD@.h` |
| Fuente | `src/@MOD@/@FUNC@.c` |
| Requisitos | [PENDIENTE] |
| Apartado de diseño | [PENDIENTE] |
| Estado | Borrador (pendiente de aprobación del usuario) |

## 1. Propósito

## 2. Firma

```c
@MOD@_status_t @FUNC@(/* parámetros */);
```

## 3. Parámetros

| Parámetro | Dirección | Tipo | Rango válido | Unidad | Descripción |
|---|---|---|---|---|---|

## 4. Precondiciones (`pre`)

| ID | Condición | Si no se cumple |
|---|---|---|
| PRE-1 | | devuelve `@MOD_UPPER@_ERR_PARAM` sin modificar salidas |

## 5. Postcondiciones (`post`)

| ID | Condición (con tolerancia si es numérica) |
|---|---|
| POST-1 | |

## 6. Errores (`err`)

| Código | Cuándo | Estado de las salidas |
|---|---|---|
| `@MOD_UPPER@_OK` | | |

## 7. Rendimiento (`perf`)

WCET objetivo / complejidad: [PENDIENTE]

## 8. Efectos laterales y dependencias

Funciones del plan que usa: —. Estado que modifica: —.

## 9. Ejemplos de referencia (opcional, solo si el usuario los aporta)
