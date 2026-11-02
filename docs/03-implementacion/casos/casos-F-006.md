# Casos de test · F-006 `ogrid_update_ray`

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F006-01 | SYS-PR-08 | post POST-5 | Ejemplo del contrato | [10,11,12,13], eco pos 2, grid reiniciado | OK; 10=11=-0,40; 12=+0,85 observed; 13=0 sin observed |
| TC-F006-02 | SYS-PR-08 | post POST-1 | Partición: sin eco | [5..8], sin eco | OK; todas -0,40 y observed |
| TC-F006-03 | SYS-PR-08 | post POST-1 | Partición: echo_pos ignorado | sin eco, echo_pos=UINT32_MAX | OK; todas -0,40 |
| TC-F006-04 | SYS-PR-08 | post POST-2 | Límite echo_pos=0 | 3 celdas, eco pos 0 | 1ª +0,85; resto sin cambio |
| TC-F006-05 | SYS-PR-08 | post POST-2 | Límite echo_pos=count-1 | 3 celdas, eco pos 2 | 2 primeras -0,40; última +0,85 |
| TC-F006-06 | SYS-PR-08 | post POST-2 | Límite count=1 | 1 celda con eco | +0,85 |
| TC-F006-07 | SYS-PR-08 | post POST-2 | Ocultas conservan | celdas ocultas con valores previos | idénticas bit a bit |
| TC-F006-08 | SYS-PR-08 | post POST-3 | Celdas ajenas, con eco | vecinas y extremos con datos | sin cambios fuera del haz |
| TC-F006-09 | SYS-PR-08 | post POST-3 | Celdas ajenas, sin eco | vecinas con datos | sin cambios fuera del haz |
| TC-F006-10 | SYS-PR-08 | post POST-1 | Valor analítico | previa 0,5 + paso | 0,1 |
| TC-F006-11 | SYS-PR-08 | post POST-2 | Valor analítico | previa 0,5 + eco | 1,35 |
| TC-F006-12 | SYS-PR-08 | post POST-4 | Límite superior | previa 3,3 + eco | 3,4 |
| TC-F006-13 | SYS-PR-08 | post POST-4 | Límite inferior | -0,6 y -0,8 + paso | -0,8 |
| TC-F006-14 | SYS-PR-08 | post POST-4 | Analítico N haces | 4 haces sin eco | -0,4; -0,8; -0,8; -0,8 |
| TC-F006-15 | SYS-PR-08 | post POST-4 | Analítico N haces | 5 haces con eco | 0,85; 1,70; 2,55; 3,4; 3,4 |
| TC-F006-16 | SYS-PR-08 | post POST-4 | Secuencia mixta | eco y luego paso | 0,45 |
| TC-F006-17 | SYS-PR-08 | post POST-4 | Propiedad: acotación | 30 haces mixtos | todo en [-0,8; 3,4] |
| TC-F006-18 | SYS-PR-08 | post POST-6 | Límite count=0 | cells_in no nulo, count 0 | OK; grid igual |
| TC-F006-19 | SYS-PR-08 | post POST-6 | Límite count=0 | cells_in NULL, count 0 | OK; grid igual |
| TC-F006-20 | SYS-PR-08 | err ERR_PARAM | Puntero nulo | grid NULL | ERR_PARAM |
| TC-F006-21 | SYS-PR-08 | err ERR_PARAM | Puntero nulo | grid NULL, cells NULL, count 0 | ERR_PARAM |
| TC-F006-22 | SYS-PR-08 | err ERR_PARAM / POST-7 | Puntero nulo | cells NULL, count 4 | ERR_PARAM; grid sin cambios |
| TC-F006-23 | SYS-PR-08 | pre PRE-5 | Valor límite máx+1 | echo_pos=count | ERR_RANGE; grid sin cambios |
| TC-F006-24 | SYS-PR-08 | pre PRE-5 | Valor extremo | echo_pos=UINT32_MAX | ERR_RANGE; grid sin cambios |
| TC-F006-25 | SYS-PR-08 | pre PRE-3 | Valor límite máx+1 | count=OGRID_CELL_COUNT+1 | ERR_RANGE; grid sin cambios |
| TC-F006-26 | SYS-PR-08 | pre PRE-3 | Valor extremo | count=UINT32_MAX | ERR_RANGE; grid sin cambios |
| TC-F006-27 | SYS-PR-08 | pre PRE-3 | Límite máx | count=COUNT, sin eco | OK; todas -0,40 y observed |
| TC-F006-28 | SYS-PR-08 | post POST-2 | Límite máx | count=COUNT, eco en última | OK; anteriores -0,40; última +0,85 |
| TC-F006-29 | SYS-PR-08, SYS-SAF-07 | pre PRE-4 / POST-7 | Límite máx+1 de índice, atomicidad | [10,11,COUNT] | ERR_RANGE; 10 y 11 sin cambios |
| TC-F006-30 | SYS-PR-08, SYS-SAF-07 | pre PRE-4 / POST-7 | Índice extremo en zona oculta | [10,11,12,UINT32_MAX], eco pos 1 | ERR_RANGE; grid sin cambios |
| TC-F006-31 | SYS-PR-08, SYS-SAF-07 | pre PRE-4 / POST-7 | Índice inválido primero | [COUNT+1,11,12] | ERR_RANGE; grid sin cambios |
| TC-F006-32 | SYS-PR-08 | pre PRE-4 | Límites mín y máx de índice | [0, COUNT-1], eco pos 1 | OK; 0=-0,40; COUNT-1=+0,85 |
| TC-F006-33 | SYS-SAF-07 | post POST-7 | Atomicidad en grid poblado | echo_pos=5 con count 2 | ERR_RANGE; grid sin cambios |
| TC-F006-34 | SYS-SAF-07 | err ERR_STATE | Celda corrupta (log-odds 5,0 fuera de rango) | [10..13], 11 corrupta, sin eco | ERR_STATE; 11, 12 y 13 sin cambios; fuera del haz sin cambios |
| TC-F006-35 | SYS-SAF-07 | err ERR_STATE | Celda corrupta (NaN) en celda de eco | [20,21,22], 21 NaN, eco pos 1 | ERR_STATE; fuera del haz sin cambios |
| TC-F006-36 | SYS-SAF-07 | err ERR_STATE | Corrupción (infinito) en la primera celda | 30 con log-odds +inf | ERR_STATE; 30, 31 y 32 sin cambios |
| TC-F006-37 | SYS-SAF-07 | err OGRID_OK | Partición: grid sano | 6 haces alternando eco/sin eco | siempre OK |
| TC-F006-38 | SYS-PR-08 | perf / post | Determinismo, haz máximo | dos grids, count=COUNT, eco pos 1000 | mismo resultado; 1000=+0,85; 1001=0 |
| TC-F006-39 | SYS-PR-08 | err ERR_RANGE | Límite count=0 con eco | cells no nulo, count 0, eco | ERR_RANGE; grid sin cambios |
| TC-F006-40 | SYS-PR-08 | err ERR_RANGE | Límite count=0 con eco | cells NULL, count 0, eco | ERR_RANGE (no PARAM); grid sin cambios |
| TC-F006-41 | SYS-PR-08 | err ERR_PARAM | Precedencia PARAM > RANGE | cells NULL, count=COUNT+1, eco pos fuera | ERR_PARAM; grid sin cambios |
| TC-F006-42 | SYS-PR-08 | err ERR_PARAM | Precedencia PARAM > RANGE | grid NULL, count y echo_pos UINT32_MAX | ERR_PARAM |
| TC-F006-43 | SYS-SAF-07 | err ERR_RANGE | Precedencia RANGE > STATE | echo_pos=7 con count 3, celda 11 corrupta | ERR_RANGE; grid sin cambios (celda 10 incluida) |
| TC-F006-44 | SYS-SAF-07 | err ERR_RANGE | Precedencia RANGE > STATE | [10,11 NaN,COUNT], sin eco | ERR_RANGE; grid sin cambios (celda 10 incluida) |
| TC-F006-45 | SYS-SAF-07 | err ERR_STATE (no aplica) | Celda corrupta oculta | [20,21,22], 22 NaN, eco pos 1 | OK; 20=-0,40; 21=+0,85; 22 sin cambios |
| TC-F006-46 | SYS-SAF-07 | err ERR_STATE (no aplica) | Celdas corruptas ocultas, eco pos 0 | [30,31,32], 31 +inf, 32 -2,0, eco pos 0 | OK; 30=+0,85; 31 y 32 sin cambios |

Tolerancia numérica: 1e-5 (error float32 de pocas sumas con magnitud ≤ 3,4 es < 1e-6; separación mínima entre valores esperados distintos 0,05).
