# Pseudocódigo de la actualización de la probabilidad de ocupación de una celda

| Campo | Valor |
|---|---|
| Proyecto | prueba-saf |
| Código | SAF-DIS-02 |
| Versión | 0.1 |
| Fecha | 2026-11-02 |
| Autor | Alicia Garcia |
| Estado | Borrador |

Este documento cubre SYS-IMP-10 (enunciado R3): pseudocódigo de la actualización de la probabilidad interna de ocupación de una celda del grid, sin derivar nunca un estado «libre». Corresponde al paso 10 del ciclo (`diseno.md` §4) y al modelo de `diseno.md` §6.2 (SYS-PR-08).

## 1. Parámetros

Todos proceden de `diseno.md` §10 (STD-CFG).

| Constante | Valor | Unidad |
|---|---|---|
| `LO_INIT` | 0 | log-odds |
| `LO_ECHO` | +0,85 | log-odds |
| `LO_PASS` | −0,40 | log-odds |
| `LO_MAX` | 3,4 | log-odds |
| `LO_MIN` | −0,8 | log-odds |
| `P_OCCUPIED` | 0,85 | probabilidad |
| Tamaño de celda | 0,25 m | m |
| Dimensiones del grid | 880 celdas en s × 4 celdas en d = 3 520 | celdas |

## 2. Datos

Memoria fija, dimensionada en compilación (SYS-IMP-01, C-LIB-01):

```
tipo CellVote   = { ECHO, PASS, HIDDEN }          // resultado del haz sobre la celda
tipo CellState  = { OCCUPIED, LOW_OBSTACLE_PROB, NOT_STUDIED }
tipo Cell       = { lo: real, observed: bool }     // lo en log-odds
tipo Grid       = Cell[880][4]                     // sin memoria dinámica
```

El estado «libre» no existe en `CellState` (SYS-SAF-07, SYS-OUT-06).

## 3. Reinicio

```
función grid_reset(grid):
    para cada celda c de grid:                     // cota estática: 3 520
        c.lo       = LO_INIT
        c.observed = falso
```

Se llama al arrancar y al volver de FALLO (`diseno.md` §6.3).

## 4. Actualización de una celda por haz

```
función cell_update(c, vote):
    // entrada: celda c (entrada/salida), voto del haz
    // un eco descartado (pasos 5 a 9) no genera voto: la celda no cambia (H-024)
    si vote == ECHO:
        c.lo = min(c.lo + LO_ECHO, LO_MAX)
        c.observed = verdadero
    si vote == PASS:
        c.lo = max(c.lo + LO_PASS, LO_MIN)
        c.observed = verdadero
    si vote == HIDDEN:
        // celda oculta detrás de un eco: sin cambio
        no hacer nada
```

Reglas:

- Una celda que ningún haz toca en el ciclo mantiene su valor, sin decaimiento temporal (`diseno.md` §6.2, §6.3).
- La función no lee el reloj, no usa aleatoriedad ni memoria dinámica: mismas entradas dan mismas salidas (SYS-IMP-02, C-CON-02, C-FUN-06).
- Cada función tiene cota de bucle estática (880 × 4 celdas).

## 5. Probabilidad y estado derivado

```
función cell_probability(c):
    devolver 1 / (1 + exp(-c.lo))                  // logaritmo natural (H-013, H-026)

función cell_state(c):
    si no c.observed:
        devolver NOT_STUDIED                       // nunca observada o reiniciada
    si cell_probability(c) > P_OCCUPIED:           // equivale a lo > 1,73
        devolver OCCUPIED
    devolver LOW_OBSTACLE_PROB                     // observada y p <= 0,85
```

Con `P_OCCUPIED = 0,85`, el umbral en log-odds es ln(0,85 / 0,15) ≈ 1,73 (`diseno.md` §6.2, §10).

## 6. Ejemplo A: tres ecos seguidos

Se parte de `lo = 0`, `observed = falso` y un haz con eco en la celda por ciclo.

| Ciclo | Voto | lo | p | Estado |
|---|---|---|---|---|
| 0 | (inicial) | 0 | 0,50 | NOT_STUDIED |
| 1 | ECHO | 0,85 | 0,70 | LOW_OBSTACLE_PROB |
| 2 | ECHO | 1,70 | 0,845 | LOW_OBSTACLE_PROB |
| 3 | ECHO | 2,55 | 0,93 | OCCUPIED |

El umbral p > 0,85 (l > 1,73) no se alcanza hasta la tercera detección, coherente con N = 3 (SYS-PR-09). Con el mínimo de −0,8, el peor caso también confirma en 3 ecos: −0,8 + 3 × 0,85 = 1,75 > 1,73 (`diseno.md` §10, CFG-02).

## 7. Ejemplo B: recuperación tras saturación

La celda está saturada con `lo = 3,4` (p ≈ 0,97). Desde el ciclo 1 el haz pasa por la celda sin eco (`PASS`) en cada ciclo.

| Ciclo | Voto | lo | p | Estado |
|---|---|---|---|---|
| 0 | (saturada) | 3,4 | 0,97 | OCCUPIED |
| 1 | PASS | 3,0 | 0,95 | OCCUPIED |
| 2 | PASS | 2,6 | 0,93 | OCCUPIED |
| 3 | PASS | 2,2 | 0,90 | OCCUPIED |
| 4 | PASS | 1,8 | 0,86 | OCCUPIED |
| 5 | PASS | 1,4 | 0,80 | LOW_OBSTACLE_PROB |

La celda deja de ser «ocupado» en 5 ciclos, es decir 250 ms (`diseno.md` §6.2). La saturación en 3,4 acota el tiempo de recuperación. En el ciclo 4 sigue ocupada porque 1,8 > 1,73. Si el haz no pasa por la celda, o está oculta, el valor no cambia.

## 8. Trazabilidad

| Elemento | Requisito |
|---|---|
| Votos, saturación y probabilidad | SYS-PR-08 |
| Estados derivados sin «libre» | SYS-OUT-06, SYS-SAF-07, SYS-IMP-10 |
| Umbral de objeto y confirmación | SYS-PR-04, SYS-PR-09 |
| Sin memoria dinámica | SYS-IMP-01 |
| Determinismo | SYS-IMP-02 |
| Eco descartado sin cambio | SYS-PR-05 |

## Historial de cambios

| Versión | Fecha | Autor | Cambios |
|---|---|---|---|
| 0.1 | 2026-11-02 | Alicia Garcia | Creación a partir de `diseno.md` §4, §6 y §10 |
