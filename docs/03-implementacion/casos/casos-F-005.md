# Casos de test · F-005 (ogrid_get_cell_state)

| Caso | Requisito | Cláusula | Técnica | Entrada | Resultado esperado |
|---|---|---|---|---|---|
| TC-F005-01 | SYS-PR-08 | err OGRID_OK | Partición válida | l=1,0, observed=YES | OGRID_OK |
| TC-F005-02 | SYS-PR-08 | post POST-1 | Partición | observed=NO, l=0 | OK, UNSURVEYED |
| TC-F005-03 | SYS-PR-08 | post POST-1 | Partición (cualquier l) | observed=NO, l en {3,4; −0,8; 2,55; −5; 100; NaN; ±inf} | OK, UNSURVEYED |
| TC-F005-04 | SYS-PR-08 | post POST-3 | Analítico (contrato) | YES, l=1,70 (p=0,8455) | OK, LOW_PROB |
| TC-F005-05 | SYS-PR-08 | post POST-2 | Analítico (contrato) | YES, l=2,55 (p=0,9276) | OK, OCCUPIED |
| TC-F005-06 | SYS-PR-08 | post POST-3 | Valor límite (umbral−) | YES, l=1,73 (p=0,8494) | OK, LOW_PROB |
| TC-F005-07 | SYS-PR-08 | post POST-2 | Valor límite (umbral+) | YES, l=1,74 (p=0,8504) | OK, OCCUPIED |
| TC-F005-08 | SYS-PR-08 | post POST-3 | Valor límite (mín) | YES, l=−0,8 y l=0 | OK, LOW_PROB |
| TC-F005-09 | SYS-PR-08 | post POST-2 | Valor límite (máx) | YES, l=3,4 (p=0,968) | OK, OCCUPIED |
| TC-F005-10 | SYS-PR-08 | post POST-2/3 | Propiedad: monotonía | YES, l de −0,8 a 3,35 paso 0,05 | OK; estado no decrece; l<1,72 LOW_PROB; l>1,75 OCCUPIED |
| TC-F005-11 | SYS-SAF-07 | post POST-4 | Propiedad: acotación de salida | l×observed (0,1,2,255) en barrido | salida siempre en los 3 estados; estado OK o ERR_STATE |
| TC-F005-12 | SYS-SAF-07 | post POST-5 | Partición (observed corrupto) | observed=2, l=2,55, salida previa OCCUPIED | ERR_STATE, UNSURVEYED |
| TC-F005-13 | SYS-SAF-07 | post POST-5 | Valor límite (observed=255) | observed=255, l=0, salida previa LOW_PROB | ERR_STATE, UNSURVEYED |
| TC-F005-14 | SYS-SAF-07 | err ERR_STATE | Combinación | observed=3, l=NaN | ERR_STATE, UNSURVEYED |
| TC-F005-15 | SYS-SAF-07 | post POST-5 | Partición (l corrupto) | YES, l=NaN | ERR_STATE, UNSURVEYED |
| TC-F005-16 | SYS-SAF-07 | post POST-5 | Partición (l corrupto) | YES, l=±inf | ERR_STATE, UNSURVEYED |
| TC-F005-17 | SYS-SAF-07 | post POST-5 | Límite fuera de rango | YES, l en {nextafter(−0,8), −0,81, −100, nextafter(3,4), 3,41, 1e30} | ERR_STATE, UNSURVEYED |
| TC-F005-18 | SYS-SAF-07 | err ERR_STATE | Escritura de salida | observed=7, salida previa centinela | ERR_STATE, salida = UNSURVEYED |
| TC-F005-19 | SYS-PR-08 | pre PRE-1 / err ERR_PARAM | Puntero nulo | grid=NULL | ERR_PARAM, salida sin cambios |
| TC-F005-20 | SYS-PR-08 | pre PRE-1 / err ERR_PARAM | Puntero nulo | state_out=NULL | ERR_PARAM, grid sin cambios |
| TC-F005-21 | SYS-PR-08 | pre PRE-1 / err ERR_PARAM | Puntero nulo | ambos NULL | ERR_PARAM |
| TC-F005-22 | SYS-PR-08 | pre PRE-2 / err ERR_RANGE | Valor límite (máx+1) | cell_idx=OGRID_CELL_COUNT | ERR_RANGE, salida sin cambios |
| TC-F005-23 | SYS-PR-08 | pre PRE-2 / err ERR_RANGE | Partición fuera de rango | cell_idx=COUNT+1, UINT32_MAX | ERR_RANGE, salida sin cambios |
| TC-F005-24 | SYS-PR-08 | pre PRE-2 | Valores límite (mín, máx) | cell_idx=0 y COUNT−1 | OK; estados esperados de cada celda |
| TC-F005-25 | SYS-PR-08 | post POST-1 | Independencia de celdas | celda 42 ocupada; consulta 41, 42, 43 | vecinas UNSURVEYED, 42 OCCUPIED |
| TC-F005-26 | SYS-SAF-07 | post POST-6 | Inmutabilidad | llamadas OK, STATE, RANGE y PARAM | grid idéntico al original |
| TC-F005-27 | SYS-SAF-07 | pre orden PRE-1 antes PRE-2 | Orden de comprobación | grid=NULL, idx=COUNT | ERR_PARAM, salida sin cambios |
| TC-F005-28 | SYS-SAF-07 | pre orden PRE-1 antes PRE-2 | Orden de comprobación | state_out=NULL, idx=UINT32_MAX | ERR_PARAM |
| TC-F005-29 | SYS-SAF-07 | pre orden PRE-2 antes POST-5 | Orden de comprobación | grid totalmente corrupto, idx=COUNT | ERR_RANGE, salida sin cambios |
| TC-F005-30 | SYS-SAF-07 | pre orden PRE-1 antes POST-5 | Orden de comprobación | state_out=NULL, celda corrupta | ERR_PARAM |
| TC-F005-31 | SYS-SAF-07 | post POST-5 | Aislamiento de fallo | celda corrupta junto a celda sana ocupada | sana: OK, OCCUPIED |

## Cobertura por cláusula

- PRE-1: 19, 20, 21, 27, 28, 30. PRE-2: 22, 23, 24, 27, 28, 29.
- POST-1: 02, 03, 25. POST-2: 05, 07, 09, 10. POST-3: 04, 06, 08, 10. POST-4: 11. POST-5: 12 a 18, 31. POST-6: 19, 20, 22, 23, 26.
- Errores: OK (01), PARAM, RANGE, STATE.
- perf: sin caso unitario; el contrato la mide en integración (WCET).

## Notas

- No se prueba l exactamente en el umbral ln(0,85/0,15)=1,734601: con float32 p=0,85 es indecidable. Se usa 1,73 y 1,74 (a 4e-4 en p, muy por encima del error de expf ~1e-7).
- Los límites −0,8 y 3,4 se comparan con las macros del módulo, sin tolerancia.
- DUDA: la sección 4 del contrato solo lista PRE-1 y PRE-2 y no fija explícitamente el orden PARAM, RANGE, STATE. Los casos 27 a 30 asumen el orden implícito PRE-1, PRE-2, POST-5.
- DUDA: observed=NO con log-odds NaN/inf devuelve OK y UNSURVEYED según POST-1 ("cualquier valor") y POST-5 (solo celdas observadas); el caso 03 lo fija así.
